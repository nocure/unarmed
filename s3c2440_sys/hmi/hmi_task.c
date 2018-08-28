/*
 * hmi_task.c
 * 		Human machine interface task
 * 		- The central message dispatch -
 *
 *  Created on: 2008/12/19
 *      Author: ww
 */
#include <sct.h>
#include "platform/YL_SYS.h"
#include <platform/YL_ERRNO.h>
#include <platform/YL_HMI.h>
#include "TouchScreen.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
//#include <minigui/gdi.h>
//#include <minigui/window.h>

#define HMIOP_REPEAT_DISABLE	1


int guiSendMsg(DWORD msg, DWORD dat);

static DWORD OPSwitch;


int hmiSendMsg(DWORD msg, DWORD dat) {
int er;
	er=snd_msg(MBXID_HMI,(void*)PACK_HMI_MSG(msg,dat));
	if (er!=0) return FAILED;
	return SUCCESS;
}


int hmiRecvMsg(DWORD *msg, DWORD *dat) {
DWORD packed;
int er;

	er=rcv_msg(MBXID_HMI,(void*)&packed);
	if (er!=0) return FAILED;

	*msg=UNPACK_HMI_MSG(packed);
	*dat=UNPACK_HMI_DAT(packed);;

	return SUCCESS;
}


void hmi_tsk(ULONG thread_input) {
int mousex;
DWORD msg;
DWORD dat;
int ret;

	mousex=0;
	OPSwitch=0;
	tsPrepareKeyScreen();

	while(1) {
		ret=hmiRecvMsg(&msg,&dat);
		if (ret) {
			rot_rdq();	// let other tasks to run
			continue;
		}

		switch(msg) {
		case HMIC_REPEAT_DISABLE:
			if (dat) BIT_SET(OPSwitch,HMIOP_REPEAT_DISABLE);
			else BIT_CLR(OPSwitch,HMIOP_REPEAT_DISABLE);
			break;

		case HMI_KEY_DOWN:
			printf("HMI_KEY_DOWN %d\n",dat);
			if (dat>=VKEY_5) tsDrawkey(dat-VKEY_5,TSKEY_DOWN);
			guiSendMsg(HMI_KEY_DOWN,dat);
			break;

		case HMI_KEY_UP:
			printf("HMI_KEY_UP %d\n",dat);
			if (dat>=VKEY_5) tsDrawkey(dat-VKEY_5,TSKEY_UP);
			guiSendMsg(HMI_KEY_UP,dat);
			break;

		case HMI_KEY_HOLD:
			printf("HMI_KEY_HOLD %d\n",dat);
			guiSendMsg(HMI_KEY_HOLD,dat);
			break;

		case HMI_KEY_REPEAT:
			if (!BIT_VAL(OPSwitch,HMIOP_REPEAT_DISABLE)) {
				guiSendMsg(HMI_KEY_REPEAT,dat);
				printf("HMI_KEY_REPEAT %d\n",dat);
			}
			break;


		case HMI_TOUCH_X:
			mousex=dat;
			break;

		case HMI_TOUCH_DOWN:
			SetCursorPos(mousex,dat);
			break;

		case HMI_TOUCH_UP:
			break;

		case HMI_TOUCH_MOVE:
			SetCursorPos(mousex,dat);
			break;


		case HMI_SD_CARD:
			if (dat==SD_CARD_REMOVED) {
				printf("SD_CARD_REMOVED\n");
//				ret=yl_dmUnmount(1);		// unmount b:
//				if (ret) dbgout("MMCSD_DRV unmount error\n");
			}
			else {
				printf("SD_CARD_INSERTED\n");
//				ret=yl_dmMount(1);		// mount b:
//				if (ret) dbgout("MMCSD_DRV mount error\n");
			}
		}
	}

}
