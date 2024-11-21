#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
//
#include<iostream>
#include<vector>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;


#define CONFIG_PRINT_CONSTRUCT 1
#define CONFIG_PRINT_DESTRUCT 0
#define CONFIG_PRINT_HIDE_COMMENTED 0


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




struct pos{
	float x,y,z;
};
struct line{
	float sx,sy;
	int chipid;
	int pinid;
};
class position{
public:
	std::vector<pos> _out;
	std::vector<pos> _in;
	std::vector<pos> _chip;
	std::vector<std::vector<line>> wire;
};
void layout(design* ds, position* pos){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	int cnt_chip = ds->_chip.size();
	int cnt_logic = ds->_logic.size();
	printf("layout: %d,%d,%d,%d\n", cnt_po, cnt_pi, cnt_chip, cnt_logic);

	float fx,fy;
	int ix,iy,sz;

	//pinout at top
	for(int j=0;j<cnt_po;j++){
		fx = (float)(j+1)/(cnt_po+1);
		fy = 0.0;
		pos->_out.push_back({fx, 0, 0});
		printf("out %d: %f,%f\n", j, fx, fy);
	}

	//pinin at bot
	for(int j=0;j<cnt_pi;j++){
		fx = (float)(j+1)/(cnt_pi+1);
		fy = 0.999;
		pos->_in.push_back({fx, fy, 0});
		printf("out %d: %f,%f\n", j, fx, fy);
	}

	//chip at middle
	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);
	printf("sqrt=%f,ceil=%d\n", sq, ce);

	for(int j=0;j<cnt_chip;j++){
		fx = (float)((j%ce)+1)/(ce+1);
		fy = (float)((j/ce)+1)/(ce+1);
		printf("chip %d: %f,%f\n", j, fx, fy);
		pos->_chip.push_back({fx, fy, 0});
	}

	//each chip has many outpin, each outpin have and src and dst
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
		ix = 1024*pos->_chip[findchip].x;
		iy = 1024*pos->_chip[findchip].y;
		sz = 1024/ce/4;

		//each pin
		std::vector<std::string> pinname = ds->_logic[j]->pinname;
		std::vector<pindef*> po = ds->_pinout;
		std::vector<pindef*> pi = ds->_pinin;
		std::vector<line> l;
		for(int k=0;k<pinname.size();k++){
			delta = (k+1) * (sz*2) / (pinname.size()+1);
			fx = -sz+delta;
			fy = -sz;
			//pinout
			for(int m=0;m<po.size();m++){
				if(pinname[k] == po[m]->name){
					printf("c %d - po %d : %f,%f\n", j, m, fx, fy);
					l.push_back({fx, fy, findchip, m});
					goto ok;
				}
			}
			//pinin
			for(int m=0;m<pi.size();m++){
				if(pinname[k] == pi[m]->name){
					printf("c %d - pi %d : %f,%f\n", j, m, fx, fy);
					l.push_back({fx, fy, findchip, cnt_po+m});
					goto ok;
				}
			}
			//power
			printf("c %d - pi : %f,%f\n", j, fx, fy);
			l.push_back({fx, fy, findchip, cnt_po+cnt_pi});
ok:
			continue;
		}
		pos->wire.push_back(l);
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

	rgb |= 0xff000000;
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
void writeppm(unsigned char* buf, int pitch, int w, int h)
{
	printf("buf=%p,pitch=0x%x,w=%d,h=%d\n", buf, pitch, w, h);
	FILE* fp = fopen("out.ppm", "wb");

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
void draw(design* ds, position* pos, u8* pix){
	int cnt_po = ds->_pinout.size();
	int cnt_pi = ds->_pinin.size();
	int cnt_chip = ds->_chip.size();
	int cnt_logic = ds->_logic.size();
	printf("draw: %d,%d,%d,%d\n", cnt_po, cnt_pi, cnt_chip, cnt_logic);

	drawcolor(pix, 0);

	float fx,fy;
	int ix,iy,sz;

	float sq = sqrt(cnt_chip);
	int ce = ceil(sq);
	printf("sqrt=%f,ceil=%d\n", sq, ce);

	for(int j=0;j<cnt_chip;j++){
		fx = pos->_chip[j].x;
		fy = pos->_chip[j].y;
		ix = 1024*fx;
		iy = 1024*fy;
		sz = 1024/ce/4;
		drawline_rect(pix, 0x888888, ix-sz, iy-sz, ix+sz, iy+sz);
	}

	std::vector<u32> colortable = buildcolortable(cnt_po+cnt_pi, 1);
	int ox;
	int oy;
	for(int j=0;j<cnt_logic;j++){
		for(int k=0;k<pos->wire[j].size();k++){
			int chipid = pos->wire[j][k].chipid;
			ix = 1024*pos->_chip[chipid].x + pos->wire[j][k].sx;
			iy = 1024*pos->_chip[chipid].y + pos->wire[j][k].sy;
			//
			int pinid = pos->wire[j][k].pinid;
			if(pinid < cnt_po){		//in pinout
				ox = 1024*pos->_out[pinid].x;
				oy = 1024*pos->_out[pinid].y;
			}
			else if(pinid < cnt_po+cnt_pi){		//in pinin
				ox = 1024*pos->_in[pinid-cnt_po].x;
				oy = 1024*pos->_in[pinid-cnt_po].y;
			}
			else{		//maybe power pin
				ox = ix;
				oy = iy-10;
			}
			//
			printf("%d,%d : %d,%d : %d,%d - %d,%d\n", j, k, chipid, pinid, ix, iy, ox, oy);
			drawline(pix, colortable[pos->wire[j][k].pinid], ix,iy, ox,oy);
		}
	}

	writeppm(pix, 1024*4, 1024, 1024);
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
