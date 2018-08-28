#include <stdlib.h>
#include "platform/YL_SYS.h"

DWORD get_Zmodem(BYTE *p);
DWORD send_Zmodem(BYTE *name, BYTE* addr, int len);

void uconZLoad(int argc, char** argv) {
DWORD addr;

    if (argc<2) {
		printf("Usage:\n");
		printf("\t %s <src addr>\n\n",argv[0]);
		return;
	}

    if (yl_strtoaddr(argv[1],&addr)) return;

    get_Zmodem((BYTE*)addr);

	return;
}


void uconZSend(int argc, char** argv) {
char *name;
DWORD addr;
DWORD len;

    if (argc<4) {
		printf("Usage:\n");
		printf("\t %s <file name> <src addr> <len>\n\n",argv[0]);
		return;
	}

    name=argv[1];
    if (yl_strtoaddr(argv[2],&addr)) return;
    if (yl_strtoul(argv[3],&len)) return;

    send_Zmodem((BYTE*)name, (BYTE*)addr, len);

	return;
}


