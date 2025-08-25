#include "util.hpp"
#include <cstdlib>




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
