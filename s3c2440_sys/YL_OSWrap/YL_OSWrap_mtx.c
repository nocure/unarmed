/*
 * YL_OSWrap_mtx.c
 *
 *  Created on: 2012/8/15
 *      Author: Jon
 */
#include "platform/yl_sys.h"
#include "sct.h"
#include <ucos_ii.h>
#include "YL_OSWrap_priv.h"

typedef struct {
	DWORD magic;
	OS_EVENT* mtx;
	int	mtx_mark;
} MUTEX_EXTRA;


#define MUTEX_EXTRA_SIZE ALIGN16(sizeof(MUTEX_EXTRA))
#define MUTEX_EXTRA_MAGIC	('m'|('t'<<8)|('x'<<16)|('\0'<<24))
#define MUTEX_EMPTY		-1


//=====================================================================
// Mutex
//=====================================================================
// === cre_mtx ===
int yl_cre_mtx(void** ppid) {
#define MTX_MEM_SIZE 	(MUTEX_EXTRA_SIZE)
MUTEX_EXTRA	*ptr;
INT8U ret;

	ptr = (MUTEX_EXTRA*)malloc(sizeof(MTX_MEM_SIZE));
	if (!ptr){
		ret = OSWRAP_NO_MEMORY;
		goto err0;
	}
	ptr->magic = MUTEX_EXTRA_MAGIC;
	ptr->mtx_mark = MUTEX_EMPTY;

	ptr->mtx = OSSemCreate(1);
	if (!(ptr->mtx)){
		ret = OSWRAP_NO_MEMORY;
		goto err1;
	}

	*ppid=ptr;
	return 0;


err1:
	free(ptr);

err0:
	*ppid=0;
	return -ret;
}


// === del_mtx ===
inline int yl_del_mtx(void* ptr) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MUTEX_EXTRA*)ptr)->magic!=MUTEX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	/* Delete semphore anyway */
	OSSemDel(ptr, OS_DEL_ALWAYS, &err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_INVALID_OPT OS_ERR_TASK_WAITING OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_DEL_ISR

	if (err!=OS_ERR_DEL_ISR) {	// if not OS_ERR_DEL_ISR, we can safely delete it.
		((MUTEX_EXTRA*)ptr)->magic = 0;
		((MUTEX_EXTRA*)ptr)->mtx_mark = 0;
		free(ptr);
	}

	return ret;
}


// === get_mtx ===
inline int yl_get_mtx(void* ptr, DWORD wait) {
INT8U err;
int ret=0;
INT16U last;
#if 0
OS_TCB  p_task;
#endif

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MUTEX_EXTRA*)ptr)->magic!=MUTEX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;
#if 0
	err = OSTaskQuery(OS_PRIO_SELF ,&p_task);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_PRIO_INVALID OS_ERR_PRIO OS_ERR_TASK_NOT_EXIST OS_ERR_PDATA_NULL
	// will happen: OS_ERR_NONE

	if (err) return ret;
	OSSchedLock();
	if (((MUTEX_EXTRA*)ptr)->mtx_mark == MUTEX_EMPTY){
		((MUTEX_EXTRA*)ptr)->mtx_mark = p_task.OSTCBPrio;
	}
	else{
		if (OSPrioCur == ((MUTEX_EXTRA*)ptr)->mtx_mark){
			return OSWRAP_MUTEX_ERROR;
		}
	}
	OSSchedUnlock();
#endif
	if (wait==0) {
		last=OSSemAccept(((MUTEX_EXTRA*)ptr)->mtx);
		if (!last) ret=OSWRAP_TIME_OUT;		// out of semaphore
	}
	else {
		while (1) {
			OSSemPend(((MUTEX_EXTRA*)ptr)->mtx,0,&err);
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

	return ret;
}


// === put_mtx ===
inline int yl_put_mtx(void* ptr) {
INT8U err;
int ret;
#if 0
INT8U x, y;
OS_TCB  p_task;
OS_EVENT *e_ptr = (OS_EVENT*)((MUTEX_EXTRA*)ptr)->mtx;
#endif

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MUTEX_EXTRA*)ptr)->magic!=MUTEX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;
#if 0
	ret = OSTaskQuery(OS_PRIO_SELF ,&p_task);
	if (ret) return -ret;
	if (p_task.OSTCBPrio != ((MUTEX_EXTRA*)ptr)->mtx_mark){
		return OSWRAP_MUTEX_ERROR;
	}
	/* Get a higher priority of pending task before posting mutex*/
	OSSchedLock();
	if (e_ptr->OSEventGrp == 0){
		((MUTEX_EXTRA*)ptr)->mtx_mark = MUTEX_EMPTY;
	}
	else{
		y = OSUnMapTbl[e_ptr->OSEventGrp];
		x = OSUnMapTbl[e_ptr->OSEventTbl[y]];
		((MUTEX_EXTRA*)ptr)->mtx_mark = (INT8U)((y << 3u) + x);
	}
	OSSchedUnlock();
#endif

	err=(OSSemPost(((MUTEX_EXTRA*)ptr)->mtx));
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_SEM_OVF OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_NONE

	return ret;
}

