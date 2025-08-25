#include "util.hpp"
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
	chipname = std::string((char*)buf, len);
}
wiredef::wiredef(std::string s){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_string wiredef %s\n", s.c_str());
#endif
	chipname = s;
}
wiredef::wiredef(wiredef* in){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_copy wiredef %s\n", in->chipname.c_str());
#endif
	chipname = in->chipname;
	for(auto p : in->pinname)pinname.push_back(p);
}
wiredef::~wiredef(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete wiredef %s\n", name.c_str());
#endif
}
void wiredef::parsepin(u8* p, int len)
{
	/*
	p0(vcc, oa, en0)
	*/
	int k;
	int now = 0;
	int round = 0;
	for(k=0;k<len;k++){
		switch(p[k]){
		case ',':
		case ' ':
		case '\t':
		case ')':
			if(1 == round){
				addpin(p+now, k-now);
				round = !round;
			}
			if(')'==p[k])k=len;	//big break
			break;
		default:
			if(0 == round){
				now = k;
				round = !round;
			}
		}
	}
}
void wiredef::addpin(std::string s){
	pinname.push_back(s);
}
void wiredef::addpin(u8* buf, int len){
	int j;
    int leftend = 0;
    int rightbegin = 0;
	for(j=0;j<len;j++){
		if('@' == buf[j]){
			leftend = j;
            rightbegin = j+1;
			break;
		}
	}
    std::string origname((char*)buf, leftend);

    std::string nickname((char*)buf+rightbegin, len-rightbegin);

	pinname.push_back(nickname);
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
	for(auto p : c->_connect)_connect.push_back(new wiredef(p));
}
design::~design(){
#if CONFIG_PRINT_DESTRUCT==1
		printf("delete design %s\n", name.c_str());
#endif
	for(auto p : _pinout)delete(p);
	for(auto p : _pinin)delete(p);
	for(auto p : _chip)delete(p);
	for(auto p : _connect)delete(p);
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
	for(auto it = _connect.begin();it != _connect.end();){
		if('/' == (*it)->chipname.c_str()[0]){
			delete (*it);
			it = _connect.erase(it);
		}
		else{
			it++;
		}
	}
}




posxyz::posxyz(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {
}
posxyz::posxyz(u8* buf, int len){
	char* str = (char*)buf;
	int j = 0;
	char* endptr;

	//remove space
	while( (j<len) && (str[j]==' ' || str[j]=='\t') )j++;
	if(j>=len){
		x = y = z = 0;
		goto byebye;
	}

	//x = std::stof(str+j);
	x = strtof(str+j, &endptr);
	if(endptr == str+j){
		x = y = z = 0;
		goto byebye;
	}

	//find ,
	while( (j<len) && (str[j]!=',') )j++;
	if(j>=len){
		y = z = 0;
		goto byebye;
	}
	j++;

	//remove space
	while( (j<len) && (str[j]==' ' || str[j]=='\t') )j++;
	if(j>=len){
		y = z = 0;
		goto byebye;
	}

	//y
	y = strtof(str+j, &endptr);
	if(endptr == str+j){
		y = z = 0;
		goto byebye;
	}

	//find ,
	while( (j<len) && (str[j]!=',') )j++;
	if(j>=len){
		z = 0;
		goto byebye;
	}
	j++;

	//remove space
	while( (j<len) && (str[j]==' ' || str[j]=='\t') )j++;
	if(j>=len){
		z = 0;
		goto byebye;
	}

	//z
	z = strtof(str+j, &endptr);
	if(endptr == str+j){
		z = 0;
		goto byebye;
	}

byebye:
	printf("%f,%f,%f\n", x, y, z);
}


filecontext::filecontext(u8* buf){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_buf filectx %s\n", buf);
#endif
	filename = std::string((char*)buf);
}
filecontext::~filecontext(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete filectx %s\n", filename.c_str());
#endif
	for(auto p : cx)delete(p);
}



session::session(){
#if CONFIG_PRINT_CONSTRUCT==1
	printf("contruct_noarg session\n");
#endif
}
session::~session(){
#if CONFIG_PRINT_DESTRUCT==1
	printf("delete session\n");
#endif
	for(auto p : file)delete(p);
}




void printdesign(design* c){
	printf("%s{\n", c->name.c_str());

	int cnt = 0;
	printf("pinout(size=%ld){\n", c->_pinout.size());
	for(auto p : c->_pinout){
		printf("	%s\n", p->name.c_str());
		cnt++;
	}
	printf("}pinout(cnt=%d)\n", cnt);

	cnt = 0;
	printf("pinin(size=%ld){\n", c->_pinin.size());
	for(auto p : c->_pinin){
		printf("	%s\n", p->name.c_str());
		cnt++;
	}
	printf("}pinin(cnt=%d)\n", cnt);

	cnt = 0;
	printf("chip(size=%ld){\n", c->_chip.size());
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
	printf("}chip(cnt=%d)\n", cnt);

	cnt = 0;
	printf("logic(size=%ld){\n", c->_connect.size());
	for(auto& p : c->_connect){
		if('/' != p->chipname.c_str()[0]){
			cnt++;
		}
#if CONFIG_PRINT_HIDE_COMMENTED==1
		else{
			continue;
		}
#endif
		printf("	%s(", p->chipname.c_str());
		for(auto& t : p->pinname)printf("%s%c", t.c_str(), (t==p->pinname.back()) ? ')' : ' ');
		printf("\n");
	}
	printf("}logic(cnt=%d)\n", cnt);

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




design* finddesign(session* sess, std::string& name){
    int k,f;
    for(f=0;f<sess->file.size();f++){
        filecontext* ctx = sess->file[f];
        for(k=0;k<ctx->cx.size();k++){
            if(name == ctx->cx[k]->name){
                return ctx->cx[k];
            }
        }
    }
    return 0;
}

wiredef* findwire(design* ds, std::string& name){
    int k;
    for(k=0;k<ds->_connect.size();k++){
        if(name == ds->_connect[k]->chipname){
            return ds->_connect[k];
            break;
        }
    }
    return 0;
}