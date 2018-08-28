/*
 * YL_OSWrap_flg.c
 *
 *  Created on: 2012/8/15
 *      Author: Jon
 */
#include "platform/yl_sys.h"
#include "sct.h"
#include <ucos_ii.h>
#include "YL_OSWrap_priv.h"

//=====================================================================
// FLAG
//=====================================================================
// === cre_flg ===
int yl_cre_flg(void** ppid, DWORD iflgptn) {
INT8U err;
int ret;

	*ppid = OSFlagCreate(iflgptn, &err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen:
	// will happen: OS_ERR_CREATE_ISR OS_ERR_FLAG_GRP_DEPLETED

	return ret;
}


// === del_flg ===
inline int yl_del_flg(void* ptr) {
INT8U err;
int ret;

	OSFlagDel(ptr, OS_DEL_ALWAYS, &err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_INVALID_OPT OS_ERR_TASK_WAITING
	// will happen: OS_ERR_DEL_ISR OS_ERR_FLAG_INVALID_PGRP OS_ERR_EVENT_TYPE

	return ret;
}



// === set_flg ===
inline int yl_set_flg(void* ptr, DWORD ptn) {
INT8U err;
int ret;

	OSFlagPost(ptr, ptn, OS_FLAG_SET, &err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_FLAG_INVALID_OPT
	// will happen: OS_ERR_FLAG_INVALID_PGRP OS_ERR_EVENT_TYPE
	return ret;
}


// === clr_flg ===
inline int yl_clr_flg(void* ptr, DWORD ptn) {
INT8U err;
int ret;

	OSFlagPost(ptr, ptn, OS_FLAG_CLR, &err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_FLAG_INVALID_OPT
	// will happen: OS_ERR_FLAG_INVALID_PGRP OS_ERR_EVENT_TYPE
	return ret;

}


// === wai_flg ===
inline int yl_wai_flg(DWORD *pflgptn, void* ptr, DWORD waiptn, DWORD wfmode, DWORD wait) {
INT8U err,opt;
int ret=0;

// ww 2012-0817 modified --> shorter code
	if (BIT_VAL(wfmode,TWF_ANDW)){
		opt=OS_FLAG_WAIT_SET_AND;
	}
	else {
		opt=OS_FLAG_WAIT_SET_OR;
	}
	if (BIT_VAL(wfmode,TWF_CLR)){
		BIT_SET(opt,OS_FLAG_CONSUME);
	}

#if 0
	if (wfmode & 0x02){
		if (wfmode & 0x01)
			opt |= (OS_FLAG_CONSUME | OS_FLAG_WAIT_SET_AND) ;
		else
			opt |= OS_FLAG_WAIT_SET_AND;
	}
	else if (wfmode & 0x01){
		opt |= (OS_FLAG_CONSUME | OS_FLAG_WAIT_SET_OR);
	}
	else{
		opt |= OS_FLAG_WAIT_SET_OR;
	}

	if (!ptr) return OSWRAP_PTR_ERROR;
#endif

// ww 2012-0817 modified <--

	if (wait == 0){
		*pflgptn = OSFlagAccept(ptr, waiptn, opt, &err);
		ret=yl_oswrap_errcode(err);
	}
	else {
		while (1) {
			*pflgptn = OSFlagPend(ptr, waiptn, opt , 0, &err);
			if (!err) break;
			else if (err==OS_ERR_TIMEOUT) {
				if (is_released()) {	// is released ?
					ret=OSWRAP_WAIT_ABORTED;
					break;
				}
				else if (wait!=-1) {	// if not wait forever then it's timeout condition, exit loop
					ret=yl_oswrap_errcode(err);
					break;
				}
			}
		}
	}

// ww 2012-0817 removed --> not necessary
//	*pflgptn &= waiptn;
// ww 2012-0817 removed <--
	return ret;


}



