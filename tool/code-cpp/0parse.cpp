#include "main.hpp"
#define CONFIG_PRINT_CONSTRUCT 1
#define CONFIG_PRINT_DESTRUCT 0


pindef::pindef(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_buflen pindef %.*s\n", len,buf);
#endif
	name = std::string((char*)buf, len);
}
pindef::pindef(std::string s){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_string pindef %s\n", s.c_str());
#endif
	name = s;
}
pindef::pindef(pindef* c){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_copy pindef %s\n", c->name.c_str());
#endif
	name = c->name;
}
pindef::~pindef(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete pindef %s\n", name.c_str());
#endif
}




chipdef::chipdef(u8* buf, int len, u8* cb, int cl){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_buflen chipdef %.*s %.*s\n", len,buf, cl,cb);
#endif
	name = std::string((char*)buf, len);
	cname = std::string((char*)cb, cl);
}
chipdef::chipdef(std::string s0, std::string s1){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_string chipdef %s %s\n", s0.c_str(), s1.c_str());
#endif
	name = s0;
	cname = s1;
}
chipdef::chipdef(chipdef* c){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_copy chipdef %s %s\n", c->name.c_str(), c->cname.c_str());
#endif
	name = c->name;
	cname = c->cname;
}
chipdef::~chipdef(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete chipdef %s %s\n", name.c_str(), cname.c_str());
#endif
}




wiredef::wiredef(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_buflen wiredef %.*s\n", len,buf);
#endif
	name = std::string((char*)buf, len);
}
wiredef::wiredef(std::string s){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_string wiredef %s\n", s.c_str());
#endif
	name = s;
}
wiredef::wiredef(wiredef* c){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_copy wiredef %s\n", c->name.c_str());
#endif
	name = c->name;
	for(auto p : c->pinname)pinname.push_back(p);
}
wiredef::~wiredef(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete wiredef %s\n", name.c_str());
#endif
}
void wiredef::addpin(std::string s){
	pinname.push_back(s);
}
void wiredef::addpin(u8* buf, int len){
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




design::design(u8* buf, int len){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_buflen design %.*s\n", len,buf);
#endif
	name = std::string((char*)buf, len);
}
design::design(design* c){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_copy design %s\n", c->name.c_str());
#endif
	name = c->name;
	for(auto p : c->_pinout)_pinout.push_back(new pindef(p));
	for(auto p : c->_pinin)_pinin.push_back(new pindef(p));
	for(auto p : c->_chip)_chip.push_back(new chipdef(p));
	for(auto p : c->_logic)_logic.push_back(new wiredef(p));
}
design::~design(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete design %s\n", name.c_str());
#endif
	for(auto p : _pinout)delete(p);
	for(auto p : _pinin)delete(p);
	for(auto p : _chip)delete(p);
	for(auto p : _logic)delete(p);
}
void design::shrinkunused(){
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