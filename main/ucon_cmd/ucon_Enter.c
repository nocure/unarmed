#include <stdlib.h>
#include <string.h>
#include <platform/yl_Sys.h>


void uconEnter(int argc, char** argv) {
DWORD addr;
DWORD value;
char width =4;
int  s;
char *buf;

	getopt(0,0,0); // reset getopt state
	while( (s=getopt(argc,argv,"hbwd")) != EOF ) {
		switch(s) {
		case 'b':
			width=1;
			break;

		case 'w':
			width=2;
			break;

		case 'd':
			width=4;
			break;

		case 'h':
			printf(
				"Usage: %s [switches] <memory address>\n"
				"Switches:\n"
				"    -b      byte format\n"
				"    -w      word format\n"
				"    -d      dword format <--\n"
				,argv[0]);
			return;
		}
	}

	if ((argc-optind)>0) {
		if (yl_strtoaddr(argv[optind],(DWORD*)&addr)) return;
		addr&=~3;
		optind++;
	}
	else {
		printf("no memory address\n");
		return;
	}

	while(1) {
	    printf("%08x: ", addr);

		uconLineReset();
		while(!uconInputCheck());

		buf=uconGetCmdBuf();
    	if(buf[0]==0) break;

		if (yl_strtoul(buf,&value)) continue;
		if ( ((width==1)&&(value>0xff)) || ((width==2)&&(value>0xffff)) ) {
			printf("do not match type \n");
			continue;
		}
        if(width==1){
            *((BYTE*)addr)=(BYTE)value;
            addr++;
	    }
	    if(width==2){
    	    *((WORD*)addr)=(WORD)value;
	        addr=addr+2 ;
    	}
        if(width==4){
        	*((DWORD*)addr)=(DWORD)value;
	        addr+=4;
        }
    }
    return;
}








