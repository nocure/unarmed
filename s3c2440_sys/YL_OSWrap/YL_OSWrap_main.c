/*
 * YL_OSWrap_err.c
 *
 *  Created on: 2012/8/16
 *      Author: User
 */
#include "platform/yl_sys.h"
#include "sct.h"
#include <ucos_ii.h>


// ==============================================================================================

void appPreInit(void* unused_memory);

// ==============================================================================================

int YL_OSWrap_LUT[]={
		0,											//   0u  OS_ERR_NONE

		OSWRAP_PTR_ERROR,							//   1u  OS_ERR_EVENT_TYPE
		OSWRAP_CALLER_ERROR,						//   2u  OS_ERR_PEND_ISR
		OSWRAP_NOT_HANDLED,							//   3u  OS_ERR_POST_NULL_PTR
		OSWRAP_PTR_ERROR,							//   4u  OS_ERR_PEVENT_NULL
		OSWRAP_NOT_HANDLED,							//   5u  OS_ERR_POST_ISR
		OSWRAP_NOT_HANDLED,							//   6u  OS_ERR_QUERY_ISR
		OSWRAP_NOT_HANDLED,							//   7u  OS_ERR_INVALID_OPT
		OSWRAP_NOT_HANDLED,							//   8u  OS_ERR_ID_INVALID
		OSWRAP_NOT_HANDLED,							//   9u  OS_ERR_PDATA_NULL

		OSWRAP_TIME_OUT,							//  10u  OS_ERR_TIMEOUT
		OSWRAP_NOT_HANDLED,							//  11u  OS_ERR_EVENT_NAME_TOO_LONG
		OSWRAP_OPTION_ERROR,						//  12u  OS_ERR_PNAME_NULL
		OSWRAP_WAIT_ERROR,							//  13u  OS_ERR_PEND_LOCKED
		OSWRAP_NOT_HANDLED,							//  14u  OS_ERR_PEND_ABORT
		OSWRAP_CALLER_ERROR,						//  15u  OS_ERR_DEL_ISR
		OSWRAP_CALLER_ERROR,						//  16u  OS_ERR_CREATE_ISR
		OSWRAP_CALLER_ERROR,						//  17u  OS_ERR_NAME_GET_ISR
		OSWRAP_CALLER_ERROR,						//  18u  OS_ERR_NAME_SET_ISR
		OSWRAP_NOT_HANDLED,							//  19u  OS_ERR_ILLEGAL_CREATE_RUN_TIME

		OSWRAP_NOT_HANDLED,							//  20u  OS_ERR_MBOX_FULL

		-1,											//  21u  Not used
		-1,											//  22u  Not used
		-1,											//  23u  Not used
		-1,											//  24u  Not used
		-1,											//  25u  Not used
		-1,											//  26u  Not used
		-1,											//  27u  Not used
		-1,											//  28u  Not used
		-1,											//  29u  Not used

		OSWRAP_NOT_AVAILABLE,						//  30u  OS_ERR_Q_FULL
		OSWRAP_TIME_OUT,							//  31u  OS_ERR_Q_EMPTY

		-1,											//  32u  Not used
		-1,											//  33u  Not used
		-1,											//  34u  Not used
		-1,											//  35u  Not used
		-1,											//  36u  Not used
		-1,											//  37u  Not used
		-1,											//  38u  Not used
		-1,											//  39u  Not used

		OSWRAP_PRIORITY_ERROR,						//  40u  OS_ERR_PRIO_EXIST
		OSWRAP_NOT_HANDLED,							//  41u  OS_ERR_PRIO
		OSWRAP_PRIORITY_ERROR,						//  42u  OS_ERR_PRIO_INVALID

		-1,											//  43u  Not used
		-1,											//  44u  Not used
		-1,											//  45u  Not used
		-1,											//  46u  Not used
		-1,											//  47u  Not used
		-1,											//  48u  Not used
		-1,											//  49u  Not used

		OSWRAP_NOT_HANDLED,							//  50u  OS_ERR_SCHED_LOCKED
		OSWRAP_NOT_HANDLED,							//  51u  OS_ERR_SEM_OVF

		-1,											//  52u  Not used
		-1,											//  53u  Not used
		-1,											//  54u  Not used
		-1,											//  55u  Not used
		-1,											//  56u  Not used
		-1,											//  57u  Not used
		-1,											//  58u  Not used
		-1,											//  59u  Not used

		OSWRAP_CALLER_ERROR,						//  60u  OS_ERR_TASK_CREATE_ISR
		OSWRAP_NOT_AVAILABLE,						//  61u  OS_ERR_TASK_DEL
		OSWRAP_NOT_AVAILABLE,						//  62u  OS_ERR_TASK_DEL_IDLE			not happen
		OSWRAP_NOT_HANDLED,							//  63u  OS_ERR_TASK_DEL_REQ
		OSWRAP_CALLER_ERROR,						//  64u  OS_ERR_TASK_DEL_ISR
		OSWRAP_NOT_HANDLED,							//  65u  OS_ERR_TASK_NAME_TOO_LONG
		OSWRAP_NOT_AVAILABLE,						//  66u  OS_ERR_TASK_NO_MORE_TCB
		OSWRAP_NOT_AVAILABLE,						//  67u  OS_ERR_TASK_NOT_EXIST
		OSWRAP_NOT_HANDLED,							//  68u  OS_ERR_TASK_NOT_SUSPENDED
		OSWRAP_NOT_HANDLED,							//  69u  OS_ERR_TASK_OPT
		OSWRAP_NOT_HANDLED,							//  70u  OS_ERR_TASK_RESUME_PRIO
		OSWRAP_NOT_HANDLED,							//  71u  OS_ERR_TASK_SUSPEND_IDLE
		OSWRAP_NOT_HANDLED,							//  72u  OS_ERR_TASK_SUSPEND_PRIO
		OSWRAP_NOT_HANDLED,							//  73u  OS_ERR_TASK_WAITING

		-1,											//  74u  Not used
		-1,											//  75u  Not used
		-1,											//  76u  Not used
		-1,											//  77u  Not used
		-1,											//  78u  Not used
		-1,											//  79u  Not used

		OSWRAP_NOT_DONE,							//  80u  OS_ERR_TIME_NOT_DLY
		OSWRAP_NOT_HANDLED,							//  81u  OS_ERR_TIME_INVALID_MINUTES
		OSWRAP_NOT_HANDLED,							//  82u  OS_ERR_TIME_INVALID_SECONDS
		OSWRAP_NOT_HANDLED,							//  83u  OS_ERR_TIME_INVALID_MS
		OSWRAP_NOT_HANDLED,							//  84u  OS_ERR_TIME_ZERO_DLY
		OSWRAP_NOT_HANDLED,							//  85u  OS_ERR_TIME_DLY_ISR

		-1,											//  86u  Not used
		-1,											//  87u  Not used
		-1,											//  88u  Not used
		-1,											//  89u  Not used

		OSWRAP_NOT_HANDLED,							//  90u  OS_ERR_MEM_INVALID_PART
		OSWRAP_NOT_HANDLED,							//  91u  OS_ERR_MEM_INVALID_BLKS
		OSWRAP_NOT_HANDLED,							//  92u  OS_ERR_MEM_INVALID_SIZE
		OSWRAP_NOT_HANDLED,							//  93u  OS_ERR_MEM_NO_FREE_BLKS
		OSWRAP_NOT_HANDLED,							//  94u  OS_ERR_MEM_FULL
		OSWRAP_NOT_HANDLED,							//  95u  OS_ERR_MEM_INVALID_PBLK
		OSWRAP_NOT_HANDLED,							//  96u  OS_ERR_MEM_INVALID_PMEM
		OSWRAP_NOT_HANDLED,							//  97u  OS_ERR_MEM_INVALID_PDATA
		OSWRAP_NOT_HANDLED,							//  98u  OS_ERR_MEM_INVALID_ADDR
		OSWRAP_NOT_HANDLED,							//  99u  OS_ERR_MEM_NAME_TOO_LONG

		OSWRAP_NOT_HANDLED,							// 100u  OS_ERR_NOT_MUTEX_OWNER

		-1,											// 101u  Not used
		-1,											// 102u  Not used
		-1,											// 103u  Not used
		-1,											// 104u  Not used
		-1,											// 105u  Not used
		-1,											// 106u  Not used
		-1,											// 107u  Not used
		-1,											// 108u  Not used
		-1,											// 109u  Not used

		OSWRAP_PTR_ERROR,							// 110u  OS_ERR_FLAG_INVALID_PGRP
		OSWRAP_NOT_HANDLED,							// 111u  OS_ERR_FLAG_WAIT_TYPE
		OSWRAP_NOT_HANDLED,							// 112u  OS_ERR_FLAG_NOT_RDY
		OSWRAP_NOT_HANDLED,							// 113u  OS_ERR_FLAG_INVALID_OPT
		OSWRAP_NO_MEMORY,							// 114u  OS_ERR_FLAG_GRP_DEPLETED
		OSWRAP_NOT_HANDLED,							// 115u  OS_ERR_FLAG_NAME_TOO_LONG

		-1,											// 116u  Not used
		-1,											// 117u  Not used
		-1,											// 118u  Not used
		-1,											// 119u  Not used

		OSWRAP_NOT_HANDLED,							// 120u  OS_ERR_PIP_LOWER

		-1,											// 121u  Not used
		-1,											// 122u  Not used
		-1,											// 123u  Not used
		-1,											// 124u  Not used
		-1,											// 125u  Not used
		-1,											// 126u  Not used
		-1,											// 127u  Not used
		-1,											// 128u  Not used
		-1,											// 129u  Not used

		OSWRAP_NOT_HANDLED,							// 130u  OS_ERR_TMR_INVALID_DLY
		OSWRAP_NOT_HANDLED,							// 131u  OS_ERR_TMR_INVALID_PERIOD
		OSWRAP_NOT_HANDLED,							// 132u  OS_ERR_TMR_INVALID_OPT
		OSWRAP_NOT_HANDLED,							// 133u  OS_ERR_TMR_INVALID_NAME
		OSWRAP_NOT_AVAILABLE,						// 134u  OS_ERR_TMR_NON_AVAIL
		OSWRAP_PTR_ERROR,							// 135u  OS_ERR_TMR_INACTIVE
		OSWRAP_NOT_HANDLED,							// 136u  OS_ERR_TMR_INVALID_DEST
		OSWRAP_PTR_ERROR,							// 137u  OS_ERR_TMR_INVALID_TYPE
		OSWRAP_PTR_ERROR,							// 138u  OS_ERR_TMR_INVALID
		OSWRAP_CALLER_ERROR,						// 139u  OS_ERR_TMR_ISR
		OSWRAP_NOT_HANDLED,							// 140u  OS_ERR_TMR_NAME_TOO_LONG
		OSWRAP_NOT_HANDLED,							// 141u  OS_ERR_TMR_INVALID_STATE
		0,											// 142u  OS_ERR_TMR_STOPPED
		OSWRAP_NOT_HANDLED,							// 143u  OS_ERR_TMR_NO_CALLBACK
};

// ==============================================================================================

int yl_oswrap_errcode(INT8U err) {
	return (YL_OSWrap_LUT[err]);
}


// ==== YL OS wrapper ====
static void* oswrap_internal_init(void* pointer) {
	return pointer;
}

// ==============================================================================================
// ==============================================================================================
extern const TSK_DEF_T info_tsk[];	///< tasks
extern const FLG_DEF_T info_evf[];	///< event flag
extern const SEM_DEF_T info_sem[];	///< Semaphore
extern const MBX_DEF_T info_mbx[];	///< Message Buffer
extern const MTX_DEF_T info_mtx[];	///< mutex
#if 0
extern const MBF_DEF_T info_mbf[];	///< event flags
extern const MPF_DEF_T info_mpf[];	///< block pool
extern const MPL_DEF_T info_mpl[];	///< Byte pool
#endif
extern const ALM_DEF_T info_alm[];	///< Alarm
extern const CYC_DEF_T info_cyc[];	///< Cyclic


void yl_OSWrapInit(void *first_unused_memory) {
int i;

	first_unused_memory=oswrap_internal_init(first_unused_memory);
	appPreInit(first_unused_memory);

	// brief create uItron objects defined in sct.c
//	for (i=0;info_mpf[i].id!=0;i++) er|= cre_mpf(info_mpf[i].id,(T_CMPF*)&info_mpf[i].mpfdef);
//	for (i=0;info_mpl[i].id!=0;i++) er|= cre_mpl(info_mpl[i].id,(T_CMPL*)&info_mpl[i].mpldef);
	for (i=0;info_evf[i].id!=0;i++) if (cre_flg(info_evf[i].id,info_evf[i].iflgptn)) goto err_flg;
	for (i=0;info_sem[i].id!=0;i++) if (cre_sem(info_sem[i].id,info_sem[i].isemcnt,info_sem[i].maxsem)) goto err_sem;
	for (i=0;info_mbx[i].id!=0;i++) if (cre_mbx(info_mbx[i].id,info_mbx[i].bufcnt)) goto err_mbx;
	for (i=0;info_mtx[i].id!=0;i++) if (cre_mtx(info_mtx[i].id)) goto err_mtx;
//	for (i=0;info_mbf[i].id!=0;i++) er|= cre_mbf(info_mbf[i].id,(T_CMBF*)&info_mbf[i].mbfdef);
//	for (i=0;info_alm[i].id!=0;i++) er|= def_alm(info_alm[i].id,(T_DALM*)&info_alm[i].almdef);
	for (i=0;info_cyc[i].id!=0;i++) if (def_cyc(info_cyc[i].id,info_cyc[i].cychdr,info_cyc[i].cyctim,info_cyc[i].cycact)) goto err_cyc;
	for (i=0;info_tsk[i].id!=0;i++) if (cre_tsk(info_tsk[i].id,info_tsk[i].task,info_tsk[i].stksz,info_tsk[i].atr_pri,0)) goto err_tsk;
//	sta_tsk(TSK_INIT, 0);

	return;

err_flg:
	while(1);

err_sem:
	while(1);

err_mbx:
	while(1);

err_mtx:
	while(1);

err_cyc:
	while(1);

err_tsk:
	while(1);


}
