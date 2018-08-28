/**************************************************************************\

	ww: 2012-0312


\**************************************************************************/

	.section ".vect"

/* Define the ARM vector area.  This should be located or copied to 0.  */

    .global  __vectors
__vectors:

    b __reset							// Reset goes to startup function
    ldr pc,__undefined_v				// Undefined handler
    ldr pc,__swi_interrupt_v			// Software interrupt handler
    ldr pc,__prefetch_handler_v		// Prefetch exeception handler
    ldr pc,__abort_handler_v			// Abort exception handler
    ldr pc,.						// Reserved exception handler
    ldr pc,__irq_handler_v			// IRQ interrupt handler
    ldr	pc,__fiq_handler_v			// FIQ interrupt handler

	__undefined_v:			.word OS_CPU_ARM_ExceptUndefInstrHndlr
	__swi_interrupt_v:		.word OS_CPU_ARM_ExceptSwiHndlr
	__prefetch_handler_v:	.word OS_CPU_ARM_ExceptPrefetchAbortHndlr
	__abort_handler_v:		.word OS_CPU_ARM_ExceptDataAbortHndlr
	__irq_handler_v:		.word OS_CPU_ARM_ExceptIrqHndlr
	__fiq_handler_v:		.word OS_CPU_ARM_ExceptFiqHndlr

