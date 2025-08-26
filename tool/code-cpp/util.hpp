#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
//
#include<iostream>
#include<vector>
#include<algorithm>
#include<map>
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned int u64;


struct posxyz{
	float x,y,z;
	posxyz(u8* str, int len);
	posxyz(float ix, float iy, float iz);
	//posxyz(posxyz& in);
};


class pindef{
public:
	pindef(u8* buf, int len);
	pindef(std::string s);
	pindef(pindef* c);
	~pindef();
public:
	std::string name;
};

class chipdef{
public:
	chipdef(u8* buf, int len, u8* cb, int cl);
	chipdef(std::string s0, std::string s1);
	chipdef(chipdef* c);
	~chipdef();
public:
	std::string name;
	std::string cname;
};

class wiredef{
public:
	struct _pinpair{
		std::string nickname;
		std::string origname;
	};
	wiredef(u8* buf, int len);
	wiredef(std::string s);
	wiredef(wiredef* c);
	~wiredef();
	void addpin(std::string& s);
	void addpin(_pinpair& s);
	void addpin(u8* buf, int len);
	void parsepin(u8* p, int len);
public:
	std::string chipname;
	std::vector<_pinpair> pinpair;
};

class design{
public:
	design(u8* buf, int len);
	design(design* c);
	~design();
	void shrinkunused();
public:
	//design.name
	std::string name;
	//design.pin
	std::vector<pindef*> _pinout;
	std::vector<pindef*> _pinin;
	std::vector<pindef*> _pinglobal;		//todo
	//design.chip
	std::vector<chipdef*> _chip;
	//wire space
	std::vector<wiredef*> _connect;
	//pcb space
	std::map<std::string, posxyz> _layout; 
};




class filecontext{
public:
	filecontext(u8* buf);
	~filecontext();
public:
	std::string filename;
	std::vector<design*> cx;
};

class session{
public:
	session();
	~session();
public:
	std::vector<filecontext*> file;
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
	std::vector<posxyz> _pin_global;
	//
	std::vector<posxyz> _chip;
	std::vector<std::vector<posxyz>> _chipfoot;
	//
	std::vector<std::vector<chipfootpin>> pinviewwire;		//[pin][conn][cid,fid,pid]
	//
	std::vector<std::vector<chipfootpin>> chipviewwire;		//[chip][foot][cid,fid,pid]
};




void printdesign(design* c);
void printfilectx(filecontext* ctx);
void printsession(session* sess);




design* finddesign(session* sess, std::string& name);
wiredef* findwire(design* ds, std::string& name);