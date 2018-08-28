#include <stdlib.h>
#include <platform/tq_bsp.h>
#include <platform/yl_Sys.h>

static DWORD g_lastaddr=0x30000000;
static int g_dispfmt =0;


static void uconDumpAscii(BYTE* addr,int len) {
char c;

	while (len--) {
		c=*addr++;
		if ((c<=0x1f)||(c>=0x7f)) c='.';
		putch(c);
	}
}


void uconMemDump(int argc, char** argv) {
int s,j, k;
DWORD addr,len;

	getopt(0,0,0); // reset getopt state
	while( (s=getopt(argc,argv,"hbwdi")) != EOF ) {
		switch(s) {
		case 'b':
			g_dispfmt=0;
			break;

		case 'w':
			g_dispfmt=1;
			break;

		case 'd':
			g_dispfmt=2;
			break;

		case 'i':
			g_dispfmt=3;
			break;

		case 'h':
			printf(
				"Usage: %s [switches] [memory address] [display length]\n"
				"Switches:\n"
				"    -b      byte format\n"
				"    -w      word format\n"
				"    -d      dword format\n"
				"    -i      bit format\n"
				,argv[0]);
			return;
		}
	}

	if ((argc-optind)>0) {
		if (yl_strtoaddr(argv[optind],&addr)) return;
		addr&=~3;
		optind++;
	}
	else addr=g_lastaddr;

	if ((argc-optind)>0) {
		if (yl_strtoul(argv[optind],&len)) return;
	}
	else len=0;
	optind++;

	if (g_dispfmt!=3) {
		if (!len) len=0x80;
		else {
			if (len&0xf) len= ((len>>4)+1)<<4;
		}
	}
	else {
		if (!len) len=0x20;
		else {
			if (len&0x3) len= ((len>>2)+1)<<2;
		}
	}

	if (g_dispfmt==0) {				// byte display
		BYTE* p=(BYTE*)addr;
		for (;len;len--) {
			if ((len&0xf)==0) printf("%08x: ", p);
			printf("%02x ", *p++);
			if ((len&0xf)==1) {
				uconDumpAscii((BYTE*)addr,16);
				printf("\n");
				addr+=16;
			}
		}
		addr=(DWORD)p;
	}
	else  if (g_dispfmt==1) {		// word display
		WORD* p=(WORD*)addr;
		len>>=1;		// len / 2
		for (;len;len--) {
			if ((len&0x7)==0) printf("%08x: ", p);
			printf("%04x ", *p++);
			if ((len&0x7)==1) {
				uconDumpAscii((BYTE*)addr,16);
				printf("\n");
				addr+=16;
			}
		}
		addr=(DWORD)p;
	}
	else if (g_dispfmt==2) {		// dword display
		DWORD* p=(DWORD*)addr;
		len>>=2;		// len / 4
		for (;len;len--) {
			if ((len&0x3)==0) printf("%08x: ", p);
			printf("%08x ", *p++);
			if ((len&0x3)==1) {
				uconDumpAscii((BYTE*)addr,16);
				printf("\n");
				addr+=16;
			}
		}
		addr=(DWORD)p;
	}
	else if (g_dispfmt==3) {
		BYTE* p=(BYTE*)addr;
		for (;len;len--) {
			if ((len&0x3)==0) printf("%08x: ", p);
			for (k=*p++,j=0x80;j;j>>=1) {
				if (k&j) putch('1');
				else putch('0');
			}
			printf(" ");
			if ((len&0x3)==1) {
				uconDumpAscii((BYTE*)addr,4);
				printf("\n");
				addr+=4;
			}
		}
		addr=(DWORD)p;
	}

	g_lastaddr = addr;
	printf("Next address is %08x:\n", g_lastaddr);

}


