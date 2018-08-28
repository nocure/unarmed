#include <platform/yl_type.h>
#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include "tq_hw_defs.h"
#include <string.h>
#include <stdlib.h>
#include "platform/YL_SYS.h"

#define TIMER_1_START (1<<8)
#define TIMER_1_UPDATE (1<<9)
#define TIMER_1_INVERT (1<<10)
#define TIMER_1_RELOAD (1<<11)
static U32 pwm_sta =0;

void tq_PwmInit(U32 mode,U32 timeH,U32 timeL) {
	if((timeH<=0)||(timeL<0)){
		return;
	}
	BIT_SET(rCLKCON,CLK_CON_PWMTIMER);	// pwm power on
	if(mode==BIT_TIMER1){
		BIT_CLR(rGPBCON,(3<<2));//set GPB1 as TOUT1
		BIT_SET(rGPBCON,(1<<3));
		BIT_CLR(rGPBUP,(1<<1));//enble TOUT1 function
		BIT_CLR(rTCFG0,0xff);      // prescaler 0 = 1 = (0+1)
		BIT_CLR(rTCFG1,(0xf)<<4);  // MUX1 = 1/2
		rTCNTB1 = timeH+timeL;
		rTCMPB1 = timeL;
		BIT_SET(rTCON,TIMER_1_START|TIMER_1_UPDATE|TIMER_1_INVERT|TIMER_1_RELOAD);
		BIT_CLR(rTCON,TIMER_1_UPDATE);
		BIT_SET(pwm_sta,BIT_TIMER1);
	}
}

void tq_PwmSet(U32 mode,U32 timeH,U32 timeL){
	if((timeH<=0)||(timeL<0)){
		return;
	}
	if(!(pwm_sta&mode)){
		tq_PwmInit(mode,timeH,timeL);
	}
	else{
		if(mode==BIT_TIMER1){
			rTCNTB1 = timeH+timeL;
			rTCMPB1 = timeL;
		}
	}
}
#define PWM_DLY 429
void tq_PwmStart(U32 mode){
	//input start patent
	if(mode==BIT_TIMER1){
		BIT_SET(rGPBCON,(1<<2));//GPB1 set output
		BIT_CLR(rGPBCON,(1<<3));
		BIT_SET(rGPBDAT,(1<<1));//GPB1 data set 1
		BIT_SET(rGPBUP,(1<<1));//GPB1 functional disable
		int i = 0;
		for(i=0;i<PWM_DLY;i++);
		BIT_CLR(rGPBDAT,(1<<1));//GPB1 data set 0
		int j = 0;
		for(j=0;j<PWM_DLY;j++);
		BIT_SET(rGPBDAT,(1<<1));//GPB1 data set 1
		for(i=0;i<PWM_DLY;i++);
		BIT_CLR(rGPBDAT,(1<<1));//GPB1 data set 0
		for(j=0;j<PWM_DLY;j++);

		BIT_SET(rGPBCON,(1<<3));//GPB1 set functional
		BIT_CLR(rGPBCON,(1<<2));
		BIT_CLR(rGPBDAT,(1<<1));//GPB1 data set 0
		BIT_CLR(rGPBUP,(1<<1));//GPB1 functional enable
	}
}
void tq_PwmStop(U32 mode){
	if(mode==BIT_TIMER1){
		//make sure pwm would stop at low level
		rTCNTB1 = 100;
		rTCMPB1 = 0;
		int i = 0;
		for(i=0;i<1000;i++);
		BIT_CLR(rTCON,TIMER_1_START|TIMER_1_INVERT|TIMER_1_UPDATE|TIMER_1_RELOAD);
		BIT_CLR(pwm_sta,BIT_TIMER1);
	}
}

U32 tq_PwmGetStatus(void){
	return pwm_sta;
}

void uconPWM(int argc, char** argv){
	if (argc>=2) {
		if(strcmp(argv[1],"1")==0){
			if((argv[2]!=NULL)&&(argv[2]!=NULL)){
				int H = atoi(argv[2]);
				int L = atoi(argv[3]);
				if((H>0)&&(L>0)){
					tq_PwmSet(BIT_TIMER1,H,L);
					yl_uartPrintf("pwm channel 1 set timeH:%d timeL:%d OK\n",H,L);
				}
			}
		}
		else if(strcmp(argv[1],"l")==0){
			BIT_SET(rGPBCON,(1<<2));//GPB1 set output
			BIT_CLR(rGPBCON,(1<<3));
			BIT_CLR(rGPBDAT,(1<<1));//GPB1 set 0
		}
		else if(strcmp(argv[1],"h")==0){
			BIT_SET(rGPBCON,(1<<2));//GPB1 set output
			BIT_CLR(rGPBCON,(1<<3));
			BIT_SET(rGPBDAT,(1<<1));//GPB1 set 0
		}
		else if(strcmp(argv[1],"c")==0){
			yl_uartPrintf("pwm channel 1 Now timeH:%d timeL:%d OK\n",(rTCNTB1-rTCMPB1),rTCMPB1);
		}
		else if(strcmp(argv[1],"t")==0){
			//tq_PwmSet(BIT_TIMER1,38,910);
			tq_PwmSet(BIT_TIMER1,90,275);
			dly_tsk(1000);
			tq_PwmSet(BIT_TIMER1,90,125);
			dly_tsk(1000);
			tq_PwmSet(BIT_TIMER1,38,24);
			dly_tsk(1000);
			tq_PwmSet(BIT_TIMER1,38,12);
		}
		else if(strcmp(argv[1],"off")==0){
			tq_PwmStop(BIT_TIMER1);
			yl_uartPrintf("pwm channel 1 off\n");
		}
		else{
			yl_uartPrintf("unknow command\n");
		}
	}
}




