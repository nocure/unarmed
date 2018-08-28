/*
 * sys_arch.c
 *
 *  Created on: 2009/8/30
 *      Author: ww
 */

#include <lwip/sys.h>
#include <platform/yl_sys.h>
#include <arch/sys_arch.h>


#if defined(TX_VERSION)

struct sys_thread_wrapper_t {
	struct sys_thread_wrapper_t *next;
	TX_THREAD thread; // not a ptr in this example, but the actual space
	struct sys_timeouts timeouts;
};

void sys_init(void) {

}

//==== Semaphore functions ====
sys_sem_t sys_sem_new(u8_t count) {
sys_sem_t sem;
UINT ret;

	sem=(sys_sem_t)calloc(1,sizeof(TX_SEMAPHORE));
	if (!sem) return SYS_SEM_NULL;
	ret=tx_semaphore_create(sem,"",count);
	if (ret!=TX_SUCCESS) return SYS_SEM_NULL;
	return sem;
}

void sys_sem_signal(sys_sem_t sem) {
	tx_semaphore_put(sem);
}


u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout) {
UINT ret;
ULONG time_start;
ULONG time_stop;

	time_start=tx_time_get();
	ret=tx_semaphore_get(sem,timeout?timeout:TX_WAIT_FOREVER);
	time_stop=tx_time_get();
	if (ret!=TX_SUCCESS) return SYS_ARCH_TIMEOUT;
	return (time_stop-time_start);
}

void sys_sem_free(sys_sem_t sem) {
	tx_semaphore_delete(sem);
	free(sem);
}

/* Mailbox functions. */


//UINT tx_queue_create(TX_QUEUE *queue_ptr, CHAR *name_ptr, UINT message_size, VOID *queue_start, ULONG queue_size);
sys_mbox_t sys_mbox_new(int size) {
sys_mbox_t mbox;
UINT ret;
ULONG* buf;

	mbox=(sys_mbox_t)calloc(1,sizeof(TX_QUEUE));
	if (!mbox) goto err0;
	buf=(ULONG*)malloc(size*4);
	if (!buf) goto err1;
	ret=tx_queue_create(mbox,"",TX_1_ULONG,buf,size*4);
	if (ret!=TX_SUCCESS) goto err2;

	return mbox;

err2:
	free(buf);
err1:
	free(mbox);
err0:
	return SYS_MBOX_NULL;
}


void sys_mbox_post(sys_mbox_t mbox, void *msg) {
	tx_queue_send(mbox,&msg,TX_WAIT_FOREVER);
}


err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg) {
UINT ret;
	ret=tx_queue_send(mbox,&msg,TX_NO_WAIT);
	if (ret!=TX_SUCCESS) return ERR_MEM;
	return ERR_OK;
}


UINT tx_queue_receive(TX_QUEUE *queue_ptr, VOID *destination_ptr, ULONG wait_option);
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout) {
UINT ret;
ULONG time_start;
ULONG time_stop;

	time_start=tx_time_get();
	ret=tx_queue_receive(mbox,(VOID*)msg,timeout?timeout:TX_WAIT_FOREVER);
	time_stop=tx_time_get();
	if (ret!=TX_SUCCESS) return SYS_ARCH_TIMEOUT;
	return (time_stop-time_start);
}


u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg) {
UINT ret;

	ret=tx_queue_receive(mbox,msg,TX_NO_WAIT);
	if (ret!=TX_SUCCESS) return SYS_MBOX_EMPTY;
	return 0;
}


void sys_mbox_free(sys_mbox_t mbox) {
ULONG* buf;

	buf=mbox->tx_queue_start;
	tx_queue_delete(mbox);
	free(buf);
	free(mbox);
}

//==== Thread functions. ====
static struct sys_timeouts lwip_system_timeouts = {0}; // Default timeouts list for lwIP
static struct sys_thread_wrapper_t* lwip_system_threads = 0; // a list of all threads created by lwIP

sys_thread_t sys_thread_new(char *name, void (* thread_entry)(void *arg), void *arg, int stacksize, int prio) {
sys_thread_t thptr;
VOID *stack;
UINT ret;
SYS_ARCH_DECL_PROTECT(old_val);

	thptr=(sys_thread_t)calloc(1,sizeof(struct sys_thread_wrapper_t));
	if (!thptr) goto err0;
	stack=malloc(stacksize);
	if (!stack) goto err1;

	SYS_ARCH_PROTECT(old_val); // Need to protect this -- preemption here could be a problem!
	thptr->next = lwip_system_threads;
	lwip_system_threads = thptr;
	SYS_ARCH_UNPROTECT(old_val);
	thptr->timeouts.next=0; // initialize the linked list to NULL

	ret=tx_thread_create(&thptr->thread,name,(VOID (*)(ULONG))thread_entry,(ULONG)arg,stack,stacksize,prio,prio,TX_NO_TIME_SLICE,TX_AUTO_START);
	if (ret!=TX_SUCCESS) goto err2;

	return thptr;

err2:
	free(stack);
err1:
	free(thptr);
err0:
	return 0;

}


struct sys_timeouts *sys_arch_timeouts(void) {
sys_thread_t thread = lwip_system_threads;
TX_THREAD *self = tx_thread_identify();

	for (; thread != NULL; thread = thread->next) {
		if (&thread->thread == self)
			return &thread->timeouts;
	}
	return &lwip_system_timeouts;
}


#elif defined(UITRON_VERSION)

static const void* const NullMessage=(void*)0x33884499;
//static int init_once=0;
#if defined(OWN_DYN_OBJ)
#define TSKID_LWIP_START	100
#define SEMID_LWIP_START	100
#define MBXID_LWIP_START	100

#define MAX_SEM	10
#define MAX_MBX	10
#define MAX_TSK	10

static ID lwip_sem[MAX_SEM];
static ID lwip_mbx[MAX_MBX];
static ID lwip_tsk[MAX_TSK];
#endif

void sys_init(void) {
//int i;

//	if (!init_once) {
//		for (i=0;i<MAX_SEM;i++) lwip_sem[i]=0;
//		for (i=0;i<MAX_MBX;i++) lwip_mbx[i]=0;
//		for (i=0;i<MAX_TSK;i++) lwip_tsk[i]=0;
//		init_once=1;
//	}

}

//==== Semaphore functions ====
#if defined(OWN_DYN_OBJ)
sys_sem_t sys_sem_new(u8_t count) {
sys_sem_t sem;
ER er;
int i;
T_CSEM semdef;
SYS_ARCH_DECL_PROTECT(old_val);

	SYS_ARCH_PROTECT(old_val); // Need to protect this -- preemption here could be a problem!
	for (i=0;i<MAX_SEM;i++) {
		if (!lwip_sem[i]) break;
	}
	if (i<MAX_SEM) lwip_sem[i]=-1;	// occupy the place
	SYS_ARCH_UNPROTECT(old_val);
	if (i>=MAX_SEM) goto err0;

	sem=SEMID_LWIP_START+i;
	semdef.exinf=0;
	semdef.sematr=TA_TPRI;
	semdef.isemcnt=1;
	semdef.maxsem=1;

	er= cre_sem(sem,&semdef);

	if (er!=E_OK) goto err1;

	lwip_sem[i]=sem;

	if (!count) er=wai_sem(sem);

	return sem;

err1:
	lwip_sem[i]=0;
err0:
	return SYS_SEM_NULL;

}
#else
sys_sem_t sys_sem_new(u8_t count) {
sys_sem_t sem;
T_CSEM semdef;

	semdef.exinf=0;
	semdef.sematr=TA_TPRI;
	semdef.isemcnt=1;
	semdef.maxsem=1;

	sem=dcre_sem(&semdef);
	if (sem<0) return SYS_SEM_NULL;
	if (!count) wai_sem(sem);

	return sem;

}
#endif


void sys_sem_signal(sys_sem_t sem) {
	sig_sem(sem);
}


u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout) {
ER er;
ULONG time_start;
ULONG time_stop;

	time_start=OSTICK;
	if (timeout) er=twai_sem(sem,(TMO)timeout);
	else er=wai_sem(sem);
	time_stop=OSTICK;
	if (er!=E_OK) return SYS_ARCH_TIMEOUT;
	return (time_stop-time_start);
}


#if defined(OWN_DYN_OBJ)
void sys_sem_free(sys_sem_t sem) {
int i;

	i=sem-SEMID_LWIP_START;
	if (lwip_sem[i]!=sem) return;
	lwip_sem[i]=0;
	del_sem(sem);
}
#else
void sys_sem_free(sys_sem_t sem) {
	del_sem(sem);
}
#endif


/* Mailbox functions. */

#if defined(OWN_DYN_OBJ)
sys_mbox_t sys_mbox_new(int size) {
sys_mbox_t mbx;
ER er;
int i;
SYS_ARCH_DECL_PROTECT(old_val);
T_CMBX mbxdef;

	SYS_ARCH_PROTECT(old_val); // Need to protect this -- preemption here could be a problem!
	for (i=0;i<MAX_MBX;i++) {
		if (!lwip_mbx[i]) break;
	}
	if (i<MAX_MBX) lwip_mbx[i]=-1;	// occupy the place
	SYS_ARCH_UNPROTECT(old_val);
	if (i>=MAX_MBX) goto err0;

	mbx=MBXID_LWIP_START+i;

	mbxdef.bufcnt=size;
	mbxdef.exinf=0;
	mbxdef.mbxatr=TA_TFIFO;

	er= cre_mbx(mbx,&mbxdef);

	if (er!=E_OK) goto err1;

	lwip_mbx[i]=mbx;
	return mbx;

err1:
	lwip_mbx[i]=0;
err0:
	return SYS_MBOX_NULL;

}
#else
sys_mbox_t sys_mbox_new(int size) {
sys_mbox_t mbx;
T_CMBX mbxdef;

	mbxdef.bufcnt=size;
	mbxdef.exinf=0;
	mbxdef.mbxatr=TA_TFIFO;

	mbx= dcre_mbx(&mbxdef);
	if (mbx<0) return SYS_MBOX_NULL;

	return mbx;

}
#endif


void sys_mbox_post(sys_mbox_t mbox, void *msg) {
ER er;

	if (!msg) {
		msg = (void*)&NullMessage;
	}
	while(1) {
		er=snd_msg(mbox,(T_MSG*)msg);
		if (er==E_OK) break;
		dly_tsk(1);
//		rot_rdq(0);
	}
}


err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg) {
ER er;

	if (!msg) {
		msg = (void*)&NullMessage;
	}
	er=snd_msg(mbox,(T_MSG*)msg);
	if (er!=E_OK) {
		return ERR_MEM;
	}
	return ERR_OK;
}


UINT tx_queue_receive(TX_QUEUE *queue_ptr, VOID *destination_ptr, ULONG wait_option);
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout) {
ER er;
ULONG time_start;
ULONG time_stop;


	time_start=OSTICK;
	if (timeout) er=trcv_msg((T_MSG**)msg,mbox,(TMO)timeout);
	else er=rcv_msg((T_MSG**)msg,mbox);
	time_stop=OSTICK;
	if (er!=E_OK) return SYS_ARCH_TIMEOUT;
	if (*msg==(void*)&NullMessage) {
		*msg=0;
	}
	return (time_stop-time_start);
}


u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg) {
ER er;

	er=prcv_msg((T_MSG**)msg,mbox);
	if (er!=E_OK)  return SYS_MBOX_EMPTY;
	if (*msg==(void*)&NullMessage) {
		*msg=0;
	}
	return 0;
}

#if defined(OWN_DYN_OBJ)
void sys_mbox_free(sys_mbox_t mbox) {
int i;

	i=mbox-MBXID_LWIP_START;
	if (lwip_mbx[i]!=mbox) return;
	lwip_mbx[i]=0;
	del_mbx(mbox);
}
#else
void sys_mbox_free(sys_mbox_t mbox) {
	del_mbx(mbox);
}
#endif

//==== Thread functions. ====
static struct sys_timeouts lwip_system_timeouts = {0}; // Default timeouts list for lwIP

#if defined(OWN_DYN_OBJ)
sys_thread_t sys_thread_new(char *name, void (* thread_entry)(void *arg), void *arg, int stacksize, int prio) {
sys_thread_t thptr;
int i;
void* p;
ER er;
T_CTSK tskdef;
SYS_ARCH_DECL_PROTECT(old_val);

	SYS_ARCH_PROTECT(old_val); // Need to protect this -- preemption here could be a problem!
	for (i=0;i<MAX_TSK;i++) {
		if (!lwip_tsk[i]) break;
	}
	if (i<MAX_TSK) lwip_tsk[i]=-1;	// occupy the place
	SYS_ARCH_UNPROTECT(old_val);
	if (i>=MAX_TSK) goto err0;

	thptr=TSKID_LWIP_START+i;
	p=(void*)calloc(1,sizeof(struct sys_timeouts));
	if (!p) goto err1;

	tskdef.exinf=p;
	tskdef.itskpri=prio;
	tskdef.stksz=stacksize;
	tskdef.task=(FP)thread_entry;
	tskdef.tskatr=TA_HLNG;

	er=cre_tsk(thptr,&tskdef);
	if (er!=E_OK) goto err2;
	lwip_tsk[i]=thptr;

	er=sta_tsk(thptr,0);
	if (er!=E_OK) goto err3;

	return thptr;

err3:
	del_tsk(thptr);
err2:
	free(p);
err1:
	lwip_tsk[i]=0;
err0:
	return 0;
}
#else
sys_thread_t sys_thread_new(char *name, void (* thread_entry)(void *arg), void *arg, int stacksize, int prio) {
sys_thread_t thptr;
void* p;
T_CTSK tskdef;

	p=(void*)calloc(1,sizeof(struct sys_timeouts));
	if (!p) goto err0;

	tskdef.exinf=p;
	tskdef.itskpri=prio;
	tskdef.stksz=stacksize;
	tskdef.task=(FP)thread_entry;
	tskdef.tskatr=TA_HLNG;

	thptr=dcre_tsk(&tskdef);
	if (thptr<0) goto err1;
	if (sta_tsk(thptr,0)) goto err2;

	return thptr;

err2:
	del_tsk(thptr);
err1:
	free(p);

err0:
	return 0;
}
#endif


#if defined(OWN_DYN_OBJ)
struct sys_timeouts *sys_arch_timeouts(void) {
ER er;
T_RTSK rtsk;
ID tid;
int i;

	er=get_tid(&tid);
	if (er!=E_OK) return &lwip_system_timeouts;
	for (i=0;i<MAX_TSK;i++) {
		if (lwip_tsk[i]==tid) break;
	}
	if (i>=MAX_TSK) return &lwip_system_timeouts;

	er=ref_tsk(&rtsk,tid);
	if (er!=E_OK) return &lwip_system_timeouts;
	return (struct sys_timeouts *)rtsk.exinf;
}
#else
struct sys_timeouts *sys_arch_timeouts(void) {
T_RTSK rtsk;
ID tid;

	if (get_tid(&tid)!=E_OK) return &lwip_system_timeouts;

	if (ref_tsk(&rtsk,tid)!=E_OK) return &lwip_system_timeouts;
	return (struct sys_timeouts *)rtsk.exinf;
}
#endif

#elif defined(OSWRAP_VERSION)

static const void* const NullMessage=(void*)0x33884499;
//static int init_once=0;
void sys_init(void) {
//int i;

//	if (!init_once) {
//		for (i=0;i<MAX_SEM;i++) lwip_sem[i]=0;
//		for (i=0;i<MAX_MBX;i++) lwip_mbx[i]=0;
//		for (i=0;i<MAX_TSK;i++) lwip_tsk[i]=0;
//		init_once=1;
//	}

}

//==== Semaphore functions ====
sys_sem_t sys_sem_new(u8_t count) {
sys_sem_t sem;
ER er;

	er=cre_sem(&sem,0,count);
	if (er<0) return SYS_SEM_NULL;
	while(!count--) wai_sem(sem);

	return sem;

}


void sys_sem_signal(sys_sem_t sem) {
	sig_sem(sem);
}


u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout) {
ER er;
ULONG time_start;
ULONG time_stop;

	time_start=OSTICK;
	if (timeout) er=twai_sem(sem,timeout);
	else er=wai_sem(sem);
	time_stop=OSTICK;
	if (er!=0) return SYS_ARCH_TIMEOUT;
	return (time_stop-time_start);
}


void sys_sem_free(sys_sem_t sem) {
	del_sem(sem);
}


/* Mailbox functions. */

sys_mbox_t sys_mbox_new(int size) {
sys_mbox_t mbx;
ER er;

	er=cre_mbx(&mbx,size);

	if (er<0) return SYS_MBOX_NULL;

	return mbx;

}


void sys_mbox_post(sys_mbox_t mbox, void *msg) {
ER er;

	if (!msg) {
		msg = (void*)&NullMessage;
	}
	while(1) {
		er=snd_msg(mbox,msg);
		if (er==0) break;
		dly_tsk(1);
//		rot_rdq(0);
	}
}


err_t sys_mbox_trypost(sys_mbox_t mbox, void *msg) {
ER er;

	if (!msg) {
		msg = (void*)&NullMessage;
	}
	er=snd_msg(mbox,msg);
	if (er!=0) {
		return ERR_MEM;
	}
	return ERR_OK;
}



u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout) {
ER er;
ULONG time_start;
ULONG time_stop;


	time_start=OSTICK;
	if (timeout) er=trcv_msg(mbox,msg,timeout);
	else er=rcv_msg(mbox,msg);
	time_stop=OSTICK;
	if (er!=0) return SYS_ARCH_TIMEOUT;
	if (*msg==(void*)&NullMessage) {
		*msg=0;
	}
	return (time_stop-time_start);
}


u32_t sys_arch_mbox_tryfetch(sys_mbox_t mbox, void **msg) {
ER er;

	er=prcv_msg(mbox,msg);
	if (er!=0)  return SYS_MBOX_EMPTY;
	if (*msg==(void*)&NullMessage) {
		*msg=0;
	}
	return 0;
}

void sys_mbox_free(sys_mbox_t mbox) {
	del_mbx(mbox);
}

//==== Thread functions. ====
static struct sys_timeouts lwip_system_timeouts = {0}; // Default timeouts list for lwIP

sys_thread_t sys_thread_new(char *name, void (* thread_entry)(void *arg), void *arg, int stacksize, int prio) {
sys_thread_t thptr;
void *p;
ER er;

	p=(void*)calloc(1,sizeof(struct sys_timeouts));
	if (!p) goto err0;

	er=cre_tsk(&thptr,(FUNCENTRY)thread_entry,stacksize,prio,p);
	if (er<0) goto err1;

	if (sta_tsk(thptr,0)) goto err2;

	return thptr;

err2:
	del_tsk(thptr);
err1:
	free(p);

err0:
	return 0;
}


struct sys_timeouts *sys_arch_timeouts(void) {
void **p;

	p=tsk_var(TSK_PRIVATE);
	if (!*p) {
		return &lwip_system_timeouts;
	}

	return (struct sys_timeouts *)*p;
}
#endif




