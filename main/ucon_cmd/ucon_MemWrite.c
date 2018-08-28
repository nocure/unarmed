#include <stdlib.h>
#include <platform/yl_sys.h>

void uconMemWrite(int argc, char** argv) {
DWORD addr;
DWORD data;

	if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <32bit data>\n",argv[0]);
		printf("\t *** This function will modify the content of specified memory\n");
		printf("\t *** Use with caution !!!\n\n");
		return;
	}
	if (yl_strtoaddr(argv[1],(DWORD*)&addr)) return;
	addr&=0xfffffffc;

	if (yl_strtoul(argv[2],(DWORD*)&data)) return;

	*(DWORD*)addr=data;
	printf("%08x <-- 0x%08x\n",addr,data);

}

