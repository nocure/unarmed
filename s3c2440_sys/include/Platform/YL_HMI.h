/*
 * yl_UIMSG.h
 *
 *  Created on: 2008/12/19
 *      Author: ww
 */

#ifndef yl_hmi_H_
#define yl_hmi_H_

// ==== HMI message ids ==================================
// Range 0-32767
// ========================================================
#define HMI_POWER_STATE		1000
#define HMI_SD_CARD			1001

#define HMI_KEY_DOWN		1110
#define HMI_KEY_UP			1111
#define HMI_KEY_HOLD		1112
#define HMI_KEY_REPEAT		1113

#define HMI_TOUCH_X			1120
#define HMI_TOUCH_DOWN		1121
#define HMI_TOUCH_UP		1122
#define HMI_TOUCH_MOVE		1123

// HMI Control message
#define HMIC_REPEAT_DISABLE	5000

// ==== Message data ======================================
// Range 0-65535
// ========================================================
// ==== HMI_KEY_XXX ====
#define VKEY_UNKNOWN		0
// real hardware buttons
#define VKEY_1				1
#define VKEY_2				2
#define VKEY_3				3
#define VKEY_4				4
// Touch screen keys simlated
#define VKEY_5				5
#define VKEY_6				6
#define VKEY_7				7
#define VKEY_8				8
#define VKEY_9				9
#define VKEY_10				10

// ==== HMI_SD_CARD ====
#define SD_CARD_INSERTED	0
#define SD_CARD_REMOVED		1

// ==== HMI_POWER_STATE ====
#define POWER_0PERCENT		0
#define POWER_25PERCENT		25
#define POWER_50PERCENT		50
#define POWER_75PERCENT		75
#define POWER_100PERCENT	100


// ==== Message Macros =====================================
#define PACK_HMI_MSG(msg,dat) (0x80000000|(msg<<16)|(dat&0xffff))
#define UNPACK_HMI_MSG(packed) ((packed>>16)&0x7fff)
#define UNPACK_HMI_DAT(packed) ((packed)&0xffff)

int hmiSendMsg(DWORD msg, DWORD dat);
int hmiRecvMsg(DWORD *msg, DWORD *dat);


#endif /* yl_hmi_H_ */
