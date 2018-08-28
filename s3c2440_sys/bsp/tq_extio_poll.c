#include <sct.h>
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/YL_HMI.h>
#include <platform/yl_errno.h>
#include <../hmi/TouchScreen.h>


#define TS_KEY_DELAY 13
#define AIN0_DELAY 13
#define TS_POINTER_DELAY 7

#if !defined(__LUSIMUS__)
static DWORD g_LastPortF=KEY_1|KEY_2|KEY_3|KEY_4;	// default up state: KEY UP
static DWORD g_LastPortG=SD_CARD_DET;	// let card removed as default state
const static DWORD g_HWKeyMask[]={
		KEY_1,KEY_2,KEY_3,KEY_4
};
#define NUM_HWKEYS (sizeof(g_HWKeyMask)/sizeof(DWORD))
#endif
static DWORD g_LastTSKEY=VKEY_UNKNOWN;//Touchscreen last key record

static int adcTSKeyDelay=0;
static int adcAIN0Delay=0;
static int adcTSPointerDelay=0;

static DWORD vkeyState=0;
static DWORD vkeyLastState=0;

#define NUM_VKEYS 10
#define VKEY_REPEAT_HOLDTIME 15
static DWORD vkey_holdtime[NUM_VKEYS]={0,0,0,0,0,0,0,0,0,0};


static void vkeySetState(int mask,int up) {
	if (up) {
		vkeyState&=~mask;
	}
	else {
		vkeyState|=mask;
	}
}


static void vkeyEventDispatch(void) {
DWORD keybit;
DWORD diffbit;
int i;
DWORD msg;

	keybit=1;
	diffbit=vkeyState^vkeyLastState;
	for (i=0;i<NUM_VKEYS;i++) {
		if (diffbit&keybit) {	// is it different state
			if (vkeyState&keybit) {		// the key has just been pressed
				msg=HMI_KEY_DOWN;
				vkey_holdtime[i]=1;
			}
			else {	// the key has been released
				msg=HMI_KEY_UP;
				vkey_holdtime[i]=0;
			}

		}
		else if (vkeyState&keybit) {		// the key is being held down
			msg=vkey_holdtime[i];
			if (msg==VKEY_REPEAT_HOLDTIME) {
				vkey_holdtime[i]++;
				msg=HMI_KEY_HOLD;
			}
			else if (msg>VKEY_REPEAT_HOLDTIME) {
				msg=HMI_KEY_REPEAT;
			}
			else {
				vkey_holdtime[i]++;
				msg=0;
			}
		}
		else msg=0;
		if (msg) hmiSendMsg(msg,VKEY_1+i);
		keybit<<=1;
	}

	vkeyLastState=vkeyState;
}


void cycEXTIOPollHandler(unsigned long thread_input) {
#if !defined(__LUSIMUS__)
DWORD stat,mask,keybit;
int i;


	// GPF detect
	stat=tq_gpioGet(PORT_F|KEY_1|KEY_2|KEY_3|KEY_4);	// get GPF 0,1,2,4 state
	if (g_LastPortF!=stat) {
		keybit=1;
		for (i=0;i<NUM_HWKEYS;i++) {
			mask=g_HWKeyMask[i];
			vkeySetState(keybit,stat&mask);
			keybit<<=1;
		}
		g_LastPortF=stat;
	}

	// GPG detect
	stat=tq_gpioGet(PORT_G|SD_CARD_DET);	//GPG->06, SD Card detect, 0->card insert, 1->card remove
	if (g_LastPortG!=stat) {
		if ((stat&SD_CARD_DET)&&!(g_LastPortG&SD_CARD_DET)) { // SD_CARD_REMOVED
			hmiSendMsg(HMI_SD_CARD,SD_CARD_REMOVED);
		}
		else if (!(stat&SD_CARD_DET)&&(g_LastPortG&SD_CARD_DET)) { // SD_CARD_INSERTED
			hmiSendMsg(HMI_SD_CARD,SD_CARD_INSERTED);
		}
		g_LastPortG=stat;
	}
#endif

	vkeyEventDispatch();

}


// ================================================================================

// ============
void cycADPollHandler(unsigned long thread_input) {
int ch;
int x,y;

	ch = tq_adcGetCurrentChannel();
	if(ch == ADC_TOUCHSCREEN) {
	register int rtn;
	register DWORD tskey;
		rtn=tq_adcGetTSData(&x,&y); // get the AD result
		if(rtn==SUCCESS) {
			rtn=tsPushPointToFifo(x,y);	// return failed if the x,y is not valid, eg. the panel's not touched
			if (!rtn) tsGetScreenXY(&x,&y);
			if (!adcTSPointerDelay) {
				adcTSPointerDelay=TS_POINTER_DELAY;
				if(rtn==SUCCESS) {
					tsGenHMIEvent(x,y);
				}
				else {
					tsGenHMIEvent(-1,-1);
				}
			}
			else adcTSPointerDelay--;
		}

		if (!adcTSKeyDelay) { // is it time to check key ?
			adcTSKeyDelay=TS_KEY_DELAY;
			// check only the screen is touched and point is averaged
			if (rtn==SUCCESS) {
				tskey = tsIsInsideKeyRange(x,y); 		// check if point is valid
			}
			else tskey=VKEY_UNKNOWN;
			if (g_LastTSKEY!=tskey) {
				if (g_LastTSKEY) vkeySetState(1<<(VKEY_5+g_LastTSKEY-2),1);	// up
				if (tskey) vkeySetState(1<<(VKEY_5+tskey-2),0);		// down
				g_LastTSKEY=tskey;
			}
		}
		else adcTSKeyDelay--;

		// switch to AIN0
		TS_StartScan(ADC_AIN0);
	}
	else if(ch == ADC_AIN0){
		if (!adcAIN0Delay) {
			adcAIN0Delay=AIN0_DELAY;
			tq_adcAIN0Handler();
		}
		else adcAIN0Delay--;

		// switch to touchscreen
		TS_StartScan(ADC_TOUCHSCREEN);
	}
	else {
		// first time? switch to touchscreen
		TS_StartScan(ADC_TOUCHSCREEN);
	}

}


