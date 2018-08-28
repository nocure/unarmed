#include <stdlib.h>
#include <platform/yl_sys.h>

void uconFillMem(int argc, char** argv) {
DWORD addr;
DWORD len;
DWORD value = 0x00;
char width =4;
int i,s,randval;

	value = 0; // default value to fill
	randval =0;

	getopt(0,0,0); // reset getopt state
	while( (s=getopt(argc,argv,"hbwdp:r")) != EOF ) {
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

		case 'p':
	    	if (yl_strtoul(optarg,&value)) return;
	    	break;

		case 'r':
			randval=1;
			break;

		case 'h':
			printf(
				"Usage: %s [switches] <memory address> <size in byte>\n"
				"Switches:\n"
				"    -b      byte format\n"
				"    -w      word format\n"
				"    -d      dword format <--\n"
				"    -p xxx  value to fill (0x00)\n"
				"    -r      randon value to fill\n"
				,argv[0]);
			return;
		}
	}

	if ((argc-optind)>0) {
		if (yl_strtoaddr(argv[optind],(DWORD*)&addr)) return;
		optind++;
	}
	else {
		printf("no memory address\n");
		return;
	}

	if ((argc-optind)>0) {
    	if (yl_strtoul(argv[optind],&len)) return;
		optind++;
	}
	else {
		printf("no size\n");
		return;
	}

	if ( ((width==1)&&(value>0xff)) || ((width==2)&&(value>0xffff)) ) {
		printf("format does not match.\n");
		return;
	}


	if(width==1){
	    for(i=0;i<len;i++){
			if (randval) value=rand()%0x100;
		    *((BYTE*)addr)=(BYTE)value;
		    addr++;
		}
	}
	else if(width==2){
		addr&=~1;
		len>>=1; // /2
	    for(i=0;i<len;i++){
			if (randval) value=rand()%0x10000;
		    *((WORD*)addr)=(WORD)value;
		    addr+=2 ;
	    }
	}
	else if(width==4){
		addr&=~3;
		len>>=2; // /4
	    for(i=0;i<len;i++){
			if (randval) value=rand();
	    	*((DWORD*)addr)=(DWORD)value;
		    addr+=4;
	    }
	}
	return;
}
