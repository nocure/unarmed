#include <platform/yl_type.h>
#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include <sct.h>
#include "tq_hw_defs.h"
#include <platform/yl_hmi.h>
#include <string.h>



extern 	int yl_uartPrintf(const char *format, ...);
#if 0
	#define Motor_printf yl_uartPrintf
#else
	#define Motor_printf(...)
#endif

#define MOTOMSG_MASK	0x00ffffff
#define MOTOMSG(x) 		((void*)(0x80000000|(x)))

#define MOTO_FREE 0x01
#define MOTO_BUSY 0x02

//steps per second
#define MOTO_Z_SPEED_1X (0)
#define MOTO_Z_SPEED_2X (1)

#define MOTO_AF_SPEED_1X (1)
#define MOTO_AF_SPEED_2X (2)


static int moto_sta;
static int moto_zoom_pos;
static int moto_zoom_sp; //zoom speed
static int moto_AF_sp; //zoom speed

static void* MotoSleepTaskID = 0;
#define MotoIOSet(r,b,h)	(\
{(h) ? (BIT_SET(r,b)) : (BIT_CLR(r,b));}\
)


//motor CW/CCW pin
#define FOCUS2A (1<<6)
#define FP_2A_MCLR (3<<12)
#define FP_2A_MSET (1<<12)

#define FOCUS2B (1<<5)
#define FP_2B_MCLR (3<<10)
#define FP_2B_MSET (1<<10)

#define FOCUS3A (1<<2)
#define FP_3A_MCLR (3<<4)
#define FP_3A_MSET (1<<4)

#define FOCUS3B (1<<1)
#define FP_3B_MCLR (3<<2)
#define FP_3B_MSET (1<<2)



//motor zoom pin
#define ZOOM1A (1<<12)
#define ZOOM1B (1<<13)

//motor Iris pin
#define IRIS4A (1<<9)
#define FP_4A_MCLR (3<<18)
#define FP_4A_MSET (1<<18)

#define IRIS4B (1<<10)
#define FP_4B_MCLR (3<<20)
#define FP_4B_MSET (1<<20)


//motor shutter
#define SIN6A (1<<14)
#define SIN6B (1<<16)


#if 1
#define FPORT1A rGPADAT //Zoom
#define FPORT2A rGPFDAT //AF1
#define FPORT3A rGPFDAT //AF2
#define FPORT4A rGPBDAT //IRIS
//#define FPORT5A
#define FPORT6A rGPADAT //shutter

#define FPCONFIG_1A rGPACON
#define FPCONFIG_2A rGPFCON
#define FPCONFIG_3A rGPFCON
#define FPCONFIG_4A rGPBCON
//#define FPCONFIG_5A
#define FPCONFIG_6A rGPACON



#else
#define FPORT1A rGPBDAT
#define FPORT2A rGPFDAT //port f
#define FPORT3A rGPGDAT //port g
#define FPORT4A rGPGDAT //port g
//#define FPORT5A
#define FPORT6A rGPEDAT
#endif

#define H 1
#define L 0

#if 0
#define ZLL MotoIOSet(FPORT1A,ZOOM1A,L);MotoIOSet(FPORT4A,IRIS4A,L);\
			MotoIOSet(FPORT1A,ZOOM1B,H);MotoIOSet(FPORT4A,IRIS4B,H);
#define ZLH MotoIOSet(FPORT1A,ZOOM1A,L);MotoIOSet(FPORT4A,IRIS4A,H);\
			MotoIOSet(FPORT1A,ZOOM1B,H);MotoIOSet(FPORT4A,IRIS4B,L);
#define ZHL MotoIOSet(FPORT1A,ZOOM1A,H);MotoIOSet(FPORT4A,IRIS4A,L);\
			MotoIOSet(FPORT1A,ZOOM1B,L);MotoIOSet(FPORT4A,IRIS4B,H);
#define ZHH MotoIOSet(FPORT1A,ZOOM1A,H);MotoIOSet(FPORT4A,IRIS4A,H);\
			MotoIOSet(FPORT1A,ZOOM1B,L);MotoIOSet(FPORT4A,IRIS4B,L);
#define ZOFF MotoIOSet(FPORT1A,ZOOM1A,L);MotoIOSet(FPORT4A,IRIS4A,L);\
			 MotoIOSet(FPORT1A,ZOOM1B,L);MotoIOSet(FPORT4A,IRIS4B,L);
#endif
#define SLH MotoIOSet(FPORT4A,IRIS4A,L);MotoIOSet(FPORT6A,SIN6A,H);\
			MotoIOSet(FPORT4A,IRIS4B,H);MotoIOSet(FPORT6A,SIN6B,L);
#define SHL MotoIOSet(FPORT4A,IRIS4A,H);MotoIOSet(FPORT6A,SIN6A,L);\
			MotoIOSet(FPORT4A,IRIS4B,L);MotoIOSet(FPORT6A,SIN6B,H);
#define SLL MotoIOSet(FPORT4A,IRIS4A,L);MotoIOSet(FPORT6A,SIN6A,L);\
			MotoIOSet(FPORT4A,IRIS4B,H);MotoIOSet(FPORT6A,SIN6B,H);
#define SHH MotoIOSet(FPORT4A,IRIS4A,H);MotoIOSet(FPORT6A,SIN6A,H);\
			MotoIOSet(FPORT4A,IRIS4B,L);MotoIOSet(FPORT6A,SIN6B,L);
#define SOFF MotoIOSet(FPORT4A,IRIS4A,L);MotoIOSet(FPORT6A,SIN6A,L);\
			 MotoIOSet(FPORT4A,IRIS4B,L);MotoIOSet(FPORT6A,SIN6B,L);


#define FLL MotoIOSet(FPORT2A,FOCUS2A,L);MotoIOSet(FPORT2A,FOCUS2B,H);\
			MotoIOSet(FPORT3A,FOCUS3A,L);MotoIOSet(FPORT3A,FOCUS3B,H);
#define FLH MotoIOSet(FPORT2A,FOCUS2A,L);MotoIOSet(FPORT2A,FOCUS2B,H);\
			MotoIOSet(FPORT3A,FOCUS3A,H);MotoIOSet(FPORT3A,FOCUS3B,L);
#define FHL MotoIOSet(FPORT2A,FOCUS2A,H);MotoIOSet(FPORT2A,FOCUS2B,L);\
			MotoIOSet(FPORT3A,FOCUS3A,L);MotoIOSet(FPORT3A,FOCUS3B,H);
#define FHH MotoIOSet(FPORT2A,FOCUS2A,H);MotoIOSet(FPORT2A,FOCUS2B,L);\
			MotoIOSet(FPORT3A,FOCUS3A,H);MotoIOSet(FPORT3A,FOCUS3B,L);
#define FOFF MotoIOSet(FPORT2A,FOCUS2A,L);MotoIOSet(FPORT2A,FOCUS2B,L);\
			 MotoIOSet(FPORT3A,FOCUS3A,L);MotoIOSet(FPORT3A,FOCUS3B,L);

int MotoGetSta(int mode){
	int rtn = -1;
	switch(mode){
		case Z_SPEED:
			rtn = moto_zoom_sp;
			break;
		case AF_SPEED:
			rtn = moto_AF_sp;
			break;
		case Z_POS:
			rtn = moto_zoom_pos;
			break;
		case AF_POS:
			break;
		case Z_STA:
			break;
		case AF_STA:
			rtn = moto_sta;
			break;
	}
	return rtn;
}

void MotoSetSta(int mode, int data){
	switch(mode){
		case Z_SPEED:
			moto_zoom_sp = data;
			break;
		case AF_SPEED:
			moto_AF_sp = data;
			break;
		case Z_POS:
			moto_zoom_pos = data;
			break;
		case AF_POS:
			break;
		case Z_STA:
			break;
		case AF_STA:
			moto_sta = data;
			break;
	}
}


void tq_MotoInit(void){
	Motor_printf("Motor init Ok\n");

	//focus
	BIT_CLR(FPCONFIG_2A,FP_2A_MCLR);
	BIT_SET(FPCONFIG_2A,FP_2A_MSET);
	BIT_CLR(FPCONFIG_2A,FP_2B_MCLR);
	BIT_SET(FPCONFIG_2A,FP_2B_MSET);
	BIT_CLR(FPCONFIG_3A,FP_3A_MCLR);
	BIT_SET(FPCONFIG_3A,FP_3A_MSET);
	BIT_CLR(FPCONFIG_3A,FP_3B_MCLR);
	BIT_SET(FPCONFIG_3A,FP_3B_MSET);

	//zoom
	BIT_CLR(FPCONFIG_1A,ZOOM1A);//output only
	BIT_CLR(FPCONFIG_1A,ZOOM1B);//output only

	//SHUTTER
	BIT_CLR(FPCONFIG_6A,SIN6A);//output only
	BIT_CLR(FPCONFIG_6A,SIN6B);//output only

	//IRIS
	BIT_CLR(FPCONFIG_4A,FP_4A_MCLR);
	BIT_SET(FPCONFIG_4A,FP_4A_MSET);
	BIT_CLR(FPCONFIG_4A,FP_4B_MCLR);
	BIT_SET(FPCONFIG_4A,FP_4B_MSET);

	//data initial is low
	BIT_CLR(FPORT1A,(ZOOM1A|ZOOM1B));
	BIT_CLR(FPORT2A,(FOCUS2A|FOCUS2B));
	BIT_CLR(FPORT3A,(FOCUS3A|FOCUS3B));
	BIT_CLR(FPORT4A,(IRIS4A|IRIS4B));
	BIT_CLR(FPORT6A,(SIN6A|SIN6B));

	//functional disable
	BIT_SET(rGPBUP,(IRIS4A|IRIS4B));
	BIT_SET(rGPFUP,(FOCUS2A|FOCUS2B|FOCUS3A|FOCUS3B));

	//moto_sta = MOTO_FREE;
	MotoSetSta(AF_STA, MOTO_FREE);
	//moto_zoom_pos = 0;
	MotoSetSta(Z_POS, 0);
	//moto_zoom_sp = MOTO_Z_SPEED_1X;
	MotoSetSta(Z_SPEED, MOTO_Z_SPEED_1X);
	//moto_AF_sp = MOTO_AF_SPEED_1X;
	MotoSetSta(AF_SPEED, MOTO_AF_SPEED_1X);
}


//maximun delay time
#define MOTOMAXDELAYTIME 65535
void MotoDelay100us(int count){
	if(count<=0)
		return;
	DWORD dt = 100*count;
	if(dt<=MOTOMAXDELAYTIME)
		tq_MotoDelay(dt);
	else{
		Motor_printf("%s Warning Too much delay time\n",__func__);
		tq_MotoDelay(MOTOMAXDELAYTIME);
	}
}

#define MOTOFOCUS_DLY (13)
void tq_MotoFocus(int mode){
	if(MOTO_BUSY==moto_sta){
		Motor_printf("Motor is busy now\n");
		return;
	}
	int i = 0;
	//int t = 10000/moto_AF_sp;
	int t = MotoGetSta(AF_SPEED);

	if(mode==FOCUS_CW){
		moto_sta = MOTO_BUSY;
		Motor_printf("Motor FOCUS_CW\n");
		FHH;
		MotoDelay100us(100);
		for(;i<90;i++){
			FLH;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FLL;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FHL;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FHH;
			MotoDelay100us(MOTOFOCUS_DLY*t);
		}
		MotoDelay100us(85);
		FOFF;
		Motor_printf("rGPGDAT FOFF::%8x\n",rGPGDAT);
		moto_sta = MOTO_FREE;
	}
	else if(mode==FOCUS_CCW){
		moto_sta = MOTO_BUSY;
		Motor_printf("Motor FOCUS_CCW\n");
		FHH;
		MotoDelay100us(100);
		for(;i<90;i++){
			FHL;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FLL;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FLH;
			MotoDelay100us(MOTOFOCUS_DLY*t);
			FHH;
			MotoDelay100us(MOTOFOCUS_DLY*t);
		}
		MotoDelay100us(85);
		FOFF;
		moto_sta = MOTO_FREE;
	}
	else{
	}
}

void MotoZoomIO(int count,DWORD pin){
	int i=0;
	int t= MotoGetSta(Z_SPEED);
	for(i=0;i<count;i++){
		MotoIOSet(FPORT1A,pin,H);
		MotoDelay100us(100);
		MotoIOSet(FPORT1A,pin,L);
		MotoDelay100us(100*t);
	}
}

//close<-->tele<-->mean<-->wide
//0                                          245
void tq_MotoZoom(int mode){
//int i;
	switch(mode){

		case ZOOM_CW://close to wide
			MotoIOSet(FPORT1A,ZOOM1A,L);
			MotoZoomIO(21,ZOOM1B);
			break;
		case ZOOM_WC://wide to close
			MotoIOSet(FPORT1A,ZOOM1B,L);
			MotoZoomIO(21,ZOOM1A);
			break;
		case ZOOM_CW_1://close to wide
			MotoIOSet(FPORT1A,ZOOM1A,L);
			MotoZoomIO(1,ZOOM1B);
			break;
		case ZOOM_WC_1://wide to close
			MotoIOSet(FPORT1A,ZOOM1B,L);
			MotoZoomIO(1,ZOOM1A);
			break;
	}
}

void tq_MotoShutter(int mode){
	switch(mode){
		case SHUTTER_OPEN:
			MotoIOSet(FPORT6A,SIN6A,H);
			MotoIOSet(FPORT6A,SIN6B,L);
			MotoDelay100us(500);
			MotoIOSet(FPORT6A,SIN6A,L);
			break;
		case SHUTTER_CLOSE:
			MotoIOSet(FPORT6A,SIN6A,L);
			MotoIOSet(FPORT6A,SIN6B,H);
			MotoDelay100us(250);
			MotoIOSet(FPORT6A,SIN6B,L);
			break;
		case SHUTTER_BIG:
			SLL;
			MotoDelay100us(250);
			SOFF;
			break;
	}
}

void tq_MotoIris(int mode){
	switch(mode){
		case IRIS_OPEN:
			MotoIOSet(FPORT4A,IRIS4A,L);
			MotoIOSet(FPORT4A,IRIS4B,H);
			MotoDelay100us(250);
			MotoIOSet(FPORT4A,IRIS4B,L);
			break;
		case IRIS_CLOSE:
			MotoIOSet(FPORT4A,IRIS4A,H);
			MotoIOSet(FPORT4A,IRIS4B,L);
			MotoDelay100us(250);
			MotoIOSet(FPORT4A,IRIS4A,L);
			break;
	}
}

void tq_MotoReset(void){
	tq_MotoZoom(ZOOM_RESET);
	tq_MotoFocus(FOCUS_CW);
	tq_MotoShutter(SHUTTER_BIG);
}

#define TIMER_3_RELOAD (1<<19)
#define TIMER_3_INVERT (1<<18)
#define TIMER_3_UPDATE (1<<17)
#define TIMER_3_START  (1<<16)

int MotoInt(void) {
	BIT_CLR(rTCON,TIMER_3_START);
	BIT_SET(rINTMSK,BIT_TIMER3);//disable interrup
	//wup_tsk(TSKID_UI);
	static SYS_MSG msg;
	msg.id = MOTO_TASK_WAKEUP;
	msg.data_1 = (int)MotoSleepTaskID;
	//snd_msg(MBXID_MOTOR_DRV,UIMSG(TASK_WAKEUP));
	//wai_flg()
	if(0==snd_msg(MBXID_WAKEUP_RECV,&msg)){
		Motor_printf("send msg out\n");
	}
	else{
		Motor_printf("send msg fail\n");
	}

	//wup_tsk(MotoSleepTaskID);

	return 0;
}

void tq_MotoDelay(DWORD time) {
	if(MotoSleepTaskID!=0){
		Motor_printf("%d task is using this timer, can't be used by your task!!\n",MotoSleepTaskID);
		return;
	}

	MotoSleepTaskID = get_tid();
	if(MotoSleepTaskID==0){
		yl_uartPrintf("Get task ID fail\n");
		return;
	}
	BIT_SET(rCLKCON,CLK_CON_PWMTIMER);	// pwm power on
	BIT_CLR(rINTMSK,BIT_TIMER3);//enable interrup
	tq_irqSetISR(13, MotoInt);
	rTCFG0=(24<<8);		// prescaler 1 = 25 = (24+1)
	rTCFG1=(0x0<<12);	// MUX3 = 1/2
	rTCNTB3=time;
	BIT_SET(rTCON,TIMER_3_START|TIMER_3_UPDATE|TIMER_3_INVERT|TIMER_3_RELOAD);
	BIT_CLR(rTCON,TIMER_3_UPDATE);
	slp_tsk();
}

void wakeup_tsk(DWORD thread_input){
DWORD tempmsg;
int er;
	while(1) {
		er=rcv_msg(MBXID_WAKEUP_RECV,(void*)&tempmsg);
		if (er) {
			rot_rdq();	// let other tasks to run
			continue;
		}
		SYS_MSG* rmsg = (SYS_MSG*)tempmsg;
		int type = rmsg->id;
		void* taskID = (void*)rmsg->data_1;
		switch(type) {
		    case MOTO_TASK_WAKEUP:
		    	er=wup_tsk(taskID);
				if(0!=er){
					yl_uartPrintf("wake up %08x task fail error code %d!!\n",taskID,er);
				}
				else{
					Motor_printf("wake up %d task OK!!\n",taskID);
				}

				MotoSleepTaskID = 0;

				break;

		    default:
		    	yl_uartPrintf("default!!\n");
		    	break;
		}
	}
}

void motor_tsk(DWORD thread_input){
DWORD msg;
int er;

	while(1) {
		er=rcv_msg(MBXID_MOTOR_DRV,(void*)&msg);
		if (er) {
			rot_rdq();	// let other tasks to run
			continue;
		}

		switch(msg&MOTOMSG_MASK) {
			case MOTO_INIT:
			case MOTO_POWERON:
				break;
			case MOTO_POWEROFF:
				break;
			case MOTO_RESET:
				tq_MotoReset();
				break;
			case IRIS_OPEN:
				tq_MotoIris(IRIS_OPEN);
				break;
			case IRIS_CLOSE:
				tq_MotoIris(IRIS_CLOSE);
				break;
			case SHUTTER_OPEN:
				tq_MotoShutter(SHUTTER_OPEN);
				break;
			case SHUTTER_CLOSE:
				tq_MotoShutter(SHUTTER_CLOSE);
				break;
			case ZOOM_CW:
				tq_MotoZoom(ZOOM_CW);
				break;
			case ZOOM_WC:
				tq_MotoZoom(ZOOM_WC);
				break;
			case FOCUS_CCW:
				tq_MotoFocus(FOCUS_CCW);
				break;
			case FOCUS_CW:
				tq_MotoFocus(FOCUS_CW);
				break;
		}
	}
}

void uconMotor(int argc, char** argv){
	if (argc>=2) {
		DWORD msg = -1;
		if(strcmp(argv[1],"afccw")==0){
			msg = FOCUS_CCW;
		}
		else if(strcmp(argv[1],"afcw")==0){
			msg = FOCUS_CW;
		}
		else if(strcmp(argv[1],"zwc")==0){
			msg = ZOOM_WC;
		}
		else if(strcmp(argv[1],"zcw")==0){
			msg = ZOOM_CW;
		}

		if(msg>0)
			snd_msg(MBXID_MOTOR_DRV,MOTOMSG(msg));
	}
}


