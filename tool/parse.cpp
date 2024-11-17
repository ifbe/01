#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<vector>
typedef unsigned char u8;


class pindef{
public:
	pindef(u8* buf, int len){
		name = std::string((char*)buf, len);
	}
	std::string name;
};

class chip{
public:
	chip(u8* buf, int len, u8* cb, int cl){
		name = std::string((char*)buf, len);
		cname = std::string((char*)cb, cl);
	}
	std::string name;
	std::string cname;
};

class wire{
public:
	wire(u8* buf, int len){
		name = std::string((char*)buf, len);
	}
	void addpin(u8* buf, int len){
		pin.push_back(std::string((char*)buf, len));
	}
	std::string name;
	std::vector<std::string> pin;
};

class composite{
public:
	composite(u8* buf, int len){
		name = std::string((char*)buf, len);
	}
	std::string name;
	std::vector<pindef*> _pinout;
	std::vector<pindef*> _pinin;
	std::vector<chip*> _chip;
	std::vector<wire*> _logic;
};

class filecontext{
public:
	std::vector<composite*> cx;
};

void parse(filecontext* ctx, u8* buf, int len){
	composite* comp = 0;
	pindef* pin = 0;
	chip* ch = 0;
	wire* wi = 0;

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
					if(firstmaohao >= 0){
						pin = new pindef(buf+firststr, firstmaohao-firststr);
					}
					else{
						pin = new pindef(buf+firststr, j-firststr);
					}
					comp->_pinout.push_back(pin);
				}
				else if(PININ == currsection){
					if(firstmaohao >= 0){
						pin = new pindef(buf+firststr, firstmaohao-firststr);
					}
					else{
						pin = new pindef(buf+firststr, j-firststr);
					}
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
							ch = new chip(p+now, k-now, qb, ql);
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
					wi = new wire(buf+firststr, firstkuohao-firststr);
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
					comp = new composite(buf+firststr, j-firststr);
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


void print(filecontext* ctx){
	for(auto c : ctx->cx){
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
			for(auto t : p->pin)printf("%s%s", t.c_str(), (t==p->pin.back()) ? ")\n" : " ");
		}

		printf("}\n");
	}
}


u8 buf[0x100000];
int main(int argc, char** argv)
{
	if(argc<=1){
		printf("./a.out xxxx.chip\n");
		return 0;
	}

	FILE* fp = fopen(argv[1], "rb");
	int sz = fread(buf, 1, 0x100000, fp);

	filecontext* ctx = new filecontext;
	parse(ctx, buf, sz);
	print(ctx);

	fclose(fp);
}
