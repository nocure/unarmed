#include <platform/yl_sys.h>

void uconCopyMem(int argc, char** argv) {
DWORD datalen,tmp;
DWORD *src;
DWORD *dst;
DWORD i;

    if (argc<4) {
		printf("Usage:\n");
		printf("\t %s <src addr> <dst addr> <length in byte>\n\n",argv[0]);
		return;
	}

    if (yl_strtoaddr(argv[1],&tmp)) return;
	src=(DWORD*)(tmp&0xfffffffc);

    if (yl_strtoaddr(argv[2],&tmp)) return;
	dst=(DWORD*)(tmp&0xfffffffc);

    if (yl_strtoul(argv[3],&datalen)) return;
	for(i=0;i<(datalen/4);i++){
	    *dst++=*src++;
	}

	return;
}

