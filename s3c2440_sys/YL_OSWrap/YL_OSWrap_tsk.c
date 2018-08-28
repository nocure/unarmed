/*
 * YL_OSWrap.c
 *
 *  Created on: 2012/8/13
 *      Author: User
 *
 *  ww 2012-0817 fixed error codes
 *
 */
#include "platform/yl_sys.h"
#include "sct.h"
#include <ucos_ii.h>
#include "YL_OSWrap_priv.h"


typedef struct {
	DWORD magic;
	int wake_up_call;
	DWORD attr;
	void* tskvar[NUM_TSKVARS];

	FUNCENTRY task;
	int stksz;
	DWORD atr_pri;
	int id;
	void* ptos;
	void* pbos;
} THREAD_EXTRA;

#define THREAD_EXTRA_SIZE ALIGN16(sizeof(THREAD_EXTRA))
#define THREAD_EXTRA_MAGIC	('t'|('s'<<8)|('k'<<16)|('\0'<<24))

#define EXTRA_STATE_DORMANT			0
#define EXTRA_STATE_STARTED			(1<<0)
#define TASK_SLEEPING				(1<<1)		// put sleep by slp_tsk
#define TASK_WOKEUP					(1<<2)		// woke up by wup_tsk
#define TASK_RELEASE				(1<<3)


// ==============================================================================================

static void* global_tskvar[NUM_TSKVARS];

// ==============================================================================================


//=====================================================================
// TASK
//=====================================================================


//---------------------------------------------------------------------
// === cre_tsk ===
int yl_cre_tsk(void** ppid, FUNCENTRY task, int stksz, DWORD atr_pri, void* private) {
int pinc;
void *pstk,*ptr;
INT8U err,id;
int ret;
THREAD_EXTRA *extra;

#define TSK_MEM_SIZE	( THREAD_EXTRA_SIZE+stksz )

	pinc=TSK_MEM_SIZE;

	// 2018-0823 no virtual console here
#if 0
	// Jon added. 2011-1129.--->
	// Add extra uart buffer.
	pinc += vconVarSize();
	// Jon added. <---
#endif

#if 0
	if (BIT_VAL(atr_pri,TSK_NEEDLPI)) {	// allocate reent space in exinf
		pinc+=lpiVarSize();
	}
#endif
	if (BIT_VAL(atr_pri,TSK_REVMEM)) {
		ptr=mallocr(pinc);
	}
	else {
		ptr=malloc(pinc);
	}
	if (!ptr) {
		ret=OSWRAP_NO_MEMORY;
		goto err0;
	}
	memset(ptr,0,pinc);
	pinc=0;

	MEMDEF(extra,THREAD_EXTRA_SIZE);
	MEMDEF(pstk,stksz);

	// 2018-0823 no virtual console here
#if 0
	// Jon added. 2011-1228. --->
	MEMDEF(extra->tskvar[TSK_CONSOLE],vconVarSize());
	// Jon added. <---
#endif
#if 0
	if (BIT_VAL(atr_pri,TSK_NEEDLPI)) {	// allocate reent space in exinf
		MEMDEF(extra->tskvar[TSK_LPI],lpiVarSize());
	}
#endif

	extra->magic=THREAD_EXTRA_MAGIC;
	extra->task=task;
	extra->stksz=stksz;
	extra->atr_pri=atr_pri;

	id=(atr_pri&0xffff);
	extra->id=id;

	stksz/=sizeof(OS_STK);
	extra->ptos=(OS_STK*)(pstk)+(stksz-1);
	extra->pbos=(OS_STK*)(pstk);

	extra->tskvar[TSK_PRIVATE]=private;

	// 2018-0823 no virtual console here
#if 0
	vconTaskInit(extra->tskvar[TSK_CONSOLE]);		// init for vcon
#endif

	if (BIT_VAL(atr_pri,TSK_START)) {
		OSSchedLock();
		err=OSTaskCreateExt(
			(void (*)(void*))task,		// entry_function
			(void*)0,					// value that is passed to the thread・s entry function
			extra->ptos,						// stack memory area (task・s top-of-stack)
			id,							// priority
			id,							// id
			extra->pbos,				// stack memory area (task・s bottom-of-stack)
			stksz,						// stack size
			0,							// exinf, this field is used by kernel to save VFP registers
			OS_TASK_OPT_SAVE_FP
		);
		ret=yl_oswrap_errcode(err);
		if (ret) {
			OSSchedUnlock();
			goto err1;
		}
		BIT_SET(extra->attr,EXTRA_STATE_STARTED);
		OSTaskNameSet(id,(INT8U*)extra,&err);
		OSSchedUnlock();
		ret=yl_oswrap_errcode(err);
		if (ret) goto err2;
	}

	*ppid=extra;	// Assign PPID.
	return 0;

err2:
	OSTaskDel(id);
err1:
	free(ptr);
err0:

	*ppid=0;
	return ret;
}


// === del_tsk ===
inline int yl_del_tsk(void* ptr) {
INT8U err,id;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((THREAD_EXTRA*)ptr)->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;
	id=((THREAD_EXTRA*)ptr)->id;

	err=OSTaskDel(id);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_TASK_IDLE OS_ERR_TASK_DEL OS_ERR_PRIO_INVALID
	// will happen: OS_ERR_TASK_NOT_EXIST OS_ERR_TASK_DEL_ISR

	// only OS_ERR_NONE & OS_ERR_TASK_NOT_EXIST are ok to free ptr
	// if OS_ERR_TASK_DEL OS_ERR_TASK_IDLE OS_ERR_TASK_DEL_ISR the task is not deleted.
	if ((err=OS_ERR_NONE)||(err==OS_ERR_TASK_NOT_EXIST)) {
		free(ptr);
	}

	return ret;
}


// === sta_tsk ===
inline int yl_sta_tsk(void* ptr, DWORD stacd) {
INT8U err;
int ret;
THREAD_EXTRA *extra;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((THREAD_EXTRA*)ptr)->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;
	extra=ptr;

	OSSchedLock();
	if (BIT_VAL(extra->attr,EXTRA_STATE_STARTED)) {
		OSSchedUnlock();
		return OSWRAP_NOT_DONE;
	}

	err=OSTaskCreateExt(
		(void (*)(void*))extra->task,	// entry_function
		(void*)stacd,					// value that is passed to the thread・s entry function
		extra->ptos,					// stack memory area (task・s top-of-stack)
		extra->id,						// priority
		extra->id,						// id
		extra->pbos,					// stack memory area (task・s bottom-of-stack)
		extra->stksz/sizeof(OS_STK),	// stack size, units in OS_STK
		0,								// exinf, this field is used by kernel to save VFP registers
		OS_TASK_OPT_SAVE_FP
	);
	ret=yl_oswrap_errcode(err);
	if (err) {
		OSSchedUnlock();
		return ret;
	}
	BIT_SET(extra->attr,EXTRA_STATE_STARTED);
	OSTaskNameSet(extra->id,(INT8U*)extra,&err);	// no err should happed here. if there is err, the kernel is mulfunctioning.
	OSSchedUnlock();
	return ret;
}


// === ter_tsk ===
inline int yl_ter_tsk(void* ptr) {
INT8U err;
int ret;
THREAD_EXTRA *extra;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((THREAD_EXTRA*)ptr)->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	// check if trying to self terminate.
	OSTaskNameGet(OS_PRIO_SELF,(INT8U**)&extra,&err);
	if (err) return yl_oswrap_errcode(err);

	if (extra->id==((THREAD_EXTRA*)ptr)->id) return OSWRAP_NOT_DONE;	// self ?

	OSSchedLock();
	if (!BIT_VAL(((THREAD_EXTRA*)ptr)->attr,EXTRA_STATE_STARTED)) {	// not even started ?
		OSSchedUnlock();
		return OSWRAP_NOT_AVAILABLE;
	}

	// delete task;
	err=OSTaskDel(((THREAD_EXTRA*)ptr)->id);
	// will NOT happen: OS_ERR_TASK_IDEL OS_ERR_TASK_DEL OS_ERR_TASK_DEL_ISR OS_ERR_PRIO_INVALID OS_ERR_TASK_NOT_EXIST
	// will happen: OS_ERR_NONE OS_ERR_TASK_NOT_EXIST
	if (!err||(err==OS_ERR_TASK_NOT_EXIST)) {
		BIT_CLR(((THREAD_EXTRA*)ptr)->attr,EXTRA_STATE_STARTED);
	}
	OSSchedUnlock();

	ret=yl_oswrap_errcode(err);

	return ret;
}


// === ext_tsk ===
void yl_ext_tsk(void) {
INT8U err;
THREAD_EXTRA *extra;

	OSTaskNameGet(OS_PRIO_SELF,(INT8U**)&extra,&err);
	if (err) return;

	OSSchedLock();
	err=OSTaskDel(extra->id);
	// will NOT happen: OS_ERR_TASK_IDEL OS_ERR_TASK_DEL OS_ERR_TASK_DEL_ISR OS_ERR_PRIO_INVALID OS_ERR_TASK_NOT_EXIST
	// will happen: OS_ERR_NONE OS_ERR_TASK_NOT_EXIST
	if (!err||(err==OS_ERR_TASK_NOT_EXIST)) {
		BIT_CLR(extra->attr,EXTRA_STATE_STARTED);
	}
	OSSchedUnlock();

	// ===== should not get here ====
	while(1);
}


// === exd_tsk ===
void yl_exd_tsk(void) {
INT8U err;
THREAD_EXTRA *extra;

	OSTaskNameGet(OS_PRIO_SELF,(INT8U**)&extra,&err);
	if (err) return;

	OSSchedLock();
	err=OSTaskDel(extra->id);
	// will NOT happen: OS_ERR_TASK_IDEL OS_ERR_TASK_DEL OS_ERR_TASK_DEL_ISR OS_ERR_PRIO_INVALID OS_ERR_TASK_NOT_EXIST
	// will happen: OS_ERR_NONE OS_ERR_TASK_NOT_EXIST
	if (!err||(err==OS_ERR_TASK_NOT_EXIST)) {
		free(extra);
	}
	OSSchedUnlock();

	// ===== should not get here ====
	while(1);
}


// === get_tid ===
inline void* yl_get_tid(void) {
INT8U err;
void *ptr;

	OSTaskNameGet(OS_PRIO_SELF,(INT8U**)&ptr,&err);
	if (err)  return 0;
	return ptr;

}


// === tsk_var ===
inline void* yl_tsk_var(int varid) {
THREAD_EXTRA *extra;

	extra=yl_get_tid();
	if (!extra) return &global_tskvar[varid];

	if ( extra->magic!=THREAD_EXTRA_MAGIC) return &global_tskvar[varid];
	return &extra->tskvar[varid];
}


// === rot_rdq ===
inline void yl_rot_rdq(void) {
	// not supported, there can only be 1 task per priority number in uCOS.
	// rotate ready queue is meaningless.
}


// === rel_wai ===
inline int yl_rel_wai(void* ptr) {
INT8U err;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((THREAD_EXTRA*)ptr)->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	OSSchedLock();
	err=OSTimeDlyResume(((THREAD_EXTRA*)ptr)->id);
	// will NOT happen: OS_ERR_PRIO_INVALID OS_ERR_TASK_NOT_EXIST
	// will happen: OS_ERR_TIME_NOT_DLY
	if (err) {
		OSSchedUnlock();
		return yl_oswrap_errcode(err);
	}

	BIT_SET(((THREAD_EXTRA*)ptr)->attr,TASK_RELEASE);
	OSSchedUnlock();

	return 0;
}


// === dly_tsk ===
inline int yl_dly_tsk(DWORD wait) {
	OSTimeDly(wait);
	return 0;
}


// === tslp_tsk ===
int yl_tslp_tsk(DWORD wait) {
INT8U ret;
THREAD_EXTRA *extra;

	extra=yl_get_tid();
	if (!extra) return OSWRAP_PTR_ERROR;
	if ( extra->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	OSSchedLock();
    if (extra->wake_up_call) {
    	extra->wake_up_call--;
    	OSSchedUnlock();
    	return 0;
    }
	BIT_SET(extra->attr,TASK_SLEEPING);
	OSSchedUnlock();
	while(1) {
		OSTimeDly(wait);
		OSSchedLock();
		if (BIT_VAL(extra->attr,TASK_WOKEUP)) {	// being woke up, normal exit
			ret=0;
			break;
		}
		else if (BIT_VAL(extra->attr,TASK_RELEASE)) {	// being released,
			ret=OSWRAP_WAIT_ABORTED;
			break;
		}
		else if (wait!=-1) {	// not wait forever
			ret=OSWRAP_TIME_OUT;
			break;
		}
		OSSchedUnlock();	// keep waiting ...
	}
	BIT_CLR(extra->attr,TASK_SLEEPING|TASK_WOKEUP|TASK_RELEASE);
	OSSchedUnlock();
	return ret;
}


// === wup_tsk ===
int yl_wup_tsk(void* ptr) {
INT8U err;
int ret;
THREAD_EXTRA *extra;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((THREAD_EXTRA*)ptr)->magic!=THREAD_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	extra=ptr;

    OSSchedLock();
	if (BIT_VAL(extra->attr,TASK_SLEEPING)) {
		OSSchedUnlock();
		err=OSTimeDlyResume(extra->id);
		ret=yl_oswrap_errcode(err);
		// will NOT happen: OS_ERR_PRIO_INVALID OS_ERR_TASK_NOT_EXIST
		// will happen: OS_ERR_TIME_NOT_DLY

		// if any err occurs, no wakeing up done, return with return code.
		if (!err) {
			BIT_SET(extra->attr,TASK_WOKEUP);
		}
		OSSchedUnlock();
		return ret;
	}
	extra->wake_up_call++;
	OSSchedUnlock();
	return 0;
}


//====
int is_released(void) {
THREAD_EXTRA *extra;

	extra=yl_get_tid();
	if (!extra) return 0;
	if ( extra->magic!=THREAD_EXTRA_MAGIC) return 0;
	OSSchedLock();
	if (BIT_VAL(extra->attr,TASK_RELEASE)) {	// being released,
		BIT_CLR(extra->attr,TASK_RELEASE);
		OSSchedUnlock();
		return 1;
	}
	OSSchedUnlock();
	return 0;
}
