#include "main.hpp"
#include <cstdlib>
#define CONFIG_PRINT_CONSTRUCT 1
#define CONFIG_PRINT_DESTRUCT 0


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




void printdesign(design* c);
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




//step0: file to mem
void parse(filecontext* ctx, u8* buf, int len);
//step1: expand until cannot do further
design* expand(session* sess, std::string name);
//step2: decide chip and pin position
void layout(session* sess, design* ds, position* pos);
//step3: auto wiring, no intersect
void draw(session* sess, design* ds, position* pos, u8* pix);




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

	printf("name to layout:");
	std::cin >> input;

	position pos = position();
	layout(sess, it, &pos);

	printf("name to draw:");
	std::cin >> input;

	draw(sess, it, &pos, buf);

	delete it;
	return 0;
}
