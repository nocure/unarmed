/*
 * tq_irq.c
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */
#include "platform/TQ_BSP.h"
#include "2440addr.h"

int __isr_do_nothing(void);
extern ISR __irq_vector_table[];


void tq_irqSetISR(DWORD irq, ISR isr) {
	if (irq>31) return;
	__irq_vector_table[irq]=isr;
}

void tq_irqClearISR(DWORD irq) {
	if (irq>31) return;
	__irq_vector_table[irq]=__isr_do_nothing;
}


void tq_irqInit(void) {
//	rINTMSK=BIT_ALLMSK;	  // All interrupt is masked.
//	rINTSUBMSK=BIT_SUB_ALLMSK;

	rINTMOD=0x0;	  // All=IRQ mode
	rSRCPND=BIT_ALLMSK;	// clear source pending
	rSUBSRCPND=BIT_SUB_ALLMSK;
	rINTPND=BIT_ALLMSK;	// clear interrupt pending

	rEINTPEND=BIT_ALLMSK;	// clear eint pendings
	rEINTMASK=BIT_ALLMSK;	// disable all eints

}

