#include<stdio.h>
#include<stdlib.h>
typedef unsigned char u8;


void parse(u8* buf, int len){
	int linecount = 0;
	int bracketcount = 0;
	int thislinefirstbyte = 0;
	int thislinefirstname = -1;

	int j;
	for(j=0;j<len;j++){
		if(0){
		}
		else if('\n' == buf[j]){
			linecount++;
			thislinefirstbyte = j+1;
			thislinefirstname = -1;
		}
		else if(':' == buf[j]){
			if(thislinefirstname>=0)printf("a%d_b%d_%.*s\n", linecount+1, bracketcount, j-thislinefirstname, buf+thislinefirstname);
		}
		else if('{' == buf[j]){
			if(thislinefirstname>=0)printf("a%d_b%d_%.*s\n", linecount+1, bracketcount, j-thislinefirstname, buf+thislinefirstname);
			bracketcount++;
		}
		else if('}' == buf[j]){
			bracketcount--;
		}
		else if( ('a' <= buf[j]) && ('z' >= buf[j]) ){
			if(thislinefirstname<0)thislinefirstname = j;
		}
		else{
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
