#include "main.hpp"
#define CONFIG_PRINT_CONSTRUCT 1
#define CONFIG_PRINT_DESTRUCT 0




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




//
void parse(filecontext* ctx, u8* buf, int len);
design* expand(session* sess, std::string name);
//
void layout(design* ds, position* pos);
void draw(design* ds, position* pos, u8* pix);




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
