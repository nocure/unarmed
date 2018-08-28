/*
 * yl_timer.c
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */

#include "platform/YL_sys.h"
#include "platform/tq_bsp.h"
#include "2440addr.h"
#include "tq_hw_defs.h"
#include <sct.h>

//=============================================================================

static QWORD g_SystemTimerTick;
//static QWORD g_SystemPreciseTimerTick = 0;
//static int g_btimeinit = 0;

//=============================================================================

void cycSystemTickHandler(unsigned long thread_input) {

#if 0
extern int __SDRAM_SAFE_MARKER;
register DWORD m1,m2;
	m1=REG32(&__SDRAM_SAFE_MARKER+0);
	m2=REG32(&__SDRAM_SAFE_MARKER+1);
	if (((m1|m2)!=0xffffffff) || ((m1&m2)!=0) ) {
		// eku system has crossed the line !!!
		dbgout("EKU MEMORY OVERLAPED\n");
	}
#endif

	g_SystemTimerTick++;
}


void OSTimeTick (void);
static int __isr_timer4(void) {
	OSTimeTick();
	BIT_SET(rSRCPND,BIT_TIMER4);
	BIT_SET(rINTPND,BIT_TIMER4);
	return 0;
}


QWORD tq_timerGetTimerTick(void) {
YL_PSR_SAVE_AREA;
QWORD t;
	XDISABLE;
	t=g_SystemTimerTick;
	XRESTORE;
	return t;
}



void tq_timerInit(void) {
	g_SystemTimerTick=0;

	// configure timer4 to 1000ms for OS timer tick
	BIT_SET(rCLKCON,CLK_CON_PWMTIMER);	// enable clock

	rTCON=0;

	//50000000 / (24+1) / 2 / 1000 = 1000
	rTCFG0=(24<<8);		// prescaler 1 = 25 = (24+1)
	rTCFG1=(0x0<16);	// MUX4 = 1/2
	rTCNTB4=1000;
	rTCON =(0x7<<20);	// start and update
	BIT_CLR(rTCON,(1<<21));	// clear update bits

	tq_irqSetISR(INT_TIMER4,__isr_timer4);

	ClearPending(BIT_TIMER4);
	EnableIrq(BIT_TIMER4);

}
#if 0
typedef struct _st_timer{
	struct _st_timer* Next;
	int task_ID;
	long long t_rec;
}st_timer;

static st_timer* Thead = NULL;
void timer_Add(st_timer* tnew){
	if(tnew==NULL)
		return;
	st_timer* p = Thead;

	if(Thead){
		while(p!=NULL){
			p = (st_timer*)p->Next;
		}
		tnew->Next=NULL;
		p = tnew;
	}
	else{
		tnew->Next=NULL;
		Thead = tnew;
	}
}

ER timer_delete(int task_id){
	ER er = ERROR;
	st_timer* p = Thead;
	st_timer* pre = NULL;
	while(p!=NULL){
		pre = p;
		if(p->task_ID==task_id){
			er = E_OK;
			break;
		}
		p = p->Next;
	}
	if(er!=E_OK)
		return er;

	if(pre == p){
		Thead = NULL;
	}
	else if(p->Next==NULL){
		pre->Next = NULL;
	}
	else{
		pre->Next = p->Next;
	}
	free(p);
	return er;
}

void dly_tsk_H(int dlytim){
	if(dlytim<=0)
		return;
	loc_mtx(SEMID_MTX_TIMER);
	st_timer* tnew = malloc(sizeof(st_timer));
	tnew->t_rec = dlytim;
	get_tid((ID*)&tnew->task_ID);
	if(tnew->task_ID>=0){
		timer_Add(tnew);
	}
	unl_mtx(SEMID_MTX_TIMER);
	slp_tsk();

	loc_mtx(SEMID_MTX_TIMER);
	timer_delete(tnew->task_ID);
	unl_mtx(SEMID_MTX_TIMER);
}

void timer_handler(void){
	st_timer* p = Thead;
	while(p!=NULL){
		if(!(--p->t_rec)){
			wup_tsk(p->task_ID);
			p->task_ID = p->task_ID|0x80000000;
			break;
		}
		p = p->Next;
	}
}

int PreciseTimerHandler(void) {
	if(Thead!=NULL){
		timer_handler();
	}

	BIT_SET(rSRCPND,BIT_TIMER3);
	BIT_SET(rINTPND,BIT_TIMER3);
	return 0;
}

#define TIMER_3_RELOAD (1<<19)
#define TIMER_3_INVERT (1<<18)
#define TIMER_3_UPDATE (1<<17)
#define TIMER_3_START  (1<<16)
//unit of time is micro second
void tq_precisetimerInit(void){
	if(g_btimeinit!=0)
		return;
	BIT_SET(rCLKCON,CLK_CON_PWMTIMER);	// pwm power on
	BIT_CLR(rINTMSK,BIT_TIMER3);//enable interrup
	tq_irqSetISR(13, PreciseTimerHandler);

	rTCFG0=(24<<8);		// prescaler 1 = 25 = (24+1)
	rTCFG1=(0x0<<12);	// MUX3 = 1/2
	rTCNTB3=1;
	BIT_SET(rTCON,TIMER_3_START|TIMER_3_UPDATE|TIMER_3_INVERT|TIMER_3_RELOAD);
	BIT_CLR(rTCON,TIMER_3_UPDATE);

}
#endif
