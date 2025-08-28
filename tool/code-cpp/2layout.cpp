#include "util.hpp"
#define CONFIG_DEBUG_PPM_ASTAR 1

int findpin(design* userchip, std::string nickname)
{
	int m;
	std::vector<pindef*>& po = userchip->_pinout;
	std::vector<pindef*>& pi = userchip->_pinin;
	std::vector<pindef*>& pg = userchip->_pinglobal;
	int cnt_po = po.size();
	int cnt_pi = pi.size();

	//pinout
	for(m=0;m<po.size();m++){
		if(nickname == po[m]->name){
			//printf("%d: chip%d.foot%d-po%d\n", j, findchip, k, m);
			//cfl.push_back({findchip, k, m});
			//goto ok;
			return m;
		}
	}
	//pinin
	for(m=0;m<pi.size();m++){
		if(nickname == pi[m]->name){
			//printf("%d: chip%d.foot%d-pi%d\n", j, findchip, k, m);
			//cfl.push_back({findchip, k, cnt_po+m});
			//goto ok;
			return cnt_po + m;
		}
	}

	//pinglobal found
	for(m=0;m<pg.size();m++){
		if(nickname == pg[m]->name){
			//printf("%d: chip%d.foot%d-pg%d\n", j, findchip, k, m);
			//cfl.push_back({findchip, k, cnt_po+cnt_pi+m});
			//goto ok;
			return cnt_po + cnt_pi + m;
		}
	}

	//pinglobal notfound
	m = pg.size();
	pg.push_back(new pindef(nickname));
	//printf("%d: chip%d.foot%d-pg%d\n", j, findchip, k, m);
	return cnt_po + cnt_pi + m;
}
void layout_usedchip(session* sess, design* userchip, int chipidx, position* pos){
	int cnt_chip = userchip->_chip.size();

	std::string chipname_nick = userchip->_chip[chipidx]->name;
	std::string chipname_orig = userchip->_chip[chipidx]->cname;

	//chip position
	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);
	int sz = 1024/(ce*2+1)/2;
	printf("sqrt=%f,ceil=%d\n", sq, ce);

	bool gotpos = 0;
	float fx, fy;
	auto it = userchip->_layout.find(chipname_nick);
	if(it != userchip->_layout.end()){
		gotpos = true;
		fx = 512 + 512*it->second.x;
		fy = 512 - 512*it->second.y;
		printf("chippos=%f,%f\n", fx, fy);
	}
	if(!gotpos){
		fx = 1024*(float)((chipidx%ce)*2+1.5)/(ce*2+1);
		fy = 1024*(float)((chipidx/ce)*2+1.5)/(ce*2+1);
	}
	printf("chip[%d]: nick=%s orig=%s pos=%f,%f\n",
		chipidx, chipname_nick.c_str(), chipname_orig.c_str(), fx, fy);
	pos->_chip.push_back({fx, fy, 0});


	//pin position
	wiredef* wd = findwire(userchip, chipname_nick);
	if(0 == wd)return;

	design* origchip = finddesign(sess, chipname_orig);
	if(0 == origchip)return;

	int k;
	std::vector<chipfootpin> cfp;
	for(int k=0;k<origchip->_pinout.size();k++){
		std::string pinname_orig = origchip->_pinout[k]->name.c_str();
		printf("k=%d name=%s\n", k, pinname_orig.c_str());

		//is this pin used?
		bool iswire = 0;
		bool isused = 0;
		std::string pinname_nick;
		std::vector<wiredef::_pinpair>& pinpair = wd->pinpair;
		if(pinname_orig.size() > 0){
			isused = true;
			for(auto& pp : pinpair){
				if(pp.origname == pinname_orig){
					printf("$$$$$$$$$$$$$$$$$$\n");
					pinname_nick = pp.nickname;
					iswire = true;
					break;
				}
			}
		}
		else if(origchip->_pinout.size() == wd->pinpair.size()){
			isused = true;
			iswire = true;
			pinname_nick = pinpair[k].nickname;
			pinname_orig = pinpair[k].origname;
		}
		else{
			isused = 0;
			iswire = 0;
		};
		printf("nickname=%s origname=%s\n", pinname_nick.c_str(), pinname_orig.c_str());

		//locate it
		bool located = 0;
		if(isused){
			auto it = origchip->_layout.find(pinname_orig);  //find foot in chip
			if(it != origchip->_layout.end()){
				posxyz& pos = it->second;
				fx = sz * pos.x;
				fy = -sz * pos.y;
				located = true;
				printf("user pos:%f,%f,%f,%f sz=%d\n", pos.x, pos.y, fx, fy, sz);
			}
		}
		if(0 == located){
			fx = -sz + (float)(k+1) * (sz*2) / (origchip->_pinout.size()+1);		//must convert to float, or crash
			fy = -sz + 1;
			printf("test pos:%f,%f sz=%d\n", fx, fy, sz);
		}

		//put every pin position in vector
		pos->_chipfoot[chipidx].push_back({fx, fy, 0.0});

		//handle chip-foot-pin
		if(iswire){
			int pin = findpin(userchip, pinname_nick);
			cfp.push_back({chipidx, k, pin});
		}
	}
	pos->chipviewwire.push_back(cfp);
}
void layout(session* sess, design* ds, position* pos){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	int cnt_chip = ds->_chip.size();
	int cnt_connect = ds->_connect.size();
	printf("layout: %d,%d,%d,%d\n", cnt_po, cnt_pi, cnt_chip, cnt_connect);

	//foreach chip
	printf("chipview: generate pos\n");
	pos->_chipfoot.resize(cnt_chip);
	for(int j=0;j<cnt_chip;j++){
		layout_usedchip(sess, ds, j, pos);
	}

	float fx,fy;

	//pinout at top
	for(int j=0;j<cnt_po;j++){
		bool located = 0;
		auto it = ds->_layout.find(ds->_pinout[j]->name);  //find foot in chip
		if(it != ds->_layout.end()){
			posxyz& pos = it->second;
			fx = 512 + 512 * pos.x;
			fy = 512 - 512 * pos.y;
			located = true;
			printf("user pos:%f,%f,%f,%f\n", pos.x, pos.y, fx, fy);
		}
		if(!located){
			fx = 1024*(float)(j+1)/(cnt_po+1);
			fy = 0.0;
		}
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

	//pinglobal at left
	int cnt_pg = ds->_pinglobal.size();
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
