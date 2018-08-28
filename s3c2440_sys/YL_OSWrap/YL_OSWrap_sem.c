/*
 * YL_OSWrap_sem.c
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

typedef struct {
	DWORD magic;
	OS_EVENT* sem;	// count up semaphore
	OS_EVENT* max;	// count down semaphore
} SEMAPHORE_EXTRA;


#define SEMAPHORE_EXTRA_SIZE ALIGN16(sizeof(SEMAPHORE_EXTRA))
#define SEMAPHORE_EXTRA_MAGIC	('s'|('e'<<8)|('m'<<16)|('\0'<<24))

//=====================================================================
// Semaphore
//=====================================================================
// === cre_sem ===


int yl_cre_sem(void** ppid, int isemcnt, int maxsem) {
SEMAPHORE_EXTRA *ptr;
INT8U err;
int ret;
#define SEM_MEM_SIZE	( SEMAPHORE_EXTRA_SIZE )

	if ((isemcnt<0)||(maxsem>65535)||(isemcnt>maxsem)) return OSWRAP_OPTION_ERROR;

	ptr=malloc(SEM_MEM_SIZE);
	if (!ptr) {
		ret=OSWRAP_NO_MEMORY;
		goto err0;
	}
	memset(ptr,0,SEM_MEM_SIZE);

	ptr->magic=SEMAPHORE_EXTRA_MAGIC;

	ptr->max=OSSemCreate(maxsem-isemcnt);
	if (!ptr->max) {
		ret=OSWRAP_NO_MEMORY;
		goto err1;
	}

	ptr->sem=OSSemCreate(isemcnt);
	if (!ptr->sem) {
		ret=OSWRAP_NO_MEMORY;
		goto err2;
	}
#if 0
	while(isemcnt--) {
		OSSemPend(ptr->max,0,&err);
		OSSemPost(ptr->sem);
	}
#endif
	*ppid=ptr;
	return 0;


err2:
	OSSemDel(ptr->max,OS_DEL_ALWAYS,&err);

err1:
	free(ptr);

err0:
	*ppid=0;
	return ret;
}


// === del_sem ===
inline int yl_del_sem(void* ptr) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((SEMAPHORE_EXTRA*)ptr)->magic!=SEMAPHORE_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	OSSemDel(((SEMAPHORE_EXTRA*)ptr)->sem,OS_DEL_ALWAYS,&err);
	// will NOT happen: OS_ERR_INVALID_OPT OS_ERR_TASK_WAITING OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_DEL_ISR

	if (err==OS_ERR_DEL_ISR) goto err0;

	OSSemDel(((SEMAPHORE_EXTRA*)ptr)->max,OS_DEL_ALWAYS,&err);
	// no err should happen here

	((SEMAPHORE_EXTRA*)ptr)->magic=0;

	free(ptr);

err0:

	ret=yl_oswrap_errcode(err);
	return ret;
}


// === sig_sem ===
inline int yl_sig_sem(void* ptr) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((SEMAPHORE_EXTRA*)ptr)->magic!=SEMAPHORE_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	// check if max count reached
	if (OSSemAccept(((SEMAPHORE_EXTRA*)ptr)->max)==0) return OSWRAP_NOT_AVAILABLE;

	// signal the semaphore
	err=OSSemPost(((SEMAPHORE_EXTRA*)ptr)->sem);
	// will NOT happen: OS_ERR_SEM_OVF OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_NONE

	ret=yl_oswrap_errcode(err);
	return ret;

}


// === wai_sem ===
inline int yl_wai_sem(void* ptr, DWORD wait) {
INT16U last;
INT8U err;
int ret=0;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((SEMAPHORE_EXTRA*)ptr)->magic!=SEMAPHORE_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	if (wait==0) {
		last=OSSemAccept(((SEMAPHORE_EXTRA*)ptr)->sem);
		if (!last) ret=OSWRAP_TIME_OUT;		// out of semaphore
		else ret=0;
	}
	else {
		while (1) {
			OSSemPend(((SEMAPHORE_EXTRA*)ptr)->sem,wait,&err);
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
	OSSemPost(((SEMAPHORE_EXTRA*)ptr)->max);		// got the semaphore, increase max
	return ret;
}

















