/*
 * yl_uart.c
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */
#include <sct.h>
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include "tq_hw_defs.h"

static DWORD g_uartTimeOut;

int uart0InterruptHandler(void) {
int d,rcnt;

	if (BIT_VAL(rSUBSRCPND,BIT_SUB_ERR0)) {
		d=rUERSTAT0;
//		if (BIT_VAL(d,UART_ERSTAT_BR|UART_ERSTAT_FE|UART_ERSTAT_PE|UART_ERSTAT_OV)) {
		BIT_SET(rUFCON0,6); // clear tx&rx fifo
		snd_msg(MBXID_UART_RECV,(void*)(UART_RECV_ERROR));
		rSUBSRCPND=BIT_SUB_ERR0;
	}
	if (BIT_VAL(rSUBSRCPND,BIT_SUB_RXD0)) {
		//	if (BIT_VAL(rUTRSTAT0,UART_TRSTAT_DR)) {
		while (1) {
			rcnt=rUFSTAT0&0x3f;
			if (!rcnt) break;
			while (rcnt--) {
				d=rURXH0;
				// **Note that snd_msg will return error if msg is NULL
				snd_msg(MBXID_UART_RECV,(void*)(d|UART_DATA_READY));
			}
		}
		rSUBSRCPND=BIT_SUB_RXD0;
	}

	BIT_SET(rSRCPND,BIT_UART0);
	BIT_SET(rINTPND,BIT_UART0);
	return 0;
}


 // rULCONn:
 //    [10]       [9]     [8]        [7]        [6]      [5]         [4]           [3:2]        [1:0]
 // Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
 //     0          1       0    ,     0          1        0           0     ,       01          01
 //   PCLK       Level    Pulse    Disable    Generate  Normal      Normal        Interrupt or Polling
void tq_uartInit(void) {

	BIT_SET(rCLKCON,CLK_CON_UART0);	// power up uart 0

	rULCON0=0;	// disable all uart channel

	BIT_SET(rINTMSK,BIT_UART0);
	BIT_SET(rINTSUBMSK,BIT_SUB_ERR0|BIT_SUB_TXD0|BIT_SUB_RXD0);

}


void tq_uartEnable(void) {

	tq_irqSetISR(INT_UART0,uart0InterruptHandler);
	BIT_CLR(rINTMSK,BIT_UART0);	// enable interrupt
	BIT_CLR(rINTSUBMSK,BIT_SUB_ERR0|BIT_SUB_RXD0);
}


void tq_uartDisable(void) {

	rUCON0=0;
	BIT_SET(rINTSUBMSK,BIT_SUB_ERR0|BIT_SUB_TXD0|BIT_SUB_RXD0);
	BIT_SET(rINTMSK,BIT_UART0);
	tq_irqClearISR(INT_UART0);
}


void tq_uartClearFifo(void) {
	BIT_SET(rUFCON0,6); // clear tx&rx fifo
}



void tq_uartConfig(UART_CONFIG_T* cfg) {

	rUCON0=0x245;

	// setup baud rate
	rUBRDIV0=UART_BAUD_RATE(cfg->BaudRate);

	rULCON0=0;
	BIT_SET(rULCON0,cfg->DataBits|cfg->Parity|cfg->StopBits);

	rUMCON0=cfg->FlowControl;

	rUFCON0=(1<<6)|(0<<4)|0x7;	//tx trigger 32, rx trigger 16, force clear and enable fifo

	g_uartTimeOut=cfg->TimeOut;
	if (!g_uartTimeOut) g_uartTimeOut=2000;

}


int tq_uartPutChar(DWORD c) {
DWORD timeo=OSTICK+g_uartTimeOut;
//TX_INTERRUPT_SAVE_AREA

	while (BIT_VAL(rUFSTAT0,UART_UFSTAT_TX_FULL)) {
		rot_rdq();	// let other tasks to run while waiting for TX FIFO
		if (time_after(OSTICK,timeo)) return UART_ERROR_SEND;
	}
	//TX_DISABLE;
	rUTXH0=c;
	//TX_RESTORE;
	return 0;

}



int tq_uartGetChar(void) {
DWORD m;
int er;

	er=trcv_msg(MBXID_UART_RECV,(void*)&m,g_uartTimeOut);
	if (er==OSWRAP_TIME_OUT) {
		return UART_ERROR_TIMEOUT;
	}
	else if (er==0) {
		if (m&UART_DATA_READY) {
			return m&0xff;
		}
	}

	return UART_ERROR_RECV;
}


int tq_uartGetCharT(int timeout) {
DWORD m;
int er;

	er=trcv_msg(MBXID_UART_RECV,(void*)&m,timeout);
	if (er==OSWRAP_TIME_OUT) {
		return UART_ERROR_TIMEOUT;
	}
	else if (er==0) {
		if (m&UART_DATA_READY) {
			return m&0xff;
		}
	}

	return UART_ERROR_RECV;
}


int tq_uartGetCharNB(void) {
DWORD m;
int er;

	er=prcv_msg(MBXID_UART_RECV,(void*)&m);
	if (!er) {
		if (m&UART_DATA_READY) {
			return m&0xff;
		}
	}

	return UART_ERROR_TIMEOUT;
}



int tq_uartWrite(BYTE* pbData,int length){
int ret;
	for (;length>0;length--,pbData++) {
		ret=tq_uartPutChar(*pbData);
		if (ret) return ret;
	}
	return 0;
}



int tq_uartRead(BYTE* pbData,int length){
int ret;
	for (;length>0;length--,pbData++) {
		ret=tq_uartGetChar();
		if (ret>0) *pbData=ret;
		else return ret;
	}
	return 0;
}




