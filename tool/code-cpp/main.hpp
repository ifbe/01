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
	wiredef(u8* buf, int len);
	wiredef(std::string s);
	wiredef(wiredef* c);
	~wiredef();
	void addpin(std::string s);
	void addpin(u8* buf, int len);
public:
	std::string name;
	std::vector<std::string> pinname;
};

class design{
public:
	design(u8* buf, int len);
	design(design* c);
	~design();
	void shrinkunused();
public:
	std::string name;
	std::vector<pindef*> _pinout;
	std::vector<pindef*> _pinin;
	//std::vector<pindef*> _pinglobal;		//todo
	std::vector<chipdef*> _chip;
	std::vector<wiredef*> _logic;
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
	std::vector<posxyz> _pin_global;
	//
	std::vector<posxyz> _chip;
	std::vector<std::vector<posxyz>> _chipfoot;
	//
	std::vector<std::vector<chipfootpin>> pinviewwire;		//[pin][conn][cid,fid,pid]
	//
	std::vector<std::vector<chipfootpin>> chipviewwire;		//[chip][foot][cid,fid,pid]
};