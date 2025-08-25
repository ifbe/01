#include "util.hpp"





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
#define CONNECT 4
#define LAYOUT 5
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
				else if(CONNECT == currsection){
					wi = new wiredef(buf+firststr, firstkuohao-firststr);

					wi->parsepin(buf+firstkuohao+1, j-firstkuohao);

					comp->_connect.push_back(wi);
				}
				else if(LAYOUT == currsection){
					printf("what=<%.*s> wholeline=%d<%.*s>\n",
						firstkuohao-firststr,buf+firststr,
						j-firststr, j-firststr, buf+firststr);
					std::string str((char*)buf+firststr, firstkuohao-firststr);
					
					comp->_layout.emplace(str, posxyz(buf+firstkuohao+1, j-firstkuohao-1));
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
						currsection = CONNECT;
					}
					else if(strncmp("layout", (char*)buf+firststr, j-firststr) == 0){
						currsection = LAYOUT;
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
