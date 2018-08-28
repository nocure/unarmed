/* Linker script file for TQ2440 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(__vectors)


MEMORY {
	IRAM (wx) : org = 0x00000000, len = 0x1000
	DRAM (wx) : org = 0x30000000, len = 64M
}

/*__CSTART_TEMP_STACK = 0x33fffffc;*/
/*__CSTART_TEMP_STACK = __TIMER_STACK_START -4;*/

SECTIONS
{

	.text : {
		*(.vect)
	} > DRAM AT > IRAM

 	.text : {
		*(.text*)
	} > DRAM

    .rodata : {
    	*(.rodata*)
    } > DRAM

	.data : {
		*(.data*)
	} > DRAM
	
	.bss ALIGN(0x08) : {
	__BSS_START = .;
		*(.bss*)
	} > DRAM

	. = ALIGN(0x08);
	__BSS_END = .;

 	__UND_STACK_START = .;
 	. += 0x10;
 	__UND_STACK_SIZE = . - __UND_STACK_START;
 	__UND_STACK_BOTTOM = . -4;

 	__ABT_STACK_START = .;
 	. += 0x10;
 	__ABT_STACK_SIZE = . - __ABT_STACK_START;
 	__ABT_STACK_BOTTOM = . -4;

 	__FIQ_STACK_START = .;
 	. += 0x10;
 	__FIQ_STACK_SIZE = . - __FIQ_STACK_START;
 	__FIQ_STACK_BOTTOM = . -4;

 	__IRQ_STACK_START = .;
 	. += 1024;
 	__IRQ_STACK_SIZE = . - __IRQ_STACK_START;
 	__IRQ_STACK_BOTTOM = . -4;

 	__SYS_STACK_START = .;
 	. += 1024;
 	__SYS_STACK_SIZE = . - __SYS_STACK_START;
 	__SYS_STACK_BOTTOM = . -4;

 	__SVC_STACK_START = .;
 	. += 1024;
 	__SVC_STACK_SIZE = . - __SVC_STACK_START;
 	__SVC_STACK_BOTTOM = . -4;

/*
 	__KERNEL_STACK_START = .;

 	__FIQ_STACK_START = .;
 	. += 512;
 	__FIQ_STACK_SIZE = . - __FIQ_STACK_START;

 	__SYS_STACK_START = .;
 	. += 1024;
 	__SYS_STACK_SIZE = . - __SYS_STACK_START;

 	__TIMER_STACK_START = .;
 	. += 1024;
 	__TIMER_STACK_SIZE = . - __TIMER_STACK_START;
*/
    
/*	__SDRAM_SAFE_MARKER = .;
    . += 0x8; */

	. = ALIGN(0x10);

    __SDRAM_FREE_START = .;

}


