#include <stdlib.h>
#include "platform/YL_SYS.h"

int xmodemReceive(unsigned char *dest, int destsz);
int xmodemTransmit(unsigned char *src, int srcsz);

void uconXLoad(int argc, char** argv) {
DWORD addr,len,lenrecv;

	if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <max length>\n",argv[0]);
		printf("\t *** This function will modify the content of specified memory\n");
		printf("\t *** Use with caution !!!\n\n");
		return;
	}

	if (yl_strtoaddr(argv[1],&addr)) return;
	if (yl_strtoul(argv[2],&len)) return;

	printf("Load Address: 0x%8x Maximum Length: 0x%02x\n",addr,len);
	printf("Press Ctrl-D to abort transfering\n");

	lenrecv=xmodemReceive((unsigned char*)addr,(int)len);

	printf("\n%d bytes received\n",lenrecv);


}

void uconXSend(int argc, char** argv) {
DWORD addr,len,lensent;

	if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <length>\n",argv[0]);
		return;
	}
	if (yl_strtoaddr(argv[1],&addr)) return;
	if (yl_strtoul(argv[2],&len)) return;

	printf("Send Address: 0x%8x Length: 0x%02x\n",addr,len);
//	printf("Press Ctrl-D to abort transfering\n");

	lensent=xmodemTransmit((unsigned char*)addr,(int)len);

	printf("\n%d bytes sent\n",lensent);


}
