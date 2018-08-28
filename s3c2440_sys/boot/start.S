/*
 * start.S
 *
 *  Created on: 2018¦~8¤ë22¤é
 *      Author: ww
 */


	.text

	/* Stack Sizes */
    .set  UND_STACK_SIZE, 0x00000004
    .set  ABT_STACK_SIZE, 0x00000004
    .set  FIQ_STACK_SIZE, 0x00000004
    .set  IRQ_STACK_SIZE, 0X00000400
    .set  SVC_STACK_SIZE, 0x00000400

	/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */
    .set  MODE_USR, 0x10            /* User Mode */
    .set  MODE_FIQ, 0x11            /* FIQ Mode */
    .set  MODE_IRQ, 0x12            /* IRQ Mode */
    .set  MODE_SVC, 0x13            /* Supervisor Mode */
    .set  MODE_ABT, 0x17            /* Abort Mode */
    .set  MODE_UND, 0x1B            /* Undefined Mode */
    .set  MODE_SYS, 0x1F            /* System Mode */

    .equ  I_BIT, 0x80               /* when I bit is set, IRQ is disabled */
    .equ  F_BIT, 0x40               /* when F bit is set, FIQ is disabled */


.global __reset
__reset:

	/* Setup a stack for each mode - note that this only sets up a usable stack
	for system/user, SWI and IRQ modes.   Also each mode is setup with
	interrupts initially disabled. */
    ldr   r0, =__UND_STACK_BOTTOM
    msr   CPSR_c, #MODE_UND|I_BIT|F_BIT /* Undefined Instruction Mode */
    mov   sp, r0
    ldr   r0, =__ABT_STACK_BOTTOM
    msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT /* Abort Mode */
    mov   sp, r0
    ldr   r0, =__FIQ_STACK_BOTTOM
    msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT /* FIQ Mode */
    mov   sp, r0
    ldr   r0, =__IRQ_STACK_BOTTOM
    msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT /* IRQ Mode */
    mov   sp, r0
    ldr   r0, =__SYS_STACK_BOTTOM
    msr   CPSR_c, #MODE_SYS|I_BIT|F_BIT /* System Mode */
    mov   sp, r0
    ldr   r0, =__SVC_STACK_BOTTOM
    msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT /* Supervisor Mode */
    mov   sp, r0

	/* we now in supervisor mode. */

/* turn off the watchdog */
# define pWTCON		0x53000000
# define INTMSK		0x4A000008	/* Interupt-Controller base addresses */
# define INTSUBMSK	0x4A00001C
# define CLKCON		0x4c00000c	// Clock Control

	ldr r0, =pWTCON
	mov r1, #0x0
	str r1, [r0]

	/*
	 * mask all IRQs by setting all bits in the INTMR - default
	 */
	mov	r1, #0xffffffff
	ldr	r0, =INTMSK
	str	r1, [r0]
	ldr	r1, =0x3ff
	ldr	r0, =INTSUBMSK
	str	r1, [r0]

	// enable only NAND flash controller to enable 0-0x1000 SRAM
	mov r1, #0x10		// 0x00fffff0
	ldr	r0, =CLKCON
	str	r1, [r0]

	mov r0,pc
	lsr r0,#10	// shift out 10bit --> first 1K
	cmp r0,#0
	beq at_addr0	// we are running from addr 0

	// running from RAM, so copy vector table to addr 0
	ldr r0,=__vectors
	mov r1,#4
	mov r2,#0x3c

copyvec:
	ldr r3,[r0,r1]
	str r3,[r1]
	add r1,#4
	cmp r1,r2
	blt copyvec

at_addr0:
//	ldr sp,=__CSTART_TEMP_STACK

clear_bss:
	ldr	r0, =__BSS_START		/* find start of bss segment        */
	ldr	r1, =__BSS_END			/* stop here                        */
	mov r2, #0x00000000			/* clear                            */

clbss_l:
	str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	// everything is ready, go main
	b main

	b .
