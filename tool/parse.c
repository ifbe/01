#include<stdio.h>
#include<stdlib.h>
typedef unsigned char u8;


void parse(u8* buf, int len){
int linehead = 0;

int j;
for(j=0;j<len;j++){
	switch(buf[j]){
	case '\n':
		linehead = j+1;
		break;
	case ':':
	case '{':
		printf("%.*s\n", j-linehead, buf+linehead);
		break;
	case '}':
		break;
	}
}//for
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
	parse(buf, sz);
	fclose(fp);
}
