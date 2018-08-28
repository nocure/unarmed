/*
 * tq_isr.c
 *
 *  Created on: 2008/12/18
 *      Author: Wesley Wang
 */

#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include "tq_hw_defs.h"
#include <string.h>

/*
NOTE:
	before return from isr,
	first clear srcpnd, then clear intpnd
 */


// the isr things that do not have main source file are placed here


void dm9k_isr(void);
// ==== EINT =========================
static int EINT0InterruptHandler(void) {
	BIT_SET(rSRCPND,BIT_EINT0);
	BIT_SET(rINTPND,BIT_EINT0);		// clear pending
	return 0;
}


static int EINT1InterruptHandler(void) {
	BIT_SET(rSRCPND,BIT_EINT1);
	BIT_SET(rINTPND,BIT_EINT1);		// clear pending
	return 0;
}


static int EINT2InterruptHandler(void) {
	BIT_SET(rSRCPND,BIT_EINT2);
	BIT_SET(rINTPND,BIT_EINT2);		// clear pending
	return 0;
}


static int EINT3InterruptHandler(void) {
	BIT_SET(rSRCPND,BIT_EINT3);
	BIT_SET(rINTPND,BIT_EINT3);		// clear pending
	return 0;
}


static int EINT4_7InterruptHandler(void) {
DWORD eintno,pending,clearpending=0;

	pending=rEINTPEND;
	while(1) {
		eintno=ffs(pending);

		if (!eintno) break;;
		eintno--;

		switch(eintno) {
		case 4:
			clearpending=BIT_EINT4;
			break;

		case 5:
			clearpending=BIT_EINT5;
			break;

		case 6:
			clearpending=BIT_EINT6;
			break;

		case 7:
			dm9k_isr();
			clearpending=BIT_EINT7;
			break;
		}

		BIT_SET(rEINTPEND,clearpending);	// clear pending
		BIT_CLR(pending,clearpending);		// clear temp
	}
	BIT_SET(rSRCPND,BIT_EINT4_7);
	BIT_SET(rINTPND,BIT_EINT4_7);
	return 0;
}


static int EINT8_23InterruptHandler(void) {
DWORD eintno,pending,clearpending;

	pending=rEINTPEND;
	while(1) {
		eintno=ffs(pending);

		if (!eintno) break;;
		eintno--;

		switch(eintno) {
		case 16:
//			stat=tq_gpioGet(PORT_G|SD_CARD_DET);	//GPG->06, SD Card detect, 0->card insert, 1->card remove
			break;

		}

		clearpending=1<<eintno;
		BIT_SET(rEINTPEND,clearpending);		// clear pending
		BIT_CLR(pending,clearpending);		// clear temp
	}
	BIT_SET(rSRCPND,BIT_EINT8_23);
	BIT_SET(rINTPND,BIT_EINT8_23);
	return 0;
}


/* NOTE:
Card detect, EINT16 can not be used as interrupt,
because the SD card jack mechanism, that causes
hi and lo edge interrupts when poll out card
*/
int tq_eintInit(void) {
	tq_irqSetISR(EINT0,EINT0InterruptHandler);
	tq_irqSetISR(EINT1,EINT1InterruptHandler);
	tq_irqSetISR(EINT2,EINT2InterruptHandler);
	tq_irqSetISR(EINT3,EINT3InterruptHandler);
	tq_irqSetISR(EINT4_7,EINT4_7InterruptHandler);
	tq_irqSetISR(EINT8_23,EINT8_23InterruptHandler);
	EnableIrq(BIT_EINT0);	// receive interrupt
	EnableIrq(BIT_EINT1);	// receive interrupt
	EnableIrq(BIT_EINT2);	// receive interrupt
	EnableIrq(BIT_EINT3);	// receive interrupt
	EnableIrq(BIT_EINT4_7);	// receive interrupt
	EnableIrq(BIT_EINT8_23);	// receive interrupt

	// setup Interrupt Filter Register
	BIT_CLR(rEINTFLT2,0xff);		// set up eint16 filtering width
	BIT_SET(rEINTFLT2,0x7f);

	BIT_CLR(rEINTMASK,BIT_EINT0|BIT_EINT1|BIT_EINT2|BIT_EINT4);
	return 0;
}
