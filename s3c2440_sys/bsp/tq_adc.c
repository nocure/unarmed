#include "2440addr.h"
#include <platform/yl_type.h>
#include <platform/tq_bsp.h>
#include <platform/yl_hmi.h>
#include <platform/yl_errno.h>
#include "tq_hw_defs.h"
#include <platform/yl_oswrap.h>
#include <string.h>
#include <stdlib.h>

extern 	int yl_uartPrintf(const char *format, ...);
#if 0
	#define Touch_printf yl_uartPrintf
#else
	#define Touch_printf(...)
#endif

#define TS_CON_ECFLG (1<<15)//End of conversion flag
#define TS_CON_PRSCEN (1<<14)//A/D converter prescaler enable
#define TS_CON_PRSCVL (9<<6)//A/D converter prescaler value
#define TS_CON_ENABLE_START (1)//A/D conversion starts by enable


#define TS_TSC_UD_SEN (1<<8)//Detect Stylus Up or Down status
#define TS_TSC_YM_SEN (1<<7)//YM Switch Enable
#define TS_TSC_YP_SEN (1<<6)//YP Switch Enable
#define TS_TSC_XM_SEN (1<<5)//XM Switch Enable
#define TS_TSC_XP_SEN (1<<4)//XP Switch Enable
#define TS_TSC_PULL_UP (1<<3)//Pull-up Switch Enable
#define TS_TSC_AUTO_PST (1<<2)//Automatically sequencing conversion of X-Position and Y-Position
#define TS_TSC_XY_PST (3)//Manually measurement of X-Position or Y-Position

#define TS_DAT_UPDOWN (1<<15)
#define TS_DAT_AUTO_PST (1<<14)
#define TS_DAT_XY_PST (1<<12)
#define TS_DAT_XPDATA (0x3ff)
#define TS_DAT_YPDATA (0x3ff)


#define TS_DLY 20000

#define FLASH_SW_READY (1<<0)
#define FLASH_HW_READY (1<<1)
#define FLASH_ENABLE   (1<<2)
#define FLASH_TRIGGER  (1<<3)

static volatile int g_TSxdata, g_TSydata;
static int g_ADCurrentChannel = ADC_LOOPBACK;
static int g_Flashstatus = 0;


void tq_adcInit(void){
	//Touch screen power enable
	BIT_SET(rCLKCON,CLK_CON_ADC);

	BIT_CLR(rGPGCON,3);//trigger pin output config
	BIT_SET(rGPGCON,1);

	BIT_CLR(rGPGCON,3<<6);//flash on/off pin output config
	BIT_SET(rGPGCON,1<<6);
}


int tq_adcGetCurrentChannel(void){
	return g_ADCurrentChannel;
}


// get coordinate of touched point
int tq_adcGetTSData(int *x, int *y) {

	if(rADCCON & TS_CON_ECFLG){
		*x=(rADCDAT0&TS_DAT_XPDATA);
	 	*y=(rADCDAT1&TS_DAT_YPDATA);
		rADCDAT0 = 0;
	 	rADCDAT1 = 0;
	 	return SUCCESS;
	}
	return FAILED;
}


static void tq_adcTSDetect(void){
	rADCCON = TS_CON_PRSCEN|TS_CON_PRSCVL;
	rADCDLY=TS_DLY;
	rADCTSC = TS_TSC_YM_SEN|TS_TSC_YP_SEN|TS_TSC_XP_SEN|TS_TSC_AUTO_PST;
	rADCCON|=TS_CON_ENABLE_START;
}


static void tq_adcAIN0Detect(void){
	rADCCON = TS_CON_PRSCEN|TS_CON_PRSCVL;
	rADCDLY=TS_DLY;
	rADCTSC = TS_TSC_YP_SEN|TS_TSC_XP_SEN;
	rADCCON|=TS_CON_ENABLE_START;
}


void TS_StartScan(int channel){
	if(channel==ADC_TOUCHSCREEN){
		g_ADCurrentChannel = ADC_TOUCHSCREEN;
		tq_adcTSDetect();
	}
	else if(channel==ADC_AIN0){
		g_ADCurrentChannel = ADC_AIN0;
		tq_adcAIN0Detect();
	}
	else {
		//g_ADCurrentChannel = ADC_TOUCHSCREEN;
		//TSKeyDetect();
	}
}


extern void GDI_PutPixel(U32 x,U32 y, U32 c );
#define AVE_NUM 5
int tsindex =0;
int count[AVE_NUM]={0};
int countflag = 0;
int Tsaverage(int index){
	countflag++;
	int avg = 0;
	int i = 0;
	count[index] = g_TSxdata;
	if(countflag>=AVE_NUM){
		for(i=0;i<AVE_NUM;i++){
			avg = avg + count[i];
		}
		avg = avg / AVE_NUM;
		if(countflag>10000)
			countflag = 20;
	}
	else{
		for(i=0;i<index;i++){
			avg = avg + count[i];
		}
		avg = avg / (index+1);
	}
	return avg;
}


typedef struct _TSVolrange{
	int v_LB;//lowerbound
	int v_UB;//upperbound
	U32 pwm_timeH;
	U32 pwm_timeL;
	int id;
}TSADVolrange;
const TSADVolrange VolRange[]={
{-200,100,90,275,1},
{100,200,90,125,2},
{200,250,38,24,3},
{250,330,38,12,4},   //AD350 V270 //AD330 V240
};
#define VOL_INTERVAL 70
#define VOLRANGE_LENGTH (sizeof(VolRange)/sizeof(TSADVolrange))


int IsFlashReady(int AD_input){
	int rtn = FAILED;
	if(AD_input>=420){
		BIT_SET(g_Flashstatus,FLASH_SW_READY);
		rtn = SUCCESS;
	}

#if 0
	//flash hw ready
	if(yl_gpioGet(PORT_G|BIT_EINT6)){
		BIT_SET(g_Flashstatus,FLASH_HW_READY);
		rtn = SUCCESS;
	}
#endif
	return rtn;
}


int tq_FlashTrigger(int ms){
	int rtn = FAILED;
	if((ms<=0)||(ms>1000000))
		return rtn;

	//check flash enable
	if(!(g_Flashstatus&FLASH_ENABLE)){
		if(tq_gpioGet(PORT_G|BIT_EINT0)){
			BIT_SET(g_Flashstatus,FLASH_ENABLE);
		}
		else{
			//yl_uartPrintf("flash is not enable yet!!\n");
		}
	}

	if((g_Flashstatus&FLASH_SW_READY)||(g_Flashstatus&FLASH_HW_READY)){
#if 0
		yl_gpioSet(PORT_G|BIT_EINT3); //flash on
		yl_MotoDelay(ms);
		yl_gpioClr(PORT_G|BIT_EINT3); //flash off
		yl_uartPrintf("flash first time!!\n");
#else
		tq_gpioSet(PORT_G|BIT_EINT3); //flash on
		tq_MotoDelay(3);
		tq_gpioClr(PORT_G|BIT_EINT3); //flash off
		yl_uartPrintf("flash first time!!\n");
		dly_tsk(ms);
		tq_gpioSet(PORT_G|BIT_EINT3); //flash on
		tq_MotoDelay(3);
		tq_gpioClr(PORT_G|BIT_EINT3); //flash off
		yl_uartPrintf("flash second time!!\n");
#endif
		rtn = SUCCESS;
	}
	return rtn;
}


void tq_adcAIN0Handler(void){

	static int level = 0;
	static int turn_on_count = 0;
	if(g_ADCurrentChannel==ADC_AIN0){

		int AD_Voltage = Tsaverage(tsindex);
		if(tsindex >=AVE_NUM){
			Touch_printf("AD:%04d",g_TSxdata);
			Touch_printf("      average:%04d\n",AD_Voltage);
			tsindex = 0;
		}
		else{
			tsindex++;
		}
		//if(AD_Voltage>=420){
		if(SUCCESS==IsFlashReady(AD_Voltage)){
			if(tq_PwmGetStatus()&BIT_TIMER1){
				tq_PwmStop(BIT_TIMER1);
				level = 0;
				turn_on_count = 0;
				yl_uartPrintf("force pwm channel 1 off\n");
			}
			Touch_printf("AD:%04d\n",g_TSxdata);
			//BIT_SET(g_Flashstatus,FLASH_SW_READY);//ready to trigger
		}
		else{
			int i=0;
			AD_Voltage = g_TSxdata;
			for(i=0;i<VOLRANGE_LENGTH;i++){
				if(tq_PwmGetStatus()&BIT_TIMER1){
					if(((AD_Voltage-VOL_INTERVAL)>=VolRange[i].v_LB)&&((AD_Voltage-VOL_INTERVAL)<VolRange[i].v_UB)){
						if(level != VolRange[i].id){
							tq_PwmSet(BIT_TIMER1,VolRange[i].pwm_timeH,VolRange[i].pwm_timeL);
							level = VolRange[i].id;
							Touch_printf("Running Pwm set H:%d L:%d\n",VolRange[i].pwm_timeH,VolRange[i].pwm_timeL);
						}
						break;
					}
				}
				else{
					if((AD_Voltage>=VolRange[i].v_LB)&&(AD_Voltage<VolRange[i].v_UB)){
						if(turn_on_count>=3){
						tq_PwmStart(BIT_TIMER1);
						tq_PwmSet(BIT_TIMER1,VolRange[i].pwm_timeH,VolRange[i].pwm_timeL);
						level = VolRange[i].id;
						BIT_CLR(g_Flashstatus,FLASH_SW_READY|FLASH_HW_READY);//charger busy
						Touch_printf("Turn On Pwm set H:%d L:%d\n",VolRange[i].pwm_timeH,VolRange[i].pwm_timeL);
						turn_on_count = 0;
						}
						else{
							turn_on_count++;
						}
						break;
					}
				}

			}
		}
	}
}


void uconTS(int argc, char** argv){
	if (argc>=2) {
		if(strcmp(argv[1],"trigger")==0){
			int dtime = atoi(argv[2]);
			tq_gpioSet(PORT_G|BIT_EINT0);//trigger enable
			tq_FlashTrigger(dtime);
			tq_gpioClr(PORT_G|BIT_EINT0);//trigger disable
		}
	}
}

