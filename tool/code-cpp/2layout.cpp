#include "util.hpp"
#define CONFIG_DEBUG_PPM_ASTAR 1


void layout(session* sess, design* ds, position* pos){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	int cnt_chip = ds->_chip.size();
	int cnt_connect = ds->_connect.size();
	printf("layout: %d,%d,%d,%d\n", cnt_po, cnt_pi, cnt_chip, cnt_connect);

	float fx,fy;

	//pinout at top
	for(int j=0;j<cnt_po;j++){
		fx = 1024*(float)(j+1)/(cnt_po+1);
		fy = 0.0;
		pos->_out.push_back({fx, fy, 0});
		printf("out %d: %f,%f\n", j, fx, fy);
	}

	//pinin at bot
	for(int j=0;j<cnt_pi;j++){
		fx = 1024*(float)(j+1)/(cnt_pi+1);
		fy = 1024*0.999;
		pos->_in.push_back({fx, fy, 0});
		printf("out %d: %f,%f\n", j, fx, fy);
	}

	//chip at middle
	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);
	int sz = 1024/(ce*2+1)/2;
	printf("sqrt=%f,ceil=%d\n", sq, ce);

	for(int j=0;j<cnt_chip;j++){
		fx = 1024*(float)((j%ce)*2+1.5)/(ce*2+1);
		fy = 1024*(float)((j/ce)*2+1.5)/(ce*2+1);
		printf("chip %d: %f,%f\n", j, fx, fy);
		pos->_chip.push_back({fx, fy, 0});
	}
	pos->_chipfoot.resize(cnt_chip);

	//each chip has many outpin, each outpin have and src and dst
	printf("generate chipviewwire\n");
	int delta;
	std::vector<std::string> pinglobal;
	for(int j=0;j<cnt_connect;j++){
		//one chip
		int findchip = -1;
		for(int k=0;k<ds->_chip.size();k++){
			if(ds->_connect[j]->chipname == ds->_chip[k]->name){
				findchip = k;
				break;
			}
		}
		if(findchip<0){
			printf("chip not found\n");
			break;
		}

		//debug
		printf("index=%d name=%s model=%s\n", findchip, ds->_chip[findchip]->name.c_str(), ds->_chip[findchip]->cname.c_str());
		design* dsn = finddesign(sess, ds->_chip[findchip]->cname);
		if(dsn){
			for(auto& it : dsn->_layout){
				printf("name=%s pos=%f,%f\n", it.first.c_str(), it.second.x, it.second.y);
			}
		}

		//each pin
		int m;
		std::vector<wiredef::_pinpair>& pinpair = ds->_connect[j]->pinpair;
		std::vector<pindef*>& po = ds->_pinout;
		std::vector<pindef*>& pi = ds->_pinin;
		std::vector<chipfootpin> cfl;
		for(int k=0;k<pinpair.size();k++){
			std::string& nickname = pinpair[k].nickname;
			std::string& origname = pinpair[k].origname;
			printf("->%d,%d,nickname=%s,origname=%s\n", j, k, nickname.c_str(), origname.c_str());
			int find = 0;
			if(dsn){
				auto it = dsn->_layout.find(origname);		//find foot in chip
				if(it != dsn->_layout.end()){
					posxyz& pos = it->second;
					fx = sz * pos.x;
					fy = -sz * pos.y;
					find = true;
					printf("debug:%f,%f,%f,%f, %d\n", pos.x, pos.y, fx, fy, sz);
				}
			}
			if(0 == find){
				fx = -sz + (float)(k+1) * (sz*2) / (pinpair.size()+1);		//must convert to float, or crash
				fy = -sz + 1;
			}
			pos->_chipfoot[findchip].push_back({fx, fy, 0.0});

			//pinout
			for(m=0;m<po.size();m++){
				if(nickname == po[m]->name){
					printf("%d: chip%d.foot%d-po%d\n", j, findchip, k, m);
					cfl.push_back({findchip, k, m});
					goto ok;
				}
			}
			//pinin
			for(m=0;m<pi.size();m++){
				if(nickname == pi[m]->name){
					printf("%d: chip%d.foot%d-pi%d\n", j, findchip, k, m);
					cfl.push_back({findchip, k, cnt_po+m});
					goto ok;
				}
			}

			//pinglobal found
			for(m=0;m<pinglobal.size();m++){
				if(nickname == pinglobal[m]){
					printf("%d: chip%d.foot%d-pg%d\n", j, findchip, k, m);
					cfl.push_back({findchip, k, cnt_po+cnt_pi+m});
					goto ok;
				}
			}
			//pinglobal notfound
			m = pinglobal.size();
			pinglobal.push_back(nickname);
			printf("%d: chip%d.foot%d-pg%d\n", j, findchip, k, m);
			cfl.push_back({findchip, k, cnt_po+cnt_pi+m});
/*
			//power
			printf("%d: chip%d.foot%d-p%d : %f,%f\n", j, findchip, k, cnt_po+cnt_pi, fx, fy);
			cfl.push_back({findchip, k, cnt_po+cnt_pi});
*/
ok:
			continue;
		}
		pos->chipviewwire.push_back(cfl);
	}

	//pinglobal at left
	int cnt_pg = pinglobal.size();
	for(int j=0;j<cnt_pg;j++){
		fx = 0;
		fy = 1024*(float)(j+1)/(cnt_pg+1);
		pos->_pin_global.push_back({fx, fy, 0});
		printf("pin_global %d: %f,%f\n", j, fx, fy);
	}

	printf("convert chipviewwire to pinviewwire\n");
	pos->pinviewwire.resize(cnt_po+cnt_pi+cnt_pg);
	for(int j=0;j<pos->chipviewwire.size();j++){
		for(int k=0;k<pos->chipviewwire[j].size();k++){
			int pinid = pos->chipviewwire[j][k].pinid;
			pos->pinviewwire[pinid].push_back(pos->chipviewwire[j][k]);
			printf("chipview %d,%d: pin=%d\n", j, k, pinid);
		}
	}
	for(int j=0;j<pos->pinviewwire.size();j++){
		for(int k=0;k<pos->pinviewwire[j].size();k++){
			printf("pinview %d,%d: chip=%d,foot=%d\n", j, k, pos->pinviewwire[j][k].chipid, pos->pinviewwire[j][k].footid);
		}
	}
}
