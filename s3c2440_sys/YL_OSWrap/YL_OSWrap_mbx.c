/*
 * YL_OSWrap_mbx.c
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
	OS_EVENT* q;	// count up semaphore
} MAILBOX_EXTRA;

#define MAILBOX_EXTRA_SIZE ALIGN16(sizeof(MAILBOX_EXTRA))
#define MAILBOX_EXTRA_MAGIC	('m'|('b'<<8)|('x'<<16)|('\0'<<24))

//=====================================================================
// Mailbox
//=====================================================================
// === cre_mbx ===
int yl_cre_mbx(void** ppid, int bufcnt) {
void *ptr,*qbuf;
MAILBOX_EXTRA *extra;
int ret;
#define BUF_SIZE_BYTE	(bufcnt*sizeof(void*))
#define MBX_MEM_SIZE	( MAILBOX_EXTRA_SIZE+BUF_SIZE_BYTE )

	if (bufcnt>65535) {
		ret=OSWRAP_OPTION_ERROR;
		goto err0;
	}
	ptr=malloc(MBX_MEM_SIZE);
	if (!ptr) {
		ret=OSWRAP_NO_MEMORY;
		goto err0;
	}

	memset(ptr,0,MBX_MEM_SIZE);

	extra=ptr;
	qbuf=ptr+MAILBOX_EXTRA_SIZE;

	extra->magic=MAILBOX_EXTRA_MAGIC;

	extra->q=OSQCreate(qbuf,bufcnt);
	if (!extra->q) {
		ret=OSWRAP_NO_MEMORY;
		goto err1;
	}

	*ppid=ptr;
	return 0;

err1:
	free(ptr);

err0:
	*ppid=0;
	return ret;
}


// === del_mbx ===
inline int yl_del_mbx(void* ptr) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MAILBOX_EXTRA*)ptr)->magic!=MAILBOX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	OSQDel(((MAILBOX_EXTRA*)ptr)->q,OS_DEL_ALWAYS,&err);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_INVALID_OPT OS_ERR_TASK_WAITING OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_DEL_ISR
	// if OS_ERR_TASK_WAITING do happen, don't delete it since the mbx is not deleted yet.
	if (!err||(err==OS_ERR_INVALID_OPT)||(err==OS_ERR_EVENT_TYPE)||(err==OS_ERR_PEVENT_NULL)) {
		free(ptr);
	}
	return ret;
}


// === snd_msg ===
inline int yl_snd_msg(void* ptr, void* pk_msg) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MAILBOX_EXTRA*)ptr)->magic!=MAILBOX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;

	err=OSQPost(((MAILBOX_EXTRA*)ptr)->q,pk_msg);
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL
	// will happen: OS_ERR_Q_FULL

	return ret;
}


// === rcv_msg ===
inline int yl_rcv_msg(void* ptr, void** ppk_msg, DWORD wait) {
INT8U err;
int ret;

	if (!ptr) return OSWRAP_PTR_ERROR;
	if ( ((MAILBOX_EXTRA*)ptr)->magic!=MAILBOX_EXTRA_MAGIC) return OSWRAP_PTR_ERROR;
	if (wait==0) {
		*ppk_msg=OSQAccept(((MAILBOX_EXTRA*)ptr)->q,&err);
	}
	else {
		while (1) {
			*ppk_msg=OSQPend(((MAILBOX_EXTRA*)ptr)->q,wait,&err);
			if (!err) break;
			else if (err==OS_ERR_TIMEOUT) {
				if (is_released()) return OSWRAP_WAIT_ABORTED;
				if (wait!=-1) return OSWRAP_TIME_OUT;
			}
		}
	}
	ret=yl_oswrap_errcode(err);
	// will NOT happen: OS_ERR_EVENT_TYPE OS_ERR_PEVENT_NULL OS_ERR_PEND_ABORT
	// will happen: OS_ERR_Q_EMPTY OS_ERR_TIMEOUT OS_ERR_PEND_ISR OS_ERR_PEND_LOCKED

	return ret;
}



