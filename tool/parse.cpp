#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
//
#include<iostream>
#include<vector>
#include<algorithm>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int u64;


#define CONFIG_PRINT_CONSTRUCT 1
#define CONFIG_PRINT_DESTRUCT 0
#define CONFIG_PRINT_HIDE_COMMENTED 0
//
#define CONFIG_DEBUG_PPM_ASTAR 1


class pindef{
public:
	pindef(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_buflen pindef %.*s\n", len,buf);
#endif
		name = std::string((char*)buf, len);
	}
	pindef(std::string s){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_string pindef %s\n", s.c_str());
#endif
		name = s;
	}
	pindef(pindef* c){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_copy pindef %s\n", c->name.c_str());
#endif
		name = c->name;
	}
	~pindef(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete pindef %s\n", name.c_str());
#endif
	}
	std::string name;
};

class chipdef{
public:
	chipdef(u8* buf, int len, u8* cb, int cl){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_buflen chipdef %.*s %.*s\n", len,buf, cl,cb);
#endif
		name = std::string((char*)buf, len);
		cname = std::string((char*)cb, cl);
	}
	chipdef(std::string s0, std::string s1){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_string chipdef %s %s\n", s0.c_str(), s1.c_str());
#endif
		name = s0;
		cname = s1;
	}
	chipdef(chipdef* c){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_copy chipdef %s %s\n", c->name.c_str(), c->cname.c_str());
#endif
		name = c->name;
		cname = c->cname;
	}
	~chipdef(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete chipdef %s %s\n", name.c_str(), cname.c_str());
#endif
	}
	std::string name;
	std::string cname;
};

class wiredef{
public:
	wiredef(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_buflen wiredef %.*s\n", len,buf);
#endif
		name = std::string((char*)buf, len);
	}
	wiredef(std::string s){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_string wiredef %s\n", s.c_str());
#endif
		name = s;
	}
	wiredef(wiredef* c){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_copy wiredef %s\n", c->name.c_str());
#endif
		name = c->name;
		for(auto p : c->pinname)pinname.push_back(p);
	}
	~wiredef(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete wiredef %s\n", name.c_str());
#endif
	}
	void addpin(std::string s){
		pinname.push_back(s);
	}
	void addpin(u8* buf, int len){
		int j;
		for(j=0;j<len;j++){
			if('@' == buf[j]){
				buf += j+1;
				len -= j+1;
				break;
			}
		}
		pinname.push_back(std::string((char*)buf, len));
	}
	std::string name;
	std::vector<std::string> pinname;
};

class design{
public:
	design(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_buflen design %.*s\n", len,buf);
#endif
		name = std::string((char*)buf, len);
	}
	design(design* c){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_copy design %s\n", c->name.c_str());
#endif
		name = c->name;
		for(auto p : c->_pinout)_pinout.push_back(new pindef(p));
		for(auto p : c->_pinin)_pinin.push_back(new pindef(p));
		for(auto p : c->_chip)_chip.push_back(new chipdef(p));
		for(auto p : c->_logic)_logic.push_back(new wiredef(p));
	}
	~design(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete design %s\n", name.c_str());
#endif
		for(auto p : _pinout)delete(p);
		for(auto p : _pinin)delete(p);
		for(auto p : _chip)delete(p);
		for(auto p : _logic)delete(p);
		//_pinout.resize(0);
		//_pinin.resize(0);
		//_chip.resize(0);
		//_logic.resize(0);
	}
	void shrinkunused(){
		for(auto it = _chip.begin();it != _chip.end();){
			if('/' == (*it)->name.c_str()[0]){
				delete (*it);
				it = _chip.erase(it);
			}
			else{
				it++;
			}
		}
		for(auto it = _logic.begin();it != _logic.end();){
			if('/' == (*it)->name.c_str()[0]){
				delete (*it);
				it = _logic.erase(it);
			}
			else{
				it++;
			}
		}
	}
	std::string name;
	std::vector<pindef*> _pinout;
	std::vector<pindef*> _pinin;
	std::vector<chipdef*> _chip;
	std::vector<wiredef*> _logic;
};

class filecontext{
public:
	filecontext(u8* buf){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_buf filectx %s\n", buf);
#endif
		filename = std::string((char*)buf);
	}
	~filecontext(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete filectx %s\n", filename.c_str());
#endif
		for(auto p : cx)delete(p);
	}
	std::string filename;
	std::vector<design*> cx;
};

class session{
public:
	session(){
#if CONFIG_PRINT_CONSTRUCT==1
		printf("contruct_noarg session\n");
#endif
	}
	~session(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete session\n");
#endif
		for(auto p : file)delete(p);
	}
	std::vector<filecontext*> file;
};




void parse(filecontext* ctx, u8* buf, int len){
	design* comp = 0;
	pindef* pin = 0;
	chipdef* ch = 0;
	wiredef* wi = 0;

	int incomment = 0;
	int linecount = 0;
	int bracketcount = 0;
	int currsection = 0;
#define PINOUT 1
#define PININ 2
#define CHIP 3
#define LOGIC 4
	int firstbyte = 0;
	int firststr = -1;
	int firstmaohao = -1;		//:
	int firstkuohao = -1;		//(

	int j;
	for(j=0;j<len;j++){
		//printf("debug: line=%d\n",linecount+1);
		if(0){
		}
		else if('/' == buf[j]){
			if('*' == buf[j+1]){
				incomment = 1;
				j = j+1;
				continue;
			}
			if('/' == buf[j+1]){
				while('\n' != buf[j+1])j++;
				continue;
			}
		}
		else if('*' == buf[j]){
			if('/' == buf[j+1]){
				incomment = 0;
			}
		}
		else if('\n' == buf[j]){
			if(incomment)continue;
			else if(firststr>=0){
				printf("j=%x line=%d brack=%d token=lf wholeline=%d<%.*s>\n", j, linecount+1, bracketcount, j-firststr, j-firststr, buf+firststr);
				if(PINOUT == currsection){
					int strend = j;
					if(firstmaohao >= 0){
						strend = firstmaohao;
					}
					while( (' '==buf[strend-1]) | ('\t'==buf[strend-1]) )strend--;
					pin = new pindef(buf+firststr, strend-firststr);
					comp->_pinout.push_back(pin);
				}
				else if(PININ == currsection){
					int strend = j;
					if(firstmaohao >= 0){
						strend = firstmaohao;
					}
					while( (' '==buf[strend-1]) | ('\t'==buf[strend-1]) )strend--;
					pin = new pindef(buf+firststr, strend-firststr);
					comp->_pinin.push_back(pin);
				}
				else if(CHIP == currsection){
					/*
					c0, c1, c2, c3 : stm32
					*/
					//" stm32"
					u8* qb = buf+firstmaohao+1;
					while( (' '==*qb) | ('\t'==*qb) )qb++;
					int ql = 0;
					while( (' '!=qb[ql]) && ('\t'!=qb[ql]) && ('\n'!=qb[ql]) )ql++;

					//"c0, c1, c2, c3 "
					u8* p = buf+firststr;
					int k;
					int now = 0;
					int round = 0;
					for(k=0;k<firstmaohao-firststr+1;k++){
					switch(p[k]){
					case ':':
					case ',':
					case ' ':
					case '\t':
						if(1 == round){
							ch = new chipdef(p+now, k-now, qb, ql);
							comp->_chip.push_back(ch);
							round = !round;
						}
						break;
					default:
						if(0 == round){
							now = k;
							round = !round;
						}
					}
					}
				}
				else if(LOGIC == currsection){
					wi = new wiredef(buf+firststr, firstkuohao-firststr);
					/*
					p0(vcc, oa, en0)
					*/
					u8* p = buf+firstkuohao+1;
					int k;
					int now = 0;
					int round = 0;
					for(k=0;k<j-firstkuohao;k++){
					switch(p[k]){
					case ',':
					case ' ':
					case '\t':
					case ')':
						if(1 == round){
							wi->addpin(p+now, k-now);
							round = !round;
						}
						if(')'==p[k])k=j-firstkuohao;	//big break
						break;
					default:
						if(0 == round){
							now = k;
							round = !round;
						}
					}
					}

					comp->_logic.push_back(wi);
				}
			}
			linecount++;
			firstbyte = j+1;
			firststr = -1;
			firstmaohao = -1;
			firstkuohao = -1;
		}
		else if(incomment){
			//do nothing
		}
		else if('{' == buf[j]){
			if(firststr>=0){
				printf("j=%x line=%d brack=%d token={ name=%d<%.*s>\n", j, linecount+1, bracketcount, j-firststr, j-firststr, buf+firststr);
				if(0 == bracketcount){
					comp = new design(buf+firststr, j-firststr);
					ctx->cx.push_back(comp);
				}
				else if(1 == bracketcount){
					if(strncmp("pinout", (char*)buf+firststr, j-firststr) == 0){
						currsection = PINOUT;
					}
					else if(strncmp("pinin", (char*)buf+firststr, j-firststr) == 0){
						currsection = PININ;
					}
					else if(strncmp("chip", (char*)buf+firststr, j-firststr) == 0){
						currsection = CHIP;
					}
					else if(strncmp("logic", (char*)buf+firststr, j-firststr) == 0){
						currsection = LOGIC;
					}
				}
				firststr = -1;
			}
			bracketcount++;
		}
		else if('}' == buf[j]){
			printf("j=%x line=%d brack=%d token=}\n", j, linecount+1, bracketcount);
			bracketcount--;
			currsection = 0;
		}
		else if('(' == buf[j]){
			if(firststr>=0){
				printf("j=%x line=%d brack=%d token=( name=%d<%.*s>\n", j, linecount+1, bracketcount, j-firststr, j-firststr, buf+firststr);
				firstkuohao = j;
			}
		}
		else if(':' == buf[j]){
			if(firststr>=0){
				printf("j=%x line=%d brack=%d currsec=%d token=: name=%d<%.*s>\n", j, linecount+1, bracketcount, currsection, j-firststr, j-firststr, buf+firststr);
				firstmaohao = j;
			}
		}
		else if(
			(('a' <= buf[j]) && ('z' >= buf[j])) |
			(('A' <= buf[j]) && ('Z' >= buf[j])) |
			('_' == buf[j])    ){
			if(firststr<0)firststr = j;
		}
		else{
		}
	}//for
}




void printdesign(design* c){
	printf("%s{\n", c->name.c_str());

	int cnt = 0;
	printf("pinout:size=%ld\n", c->_pinout.size());
	for(auto p : c->_pinout){
		printf("	%s\n", p->name.c_str());
		cnt++;
	}
	printf("pinout:cnt=%d\n", cnt);

	cnt = 0;
	printf("pinin:size=%ld\n", c->_pinin.size());
	for(auto p : c->_pinin){
		printf("	%s\n", p->name.c_str());
		cnt++;
	}
	printf("pinin:cnt=%d\n", cnt);

	cnt = 0;
	printf("chip:size=%ld\n", c->_chip.size());
	for(auto p : c->_chip){
		if('/' != p->name.c_str()[0]){
			cnt++;
		}
#if CONFIG_PRINT_HIDE_COMMENTED==1
		else{
			continue;
		}
#endif
		printf("	%s : %s\n", p->name.c_str(), p->cname.c_str());
	}
	printf("chip:cnt=%d\n", cnt);

	cnt = 0;
	printf("logic:size=%ld\n", c->_logic.size());
	for(auto p : c->_logic){
		if('/' != p->name.c_str()[0]){
			cnt++;
		}
#if CONFIG_PRINT_HIDE_COMMENTED==1
		else{
			continue;
		}
#endif
		printf("	%s(", p->name.c_str());
		for(auto t : p->pinname)printf("%s%c", t.c_str(), (t==p->pinname.back()) ? ')' : ' ');
		printf("\n");
	}
	printf("logic:cnt=%d\n", cnt);

	printf("}\n");
}
void printfilectx(filecontext* ctx){
	printf("filename=%s\n", ctx->filename.c_str());
	printf("\n");
	for(auto c : ctx->cx){
		printdesign(c);
		printf("\n");
	}
}
void printsession(session* sess){
	for(auto f : sess->file)printfilectx(f);
}




std::string translatepinname(std::string s0, design* in, wiredef* wi)
{
	//1 pinout: name unchanged
	int j;
	for(j=0;j<in->_pinout.size();j++){
		if(s0 == in->_pinout[j]->name){
			return wi->pinname[j];
		}
	}

	//2 pinin: refname/pinname
	for(j=0;j<in->_pinin.size();j++){
		if(s0 == in->_pinin[j]->name){
			return wi->name + "/" + s0;
		}
	}

	//3 notfound: maybe vcc,gnd...
	return "?" + s0;
}
void expand_real(design* out, design* in, wiredef* wi){
	printf("in: name=%s\n", in->name.c_str());
	printf("wi: name=%s, pin=%s...\n", wi->name.c_str(), wi->pinname[0].c_str());

	//inner pin: rename and insert
	for(auto p : in->_pinin){
		out->_pinin.push_back(new pindef(wi->name + "/" + p->name));
	}

	//inner chip: rename and insert
	for(auto p : in->_chip){
		out->_chip.push_back(new chipdef(wi->name + "/" + p->name, p->cname));
	}

	//inner logic: ???
	for(auto p : in->_logic){
		wiredef* tmp = new wiredef(wi->name + "/" + p->name);

		for(auto q : p->pinname){
			std::string s = translatepinname(q, in, wi);
			tmp->addpin(s);
		}

		out->_logic.push_back(tmp);
	}
}

void expand_onelayer(session* sess, design* ds, int last){
	int sz = ds->_chip.size();
	int j;
	int foundinsess = -1;
	int foundinfile = -1;
	int foundinlogic = -1;
	for(j=last;j<sz;j++){
		printf("cname=%s{{\n", ds->_chip[j]->cname.c_str());

		foundinsess = -1;
		foundinfile = -1;
		int k,f;
		for(f=0;f<sess->file.size();f++){
			filecontext* ctx = sess->file[f];
			for(k=0;k<ctx->cx.size();k++){
				if(ds->_chip[j]->cname == ctx->cx[k]->name){
					if(ctx->cx[k]->_chip.size() == 0){
						printf("this is already base element\n");
						continue;
					}
					printf("at file %d, design %d\n", f, k);
					foundinsess = f;
					foundinfile = k;
					break;
				}
			}
		}
		if(foundinfile < 0){
			printf("not found in file\n");
			goto prepnext;
		}

		foundinlogic = -1;
		for(k=0;k<ds->_logic.size();k++){
			if(ds->_chip[j]->name == ds->_logic[k]->name){
				printf("at logic %d\n", k);
				foundinlogic = k;
				break;
			}
		}
		if(foundinlogic < 0){
			printf("not found in logic\n");
			goto prepnext;
		}

		expand_real(ds, sess->file[foundinsess]->cx[foundinfile], ds->_logic[foundinlogic]);

		//comment this chip and this wire
		ds->_chip[j]->name = "//" + ds->_chip[j]->name;
		ds->_logic[foundinlogic]->name = "//" + ds->_logic[foundinlogic]->name;

prepnext:
		printf("}}\n");
	}
}

design* expand(session* sess, std::string name){
	int j;
	design* found = 0;
	for(auto ctx : sess->file){
		for(j=0;j<ctx->cx.size();j++){
			if(name == ctx->cx[j]->name){
				found = ctx->cx[j];
			}
		}
	}
	if(0 == found)return 0;
	printf("\n");

	design* theone = new design(found);
	int prev = 0;
	int curr = 0;
	for(int j=0;j<3;j++){
		curr = theone->_chip.size();
		expand_onelayer(sess, theone, prev);
		prev = curr;
	}
	printf("\n");

	printdesign(theone);

	theone->shrinkunused();

	printdesign(theone);

	//delete theone;
	return theone;
}




struct posxyz{
	float x,y,z;
};
struct chipfootpin{
	//float sx,sy;
	int chipid;
	int footid;
	int pinid;
};
class position{
public:
	std::vector<posxyz> _out;
	std::vector<posxyz> _in;
	//
	std::vector<posxyz> _chip;
	std::vector<std::vector<posxyz>> _chipfoot;
	//
	std::vector<std::vector<chipfootpin>> pinviewwire;		//[pin][conn][cid,fid,pid]
	//
	std::vector<std::vector<chipfootpin>> chipviewwire;		//[chip][foot][cid,fid,pid]
};
void layout(design* ds, position* pos){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	int cnt_chip = ds->_chip.size();
	int cnt_logic = ds->_logic.size();
	printf("layout: %d,%d,%d,%d\n", cnt_po, cnt_pi, cnt_chip, cnt_logic);

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
	int sz = 1024/ce/4;
	printf("sqrt=%f,ceil=%d\n", sq, ce);

	for(int j=0;j<cnt_chip;j++){
		fx = 1024*(float)((j%ce)+1)/(ce+1);
		fy = 1024*(float)((j/ce)+1)/(ce+1);
		printf("chip %d: %f,%f\n", j, fx, fy);
		pos->_chip.push_back({fx, fy, 0});
	}
	pos->_chipfoot.resize(cnt_chip);

	//each chip has many outpin, each outpin have and src and dst
	printf("generate chipviewwire\n");
	int delta;
	for(int j=0;j<cnt_logic;j++){
		//one chip
		int findchip = -1;
		for(int k=0;k<ds->_chip.size();k++){
			if(ds->_logic[j]->name == ds->_chip[k]->name){
				findchip = k;
				break;
			}
		}
		if(findchip<0){
			printf("chip not found\n");
			break;
		}

		//each pin
		std::vector<std::string> pinname = ds->_logic[j]->pinname;
		std::vector<pindef*> po = ds->_pinout;
		std::vector<pindef*> pi = ds->_pinin;
		std::vector<chipfootpin> cfl;
		for(int k=0;k<pinname.size();k++){
			fx = -sz + (float)(k+1) * (sz*2) / (pinname.size()+1);		//must convert to float, or crash
			fy = -sz + 1;
			pos->_chipfoot[findchip].push_back({fx, fy, 0.0});

			//pinout
			for(int m=0;m<po.size();m++){
				if(pinname[k] == po[m]->name){
					printf("%d: chip%d.foot%d-pi%d : %f,%f\n", j, findchip, k, m, fx, fy);
					cfl.push_back({findchip, k, m});
					goto ok;
				}
			}
			//pinin
			for(int m=0;m<pi.size();m++){
				if(pinname[k] == pi[m]->name){
					printf("%d: chip%d.foot%d-pi%d : %f,%f\n", j, findchip, k, m, fx, fy);
					cfl.push_back({findchip, k, cnt_po+m});
					goto ok;
				}
			}
			//power
			printf("%d: chip%d.foot%d-p%d : %f,%f\n", j, findchip, k, cnt_po+cnt_pi, fx, fy);
			cfl.push_back({findchip, k, cnt_po+cnt_pi});
ok:
			continue;
		}
		pos->chipviewwire.push_back(cfl);
	}

	printf("convert chipviewwire to pinviewwire\n");
	pos->pinviewwire.resize(cnt_po+cnt_pi+1);
	for(int j=0;j<pos->chipviewwire.size();j++){
		for(int k=0;k<pos->chipviewwire[j].size();k++){
			int pinid = pos->chipviewwire[j][k].pinid;
			pos->pinviewwire[pinid].push_back(pos->chipviewwire[j][k]);
		}
	}
	for(int j=0;j<pos->pinviewwire.size();j++){
		for(int k=0;k<pos->pinviewwire[j].size();k++){
			printf("%d,%d: %d,%d\n", j, k, pos->pinviewwire[j][k].chipid, pos->pinviewwire[j][k].footid);
		}
	}
}



u32 hsv_to_rgb(float h, float s, float v) {
	float c = v * s;
	float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
	float m = v - c;

	float r1, g1, b1;
	if (h >= 0 && h < 60) {
		r1 = c;
		g1 = x;
		b1 = 0;
	} else if (h >= 60 && h < 120) {
		r1 = x;
		g1 = c;
		b1 = 0;
	} else if (h >= 120 && h < 180) {
		r1 = 0;
		g1 = c;
		b1 = x;
	} else if (h >= 180 && h < 240) {
		r1 = 0;
		g1 = x;
		b1 = c;
	} else if (h >= 240 && h < 300) {
		r1 = x;
		g1 = 0;
		b1 = c;
	} else {
		r1 = c;
		g1 = 0;
		b1 = x;
	}

	u32 ir = (u32)((r1 + m) * 255);
	u32 ig = (u32)((g1 + m) * 255);
	u32 ib = (u32)((b1 + m) * 255);
	return (ir<<16) | (ig<<8) | ib;
}
std::vector<u32> buildcolortable(int size, int special){
	std::vector<u32> tab;
	for(int j=0;j<size;j++){
		tab.push_back(hsv_to_rgb(360.0*j/size, 1.0, 1.0));
	}
	if(special)tab.push_back(0xffffff);
	return tab;
}
void writeppm(unsigned char* buf, int pitch, int w, int h, const char* name)
{
	printf("buf=%p,pitch=0x%x,w=%d,h=%d\n", buf, pitch, w, h);
	FILE* fp = fopen(name, "wb");

	char tmp[0x100];
	int ret = snprintf(tmp, 0x100, "P6\n%d\n%d\n255\n", w, h);
	fwrite(tmp, 1, ret, fp);

	int x,y;
	unsigned char* row;
	for(y = 0; y < h; y++) {
		row = buf + pitch*y;
		for(x = 0; x < w; x++) {
			fwrite(row+2, 1, 1, fp);
			fwrite(row+1, 1, 1, fp);
			fwrite(row+0, 1, 1, fp);
			row += 4;
		}
	}

	fclose(fp);
}




void drawcolor(u8* pix, u32 rgb)
{
	int width = 1024;
	int height = 1024;
	u32* buf = (u32*)pix;
	for(int j=0;j<width*height;j++)buf[j] = rgb;
}
void drawline(u8* pix, u32 rgb, int x0, int y0, int x1, int y1)
{
	int width = 1024;
	int height = 1024;
	int stride = 1024;
	u32* buf = (u32*)pix;

	int dx,dy,sx,sy,e1,e2;
	if(x0 < x1){dx = x1-x0;sx = 1;}
	else {dx = x0-x1;sx = -1;}
	if(y0 < y1){dy = y1-y0;sy = 1;}
	else {dy = y0-y1;sy = -1;}
	if(dx > dy){e1 = dx/2;}
	else {e1 = -dy/2;}

	rgb |= 0xff000000;
	while(1)
	{
		if((x0 == x1)&&(y0 == y1))break;

		if((x0 >= 0)&&(x0 < width)&&(y0 >= 0)&&(y0 < height))
		{buf[(y0*stride) + x0] = rgb;}

		e2 = e1;
		if(e2 >-dx){e1 -= dy;x0 += sx;}
		if(e2 < dy){e1 += dx;y0 += sy;}
	}
}
void drawline_rect(u8* pix, u32 rgb, int x1, int y1, int x2, int y2)
{
	int width = 1024;
	int height = 1024;
	int stride = 1024;
	u32* buf = (u32*)pix;

	int x,y,n;
	int startx,endx,starty,endy;
	if(x1<x2){startx=x1;endx=x2;}
	else{startx=x2;endx=x1;}
	if(y1<y2){starty=y1;endy=y2;}
	else{starty=y2;endy=y1;}
	//logtoall("(%x,%x),(%x,%x)\n",startx,starty,endx,endy);

	for(n=0;n<1;n++)
	{
		if((starty+n >= 0) && (starty+n < height))
		{
			for(x=startx;x<endx;x++)
			{
				if(x >= width)break;
				if(x < 0)x=0;
				buf[((starty+n)*stride) + x] = rgb;
			}
		}
		if((endy-n >= 0) && (endy-n < height))
		{
			for(x=startx;x<endx;x++)
			{
				if(x > width-1)break;
				if(x < 0)x=0;
				buf[((endy-n-1)*stride) + x] = rgb;
			}
		}
		if((startx+n >= 0) && (startx+n < width))
		{
			for(y=starty;y<endy;y++)
			{
				if(y >= height)break;
				if(y < 0)y = 0;
				buf[(y*stride) + startx+n] = rgb;
			}
		}
		if((endx-n >= 0) && (endx-n < width))
		{
			for(y=starty;y<endy;y++)
			{
				if(y >= height)break;
				if(y < 0)y = 0;
				buf[(y*stride) + endx-n] = rgb;
			}
		}
	}
}
void drawsolid_rect(u8* pix, u32 rgb, int x1, int y1, int x2, int y2)
{
	//printf("%s:%d,%d,%d,%d\n",__FUNCTION__, x1,y1, x2, y2);
	int width = 1024;
	int height = 1024;
	int stride = 1024;
	u32* buf = (u32*)pix;

	int startx,endx;
	if(x1<x2){startx=x1;endx=x2;}
	else{startx=x2;endx=x1;}
	if(startx<0)startx = 0;
	if(endx>1024)endx = 1024;

	int starty,endy;
	if(y1<y2){starty=y1;endy=y2;}
	else{starty=y2;endy=y1;}
	if(starty<0)starty = 0;
	if(endy>1024)endy = 1024;

	int x,y;
	for(y=starty;y<endy;y++){
		u32* tmp = &buf[y*stride];
		for(x=startx;x<endx;x++)tmp[x] = rgb;
	}
}
void drawchip(design* ds, position* pos, u8* pix){
	int cnt_chip = ds->_chip.size();
	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);

	int ix,iy,sz;
	for(int j=0;j<cnt_chip;j++){
		ix = pos->_chip[j].x;
		iy = pos->_chip[j].y;
		sz = 1024/ce/4;
		drawline_rect(pix, 0x888888, ix-sz, iy-sz, ix+sz, iy+sz);
	}
}
void drawfoot(design* ds, position* pos, u8* pix){
	int cnt_chip = ds->_chip.size();
	int ix,iy;
	for(int j=0;j<cnt_chip;j++){
		for(int k=0;k<pos->_chipfoot[j].size();k++){
			ix = pos->_chip[j].x + pos->_chipfoot[j][k].x;
			iy = pos->_chip[j].y + pos->_chipfoot[j][k].y;
			drawsolid_rect(pix, 0x888888, ix-3, iy-3, ix+3, iy+3);
		}
	}
}
void drawpinout(design* ds, position* pos, u8* pix){
	int cnt_po = ds->_pinout.size();
	int ix,iy;
	for(int j=0;j<cnt_po;j++){
		for(int k=0;k<pos->_out.size();k++){
			ix = pos->_out[j].x;
			iy = pos->_out[j].y;
			drawsolid_rect(pix, 0x888888, ix-3, iy-3, ix+3, iy+3);
		}
	}
}




void drawwire_chipview(design* ds, position* pos, u8* pix){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	std::vector<u32> colortable = buildcolortable(cnt_po+cnt_pi, 1);

	int ix,iy;
	int ox,oy;
	for(int j=0;j<pos->chipviewwire.size();j++){
		for(int k=0;k<pos->chipviewwire[j].size();k++){
			int chipid = pos->chipviewwire[j][k].chipid;
			int footid = pos->chipviewwire[j][k].footid;
			ix = pos->_chip[chipid].x + pos->_chipfoot[chipid][footid].x;
			iy = pos->_chip[chipid].y + pos->_chipfoot[chipid][footid].y;
			//
			int pinid = pos->chipviewwire[j][k].pinid;
			if(pinid < cnt_po){		//in pinout
				ox = pos->_out[pinid].x;
				oy = pos->_out[pinid].y;
			}
			else if(pinid < cnt_po+cnt_pi){		//in pinin
				ox = pos->_in[pinid-cnt_po].x;
				oy = pos->_in[pinid-cnt_po].y;
			}
			else{		//maybe power pin
				ox = ix;
				oy = iy-10;
			}
			//
			printf("%d,%d : chip%d.foot%d-pin%d : %d,%d - %d,%d\n", j,k, chipid,footid,pinid, ix, iy, ox, oy);
			drawline(pix, colortable[pinid], ix,iy, ox,oy);
		}
	}
}




void drawwire_pinview(design* ds, position* pos, u8* pix){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	std::vector<u32> colortable = buildcolortable(cnt_po+cnt_pi, 1);

	int ix,iy;
	int ox,oy;
	for(int j=0;j<pos->pinviewwire.size()-1;j++){
		int pinid = j;
		if(pinid < cnt_po){		//in pinout
			ox = pos->_out[pinid].x;
			oy = pos->_out[pinid].y;
		}
		else if(pinid < cnt_po+cnt_pi){		//in pinin
			ox = pos->_in[pinid-cnt_po].x;
			oy = pos->_in[pinid-cnt_po].y;
		}
		else{		//maybe power pin
			ox = ix;
			oy = iy-10;
		}
		for(int k=0;k<pos->pinviewwire[j].size();k++){
			int chipid = pos->pinviewwire[j][k].chipid;
			int footid = pos->pinviewwire[j][k].footid;
			ix = pos->_chip[chipid].x + pos->_chipfoot[chipid][footid].x;
			iy = pos->_chip[chipid].y + pos->_chipfoot[chipid][footid].y;//
			printf("%d,%d : chip%d.foot%d-pin%d : %d,%d - %d,%d\n", j,k, chipid,footid,pinid, ix, iy, ox, oy);
			if( (pinid<cnt_po) | (k>0) ){
			drawline(pix, colortable[pinid], ix,iy, ox,oy);
			}
			ox = ix;
			oy = iy;
		}
	}
}



#define CONFIG_NOT_CONNECTED_PIXEL 16
//
#define CONFIG_DILATE_CHIP_GAP 2
#define CONFIG_DILATE_WIRE_GAP 2
//
#define MAP_VAL_EMPTY 0
#define MAP_VAL_DILATE 0x000000ff
#define MAP_VAL_DONTDILATE 0x0000ff00	//this can only change to MAP_VAL_WIRE
#define MAP_WIRE_FIRST 0xff400000
#define MAP_WIRE_FINAL (MAP_WIRE_FIRST+0xffff)
#define MAP_CHIP_FIRST 0xffc00000
#define MAP_CHIP_FINAL (MAP_CHIP_FIRST+0xffff)
void drawmap_chip(design* ds, position* pos, u32* map){
	int cnt_chip = ds->_chip.size();
	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);

	int ix,iy,sz;
	for(int j=0;j<cnt_chip;j++){
		ix = pos->_chip[j].x;
		iy = pos->_chip[j].y;
		sz = 1024/ce/4;
		drawline_rect((u8*)map, MAP_CHIP_FIRST+j, ix-sz, iy-sz, ix+sz, iy+sz);
	}
}
void drawmap_dontdilate(u32* map, int x, int y)
{
	for(int m=0;m<=CONFIG_NOT_CONNECTED_PIXEL;m++){
		if(y<m)break;
		map[(y-m)*1024+x] = MAP_VAL_DONTDILATE;
	}
}
void drawmap_begin(design* ds, position* pos, u32* map)
{
	drawcolor((u8*)map, 0);
	drawmap_chip(ds, pos, (u32*)map);

	//several pixels above the point dont allow dilate
	int ix,iy;
	int ox,oy;
	int cnt_po = ds->_pinout.size();
	for(int j=0;j<pos->pinviewwire.size()-1;j++){
		int pinid = j;
		if(pinid < cnt_po){		//in pinout
			ox = pos->_out[pinid].x;
			oy = pos->_out[pinid].y;
			drawmap_dontdilate(map, ox, oy);
		}

		for(int k=0;k<pos->pinviewwire[j].size();k++){
			int chipid = pos->pinviewwire[j][k].chipid;
			int footid = pos->pinviewwire[j][k].footid;
			ix = pos->_chip[chipid].x + pos->_chipfoot[chipid][footid].x;
			iy = pos->_chip[chipid].y + pos->_chipfoot[chipid][footid].y;
			drawmap_dontdilate(map, ix, iy);
		}
	}
}
void draw_map2pix(u32* pix, u32* map, std::vector<u32> colortable)
{
	for(int y=0;y<1024;y++){
	for(int x=0;x<1024;x++){
		int tmp = y*1024+x;
		u32 val = map[tmp];
		if( (val>=MAP_WIRE_FIRST) && (val<=MAP_WIRE_FINAL) ){
			int wireid = val - MAP_WIRE_FIRST;
			pix[tmp] = colortable[wireid];
		}
	}
	}
}




int wireid2radius(int wireid)
{
	return CONFIG_DILATE_WIRE_GAP;
}
int chipid2radius(int chipid, std::vector<int>& perchipinfo)
{
	//2, 2+1+2, 2+(1+2)*2
	int remain = perchipinfo[chipid];
	//printf("chipid=%d,remain=%d\n", chipid, remain);
	return CONFIG_DILATE_CHIP_GAP + (CONFIG_DILATE_CHIP_GAP+1)*remain;
}
void drawwire_dilate_onepixel(u32* map, int cnt, int ix, int iy)
{
	int sx = (ix < cnt) ? 0 : (ix - cnt);
	int sy = (iy < cnt) ? 0 : (iy - cnt);
	int ex = (ix+cnt>1023) ? 1023 : (ix+cnt);
	int ey = (iy+cnt>1024) ? 1023 : (iy+cnt);
	for(int y=sy;y<=ey;y++){
	for(int x=sx;x<=ex;x++){
		if(0 == map[y*1024+x]){
			map[y*1024+x] = MAP_VAL_DILATE;
		}
	}
	}
}
void drawwire_dilate(u32* map, std::vector<int>& perchipinfo){
	//clear previous dilate
	for(int y=0;y<1024;y++){
		for(int x=0;x<1024;x++){
			//printf("drawwire_dilate1: %d,%d:%x\n", x,y,map[y*1024+x]);
			if(map[y*1024+x] == MAP_VAL_DILATE)map[y*1024+x] = MAP_VAL_EMPTY;
		}
	}

	for(int y=0;y<1024;y++){
	for(int x=0;x<1024;x++){
		u32 val = map[y*1024+x];
		//printf("drawwire_dilate2: %d,%d:%x\n", x,y,val);
		if( (val >= MAP_CHIP_FIRST) && (val <= MAP_CHIP_FINAL) ){
			int chipid = val - MAP_CHIP_FIRST;
			int radius = chipid2radius(chipid, perchipinfo);
			//printf("drawwire_dilate3: %d,%d:%x,%x,%x\n", x,y,val,chipid,radius);
			drawwire_dilate_onepixel(map, radius, x, y);
		}//if
		else if( (val >= MAP_WIRE_FIRST) && (val <= MAP_WIRE_FINAL) ){
			int wireid = val - MAP_WIRE_FIRST;
			int radius = wireid2radius(wireid);
			drawwire_dilate_onepixel(map, radius, x, y);
		}//if
	}//dx
	}//dy
}




u32 astar_parent_encode(u16 x, u16 y)
{
	return 0x80008000 | (y<<16) | x;
}
int astar_parent_decode(u32* history, u16 x, u16 y, u16* parentx, u16* parenty)
{
	u32 val = history[y*1024+x];
	if((val&0x80008000) != 0x80008000)return 0;

	*parentx = val & 0x7fff;
	*parenty =(val>>16) & 0x7fff;
	return 1;
}
void astar_traceback(u32* map, u32* history, u16 x, u16 y, u32 pinid)
{
	int ret;
	u16 parentx,parenty;
	for(;;){
		//printf("%d,%d\n", x, y);
		map[(y*1024)+x] = MAP_WIRE_FIRST + pinid;

		ret = astar_parent_decode(history, x, y, &parentx, &parenty);
		if(0 == ret)break;
		if((x==parentx)&&(y==parenty))break;

		x = parentx;
		y = parenty;
	}
}




struct astar_data{
	u16 x;
	u16 y;
	//
	u16 parentx;
	u16 parenty;
	//
	int cost_to_src;
	int cost_to_dst;
	//
	int besttarget;
};
struct astar_target{
	u16 x;
	u16 y;
	int belongto;
};
int astar_check(
	std::vector<astar_data>& openlist,
	std::vector<astar_target>& tarlist,
	std::vector<int>& perchip,
	u32* map,
	u32* history,
	u16 x, u16 y, u16 parentx, u16 parenty,
	int cost, u32 pinid)
{
	if(x < 0)return 0;
	if(x > 1023)return 0;
	if(y < 0)return 0;
	if(y > 1023)return 0;
	if(history[y*1024+x])return 0;

	//cond1: mark this pixel visited
	history[y*1024+x] = astar_parent_encode(parentx, parenty);
	openlist.push_back( {x, y, parentx, parenty, cost, 0} );

	//cond2: global set map
	for(auto it=tarlist.begin();it!=tarlist.end();){
		//debug
		if(abs(x-it->x) + abs(y-it->y) < 5)printf("%d,%d ? %d,%d\n", x, y, it->x, it->y);

		//
		if((x==it->x)&&(y==it->y)){
			printf("found it: %d,%d, %d,%d\n", x, y, parentx, parenty);
			int chipid = it->belongto;
			if(perchip[chipid]>0)perchip[chipid] -= 1;
			printf("found it.traceback\n");
			astar_traceback(map, history, x, y, pinid);
			printf("found it.erase\n");
			it = tarlist.erase(it);
			printf("found it.end\n");
		}
		else it++;
	}
	return 1;
}




int drawwire_astar_one(
	u32* pix, u32* map, u32* history,
	std::vector<int>& perchip, std::vector<astar_target>& tpos,
	int pinid, u32 color)
{
	for(int y=0;y<1024;y++){
	for(int x=0;x<1024;x++){
		u32* p = &map[y*1024+x];
		u32* q = &history[y*1024+x];
		if( (0==*p) | (MAP_VAL_DONTDILATE==*p) ){*q = 0;}
		else *q = 0xffffffff;		//fake it in history, means cannot access this pixel
	}
	}

	std::vector<astar_data> openlist;
	std::vector<astar_target> tarlist;
	for(int k=0;k<tpos.size();k++){
		printf("foot%d : %d,%d\n", k, tpos[k].x, tpos[k].y);

		u16 x = tpos[k].x;
		u16 y = tpos[k].y;
		int chipid = tpos[k].belongto;

		if(0==k){
			printf("openlist += %d,%d\n", x, y);
			history[y*1024+x] = astar_parent_encode(x, y);
			openlist.push_back( {x, y, x, y, 0, 0} );
		}
		else{
			printf("tarlist += %d,%d\n", x, y);
			tarlist.push_back( {x, y, chipid} );
		}
	}


	for(int j=0;j<500000;j++){
		if(openlist.size() == 0)break;
		if(tarlist.size() == 0)break;

		int minidx = 0;
		int minval = 99999999;
		for(int p=0;p<openlist.size();p++){
			openlist[p].cost_to_dst = abs(openlist[p].x - tarlist[0].x) + abs(openlist[p].y - tarlist[0].y);
			openlist[p].besttarget = 0;
			for(int q=1;q<tarlist.size();q++){
				int cost = abs(openlist[p].x - tarlist[q].x) + abs(openlist[p].y - tarlist[q].y);
				if(openlist[p].cost_to_dst > cost){
					//printf("update cost %d\n", cost);
					openlist[p].cost_to_dst = cost;
					openlist[p].besttarget = q;
				}
			}
			//printf("round%d openlist%d: x=%d,y=%d,cost=%d+%d,besttarget=%d\n", j, p, openlist[p].x, openlist[p].y, openlist[p].cost_to_src, openlist[p].cost_to_dst, openlist[p].besttarget);
			int sum = openlist[p].cost_to_src + openlist[p].cost_to_dst;
			if(minval > sum){
				minidx = p;
				minval = sum;
			}
		}

		int mx = openlist[minidx].x;
		int my = openlist[minidx].y;
		int cost = openlist[minidx].cost_to_src;
		//printf("round%d: openlist.size=%zu,tarlist.size=%zu,minidx=%d,minval=%d,x=%d,y=%d\n", j, openlist.size(), tarlist.size(), minidx, minval, mx, my);

		//closelist.push_back(openlist[minidx]);
		openlist.erase(openlist.begin()+minidx);

		int size_old = openlist.size();
		astar_check(
			openlist, tarlist, perchip,
			map, history,
			mx  , my-1, mx, my,
			cost+1, pinid);
		astar_check(
			openlist, tarlist, perchip,
			map, history,
			mx  , my+1, mx, my,
			cost+1, pinid);
		astar_check(
			openlist, tarlist, perchip,
			map, history,
			mx+1, my  , mx, my,
			cost+1, pinid);
		astar_check(
			openlist, tarlist, perchip,
			map, history,
			mx-1, my  , mx, my,
			cost+1, pinid);
		int size_new = openlist.size();
		/*
		if(size_old == size_new){
			printf("round%d: size=%d\n", j, size_old);
		}*/
	}

	//failed foots: reach out 8 pixel
	for(int j=0;j<tarlist.size();j++){
		for(int y=0;y<8;y++){
			if(tarlist[j].y < y)break;
			pix[(tarlist[j].y - y) * 1024 + tarlist[j].x] = color;
		}
	}
#if CONFIG_DEBUG_PPM_ASTAR==1
	char name[128];
	snprintf(name, 128, "debug%x_color%06x_total%zu_remain%zu_map.ppm",
		pinid, color, tpos.size(), tarlist.size());
	writeppm((u8*)map, 1024*4, 1024, 1024, name);
	snprintf(name, 128, "debug%x_color%06x_total%zu_remain%zu_history.ppm",
		pinid, color, tpos.size(), tarlist.size());
	writeppm((u8*)history, 1024*4, 1024, 1024, name);
#endif
	return tarlist.size();
}
void drawwire_astar(design* ds, position* pos, u32* pix){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	std::vector<u32> colortable = buildcolortable(cnt_po+cnt_pi, 1);

	u32* map = (u32*)malloc(1024*1024*4);
	drawmap_begin(ds, pos, map);

	u32* parent = (u32*)malloc(1024*1024*4);
	for(int y=0;y<1024;y++)for(int x=0;x<1024;x++)parent[y*1024+x] = 0;

	std::vector<int> perchipinfo;
	for(int j=0;j<pos->_chip.size();j++){
		perchipinfo.push_back(pos->_chipfoot[j].size());
	}

	int ix,iy;
	int ox,oy;
	std::vector<std::vector<astar_target>> perpinpos;
	for(int j=0;j<pos->pinviewwire.size()-1;j++){
		std::vector<astar_target> one;
		if(j < cnt_po){		//in pinout
			ox = pos->_out[j].x;
			oy = pos->_out[j].y;
			one.push_back({(u16)ox, (u16)oy, -1});
		}
		else if(j < cnt_po+cnt_pi){		//in pinin
		}
		else break;

		for(int k=0;k<pos->pinviewwire[j].size();k++){
			//int pinid = pos->pinviewwire[j][k].pinid;
			int chipid = pos->pinviewwire[j][k].chipid;
			int footid = pos->pinviewwire[j][k].footid;
			ix = pos->_chip[chipid].x + pos->_chipfoot[chipid][footid].x;
			iy = pos->_chip[chipid].y + pos->_chipfoot[chipid][footid].y;
			one.push_back({(u16)ix, (u16)iy, chipid});
		}
		perpinpos.push_back(one);
	}

	for(int j=0;j<perpinpos.size();j++){
		printf("pin=%d,size=%zu{\n", j, perpinpos.size());
		drawwire_dilate((u32*)map, perchipinfo);
		int ret = drawwire_astar_one(
			(u32*)pix, (u32*)map, (u32*)parent,
			perchipinfo, perpinpos[j],
			j, colortable[j]);
		printf("}pin=%d,ret=%d\n", j, ret);
		printf("\n\n");

		//
		//if(j>=1)break;
	}

	//convert map to pixdraw_map2pix
	draw_map2pix(pix, map, colortable);

	free(map);
}
void drawwire_astar_lesspinfirst(design* ds, position* pos, u32* pix){
	printf("build colortable\n");
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	std::vector<u32> colortable = buildcolortable(cnt_po+cnt_pi, 1);

	printf("build map\n");
	u32* map = (u32*)malloc(1024*1024*4);
	drawmap_begin(ds, pos, map);

	printf("build history\n");
	u32* history = (u32*)malloc(1024*1024*4);
	for(int y=0;y<1024;y++)for(int x=0;x<1024;x++)history[y*1024+x] = 0;

	printf("perchip\n");
	std::vector<int> perchipinfo;
	for(int j=0;j<pos->_chip.size();j++){
		perchipinfo.push_back(pos->_chipfoot[j].size());
	}

	printf("perpin\n");
	int ix,iy;
	int ox,oy;
	std::vector<std::vector<astar_target>> perpinpos;
	for(int j=0;j<pos->pinviewwire.size()-1;j++){
		std::vector<astar_target> one;
		if(j < cnt_po){		//in pinout
			ox = pos->_out[j].x;
			oy = pos->_out[j].y;
			one.push_back({(u16)ox, (u16)oy, -1});
		}
		else if(j < cnt_po+cnt_pi){		//in pinin
		}
		else break;

		for(int k=0;k<pos->pinviewwire[j].size();k++){
			//int pinid = pos->pinviewwire[j][k].pinid;
			int chipid = pos->pinviewwire[j][k].chipid;
			int footid = pos->pinviewwire[j][k].footid;
			ix = pos->_chip[chipid].x + pos->_chipfoot[chipid][footid].x;
			iy = pos->_chip[chipid].y + pos->_chipfoot[chipid][footid].y;
			one.push_back({(u16)ix, (u16)iy, chipid});
		}

		perpinpos.push_back(one);
	}

	printf("sort\n");
	std::sort(perpinpos.begin(), perpinpos.end(), [](const std::vector<astar_target>& a, const std::vector<astar_target>& b) {
		return a.size() < b.size();
	});

	printf("go\n");
	for(int j=0;j<perpinpos.size();j++){
		printf("pin=%d,size=%zu{\n", j, perpinpos[j].size());
		drawwire_dilate((u32*)map, perchipinfo);
		int ret = drawwire_astar_one(
			(u32*)pix, (u32*)map, (u32*)history,
			perchipinfo, perpinpos[j],
			j, colortable[j]);
		printf("}pin=%d,ret=%d\n", j, ret);
		printf("\n\n");
	}

	printf("map2pix\n");
	draw_map2pix(pix, map, colortable);

	printf("free\n");
	free(history);
	free(map);
}




void draw_onlychip(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawchip(ds, pos, pix);

	writeppm(pix, 1024*4, 1024, 1024, "onlychip.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw_onlyfoot(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawfoot(ds, pos, pix);

	writeppm(pix, 1024*4, 1024, 1024, "onlyfoot.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw_onlypinout(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawpinout(ds, pos, pix);

	writeppm(pix, 1024*4, 1024, 1024, "onlypinout.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw_chipview(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawchip(ds, pos, pix);
	drawfoot(ds, pos, pix);
	drawpinout(ds, pos, pix);

	drawwire_chipview(ds, pos, pix);

	writeppm(pix, 1024*4, 1024, 1024, "wire_chipview.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw_pinview(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawchip(ds, pos, pix);
	drawfoot(ds, pos, pix);
	drawpinout(ds, pos, pix);

	drawwire_pinview(ds, pos, pix);

	writeppm(pix, 1024*4, 1024, 1024, "wire_pinview.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw_astar(design* ds, position* pos, u8* pix){
	printf("%s{\n",__FUNCTION__);
	drawcolor(pix, 0);

	drawchip(ds, pos, pix);
	//drawfoot(ds, pos, pix);
	//drawpinout(ds, pos, pix);

	//drawwire_astar(ds, pos, (u32*)pix);
	drawwire_astar_lesspinfirst(ds, pos, (u32*)pix);

	writeppm(pix, 1024*4, 1024, 1024, "wire_astar.ppm");
	printf("}%s\n",__FUNCTION__);
}
void draw(design* ds, position* pos, u8* pix){
/*
	draw_onlychip(ds, pos, pix);
	draw_onlyfoot(ds, pos, pix);
	draw_onlypinout(ds, pos, pix);
*/
	draw_chipview(ds, pos, pix);
	draw_pinview(ds, pos, pix);
	draw_astar(ds, pos, pix);
}




u8 buf[1024*1024*4];
int main(int argc, char** argv)
{
	if(argc<=1){
		printf("./a.out xxxx.chip\n");
		return 0;
	}

	session* sess = new session();
	int j;
	for(j=1;j<argc;j++){
		FILE* fp = fopen(argv[j], "rb");
		int sz = fread(buf, 1, 0x100000, fp);

		filecontext* ctx = new filecontext((u8*)argv[j]);
		parse(ctx, buf, sz);
		//printfilectx(ctx);
		sess->file.push_back(ctx);

		fclose(fp);
	}
	printsession(sess);


	printf("name to expand:");
	std::string input;
	std::cin >> input;
	design* it = expand(sess, input);
	if(0 == it)return 0;

	position pos = position();
	layout(it, &pos);
	draw(it, &pos, buf);

	delete it;
	return 0;
}
