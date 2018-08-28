#include <platform/yl_Sys.h>
#include <ucos_ii.h>

void cpu_init(void);
void yl_OSWrapInit(void*);

// ==== Memory Bottom ====================================
// *** memory from 0x33ff8000 and higher is for MMU Table, see mmu.c for detials.
const DWORD sdram_end_address=(DWORD)0x33ff8000;

extern unsigned int __SDRAM_FREE_START;

int vconVarSize(void) {
	return 0;
}
void vconTaskInit(void *ptr) {

}

void end(void) {

}

#if 0
void _exit(int e) {
	while(1);
}

void _sbrk(void) {

}

void _kill(void) {

}

void _getpid(void) {

}

void _write(void) {

}

void _close(void) {

}

void _fstat(void) {

}

void _isatty(void) {

}

void _lseek(void) {

}

void _read(void) {

}
#endif
/*********************************************************************
 *  pre-initialization, called before init_tsk
 *  It's not within task context yet
 ********************************************************************/
void appPreInit(DWORD unused_memory) {
	// ** memInit must be last one to call to take the rest of memory as heap
	unused_memory+=0x00010000;	// ww 2014-0116 make the memory 64k align
	unused_memory&=0xffff0000;	// just for convenience, so the variables will not jumping around addresses when code slight modified.
	memInit((unsigned long)unused_memory,sdram_end_address-unused_memory);
}


/*********************************************************************
 *  post-initialization, called within init_tsk
 ********************************************************************/
void post_init(void) {
//	ux_define_msc(); // mass storage class
//	ux_define_cdc(); // communnation device class
}

int app4 (int argc, char* argv[]);
// Define main entry point.
int  main() {

	/***************************************************************
	 * dbgout/printf is not available yet, they can only be used
	 * after kernel enter.
	 ***************************************************************/
	cpu_init();	// setting FCLK(cpu)=400MHz / HCLK=100M / PCLK=50M

	OS_CPU_SR_IRQ_En();

	OSInit();

	yl_OSWrapInit(&__SDRAM_FREE_START);

//	app4(0,0);

	OSStart();

    return 0;
}

#if 0

lwip init
lwip ftp

#endif

