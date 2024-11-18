#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>
#include<string>
#include<vector>
typedef unsigned char u8;


class pindef{
public:
	pindef(u8* buf, int len){
		name = std::string((char*)buf, len);
	}
	pindef(std::string s){
		name = s;
	}
	pindef(pindef* c){
		name = c->name;
	}
	std::string name;
};

class chipdef{
public:
	chipdef(u8* buf, int len, u8* cb, int cl){
		name = std::string((char*)buf, len);
		cname = std::string((char*)cb, cl);
	}
	chipdef(std::string s0, std::string s1){
		name = s0;
		cname = s1;
	}
	chipdef(chipdef* c){
		name = c->name;
		cname = c->cname;
	}
	std::string name;
	std::string cname;
};

class wiredef{
public:
	wiredef(u8* buf, int len){
		name = std::string((char*)buf, len);
	}
	wiredef(std::string s){
		name = s;
	}
	wiredef(wiredef* c){
		name = c->name;
		for(auto p : c->pinname)pinname.push_back(p);
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
		name = std::string((char*)buf, len);
	}
	design(design* c){
		name = c->name;
		for(auto p : c->_pinout)_pinout.push_back(new pindef(p));
		for(auto p : c->_pinin)_pinin.push_back(new pindef(p));
		for(auto p : c->_chip)_chip.push_back(new chipdef(p));
		for(auto p : c->_logic)_logic.push_back(new wiredef(p));
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
		filename = std::string((char*)buf);
	}
	std::string filename;
	std::vector<design*> cx;
};

class session{
public:
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
					for(k=0;k<firstmaohao-firststr;k++){
					switch(p[k]){
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

	printf("pinout:%ld\n", c->_pinout.size());
	for(auto p : c->_pinout){
		printf("	%s\n", p->name.c_str());
	}

	printf("pinin:%ld\n", c->_pinin.size());
	for(auto p : c->_pinin){
		printf("	%s\n", p->name.c_str());
	}

	printf("chip:%ld\n", c->_chip.size());
	for(auto p : c->_chip){
		printf("	%s : %s\n", p->name.c_str(), p->cname.c_str());
	}

	printf("logic:%ld\n", c->_logic.size());
	for(auto p : c->_logic){
		printf("	%s(", p->name.c_str());
		for(auto t : p->pinname)printf("%s%c", t.c_str(), (t==p->pinname.back()) ? ')' : ' ');
		printf("\n");
	}

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

void expand(session* sess, std::string name){
	int j;
	design* found = 0;
	for(auto ctx : sess->file){
		for(j=0;j<ctx->cx.size();j++){
			if(name == ctx->cx[j]->name){
				found = ctx->cx[j];
			}
		}
	}
	if(0 == found)return;
	printf("\n");

	design* theone = new design(found);
	int last = 0;
	while(1){
		expand_onelayer(sess, theone, last);
		last = theone->_chip.size();
		break;
	}
	printf("\n");

	printdesign(theone);
}


u8 buf[0x100000];
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
		printfilectx(ctx);
		sess->file.push_back(ctx);

		fclose(fp);
	}


	printf("design name to expand:");
	std::string input;
	std::cin >> input;
	expand(sess, input);
}
