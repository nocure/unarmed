/*
 * YL_OSWrap_cyc.c
 *
 *  Created on: 2012/8/15
 *      Author: User
 *
 *  ww 2012-0817 fixed error codes
 *
 */
#include "platform/yl_sys.h"
#include "sct.h"
#include <ucos_ii.h>
#include "YL_OSWrap_priv.h"


static void yl_cyclic_handler(void* ptmr, void *cyc_hdr) {
	((FUNCENTRY)cyc_hdr)(0);
}

//=====================================================================
// Timer
//=====================================================================
// === def_cyc ===
int yl_def_cyc(void** ppid, FUNCENTRY cychdr, DWORD cyctim, DWORD cycact) {
void *ptr;
INT8U err;
int ret;
#define TIM_MEM_SIZE	( sizeof(CYCLIC_EXTRA) )

	cyctim/=(OS_TICKS_PER_SEC/OS_TMR_CFG_TICKS_PER_SEC);		// claclulate the correct rate of timer service

	if (cyctim<=0) return OSWRAP_OPTION_ERROR;	// not enough timer resolution

	ptr=OSTmrCreate(
		0,						// initial delay
		cyctim,					// amount of time it will take before the timer expires
		OS_TMR_OPT_PERIODIC,	// cyclic is a periodic timer
		yl_cyclic_handler,		// timer callback function
		cychdr,					// callback arg, the actual cychdr is passed.
		(INT8U*)"tmr",			// name
		&err					// err code
	);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_TMR_INVALID_DLY OS_ERR_TMR_INVALID_PERIOD, OS_ERR_TMR_INVALID_OPT OS_ERR_TMR_NAME_TOO_LONG
	// will happen: OS_ERR_TMR_ISR OS_ERR_TMR_NON_AVAIL
	if (err) goto err0;

	if (cycact==TCY_ON) {
		OSTmrStart(ptr,&err);
		ret=yl_oswrap_errcode(err);
		// will NOT happen: OS_ERR_TMR_INVALID OS_ERR_TMR_INVALID_TYPE OS_ERR_TMR_INACTIVE OS_ERR_TMR_ISR
		// actually, no err will happen when getting this far
		if (ret) goto err1;
	}

	*ppid=ptr;
	return 0;

err1:
	OSTmrDel(ptr,&err);

err0:
	*ppid=0;
	return ret;
}


// === del_cyc ===
inline int yl_del_cyc(void* ptr) {
INT8U err;
int ret;

	OSTmrDel(ptr,&err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen:
	// will happen: OS_ERR_TMR_INVALID OS_ERR_TMR_INVALID_TYPE OS_ERR_TMR_ISR OS_ERR_TMR_INACTIVE
	return ret;
}


// === act_cyc ===
int yl_act_cyc(void *ptr, DWORD cycact) {
INT8U err;
int ret;

	if (cycact==TCY_OFF) {
		OSTmrStop(ptr,OS_TMR_OPT_NONE,0,&err);
		ret=yl_oswrap_errcode(err);
		// will NOT happen: OS_ERR_TMR_INVALID_OPT
		// will happen: OS_ERR_TMR_INVALID OS_ERR_TMR_INVALID_TYPE OS_ERR_TMR_ISR OS_ERR_TMR_STOPPED OS_ERR_TMR_INACTIVE
	}
	else {
		if (cycact&TCY_INI) {
			// uCOS timer always start with counter initialized
			// no need to TCY_INI
		}
		OSTmrStart(ptr,&err);
		ret=yl_oswrap_errcode(err);
		// will NOT happen:
		// will happen: OS_ERR_TMR_INVALID OS_ERR_TMR_INVALID_TYPE OS_ERR_TMR_ISR OS_ERR_TMR_INACTIVE
	}
	return ret;
}


// === chg_cyc ===
inline int yl_chg_cyc(void *ptr, DWORD cyctim) {
	// not supported
	return OSWRAP_FEATURE_NOT_ENABLED;
}


// === get_tim ===
inline DWORD yl_get_tim(void) {
	return OSTimeGet();
}


