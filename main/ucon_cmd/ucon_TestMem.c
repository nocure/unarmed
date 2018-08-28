#include <stdlib.h>
#include <platform/tq_bsp.h>
#include <platform/yl_sys.h>

const DWORD pat_aa = 0xAAAAAAAA;
const DWORD pat_55 = 0x55555555;
const DWORD pat_00 = 0x00000000;
const DWORD pat_ff = 0xffffffff;

DWORD* uconTestMemory(volatile DWORD *baseAddress, DWORD nBytes) {
int i,val,delay;

	delay=0;
	nBytes>>=2;

	for (i=0;i<nBytes;i++) {
		if ((i&0xfff)==0) {
			printf("%08x\r",&baseAddress[i]);
			val=tq_uartGetCharNB();
			if (val==0x03) {
				printf("\n");
				break;	// CTRL-C
			}
		}
		baseAddress[i]=pat_aa;delay++;
		baseAddress[i]=pat_55;delay++;
		baseAddress[i]=pat_aa;delay++;delay++;delay++;
		if (baseAddress[i]!=pat_aa) return (DWORD*)&baseAddress[i];

		baseAddress[i]=pat_00;delay++;
		baseAddress[i]=pat_ff;delay++;
		baseAddress[i]=pat_00;delay++;delay++;delay++;
		if (baseAddress[i]!=pat_00) return (DWORD*)&baseAddress[i];

		baseAddress[i]=pat_55;delay++;
		baseAddress[i]=pat_aa;delay++;
		baseAddress[i]=pat_55;delay++;delay++;delay++;
		if (baseAddress[i]!=pat_55) return (DWORD*)&baseAddress[i];

		baseAddress[i]=pat_ff;delay++;
		baseAddress[i]=pat_00;delay++;
		baseAddress[i]=pat_ff;delay++;delay++;delay++;
		if (baseAddress[i]!=pat_ff) return (DWORD*)&baseAddress[i];

		val=rand();
		baseAddress[i]=val;delay++;
		val=rand();
		baseAddress[i]=val;delay++;
		val=rand();
		baseAddress[i]=val;delay++;delay++;delay++;
		if (baseAddress[i]!=val) return (DWORD*)&baseAddress[i];
	}

	return (0);

}


void uconMemTest(int argc,char** argv ) {
DWORD addr;
DWORD len;
char *ep;

	if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <length>\n",argv[0]);
		printf("\t *** This test will modify the content of specified memory\n");
		printf("\t *** Use with caution !!!\n\n");
		return;
	}
	if (argc>=2) {
	if (yl_strtoaddr(argv[1],(DWORD*)&addr)) return;
	if (yl_strtol(argv[2],(int*)&len)) return;
		printf("Press CTRL-C to abort test\n");

		ep=(char*)uconTestMemory((DWORD*)addr,len);
		if(ep != NULL){
			printf("Memory Error at %08x\n",ep);
		}
		else {
			printf("Memory OK\n");
		}
	}
}


