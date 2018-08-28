/*
 * sys_arch.h
 *
 *  Created on: 2009/8/30
 *      Author: ww
 */

#ifndef SYS_ARCH_H_
#define SYS_ARCH_H_

//#define TX_VERSION
//#define UITRON_VERSION
#define OSWRAP_VERSION

#if defined(TX_VERSION)
#include <tx_api.h>

typedef TX_SEMAPHORE*	sys_sem_t;
typedef TX_QUEUE*		sys_mbox_t;
typedef struct sys_thread_wrapper_t* sys_thread_t;

#define SYS_MBOX_NULL   (sys_mbox_t)0
#define SYS_SEM_NULL    (sys_sem_t)0

#elif defined(UITRON_VERSION)

#define SEMID_LWIP_START	100
#define MBXID_LWIP_START	100
#define TSKID_LWIP_START	100

typedef ID sys_sem_t;
typedef ID sys_mbox_t;
typedef ID sys_thread_t;
//typedef struct sys_thread_wrapper_t* sys_thread_t;

#define SYS_MBOX_NULL   (sys_mbox_t)0
#define SYS_SEM_NULL    (sys_sem_t)0

#elif defined(OSWRAP_VERSION)

typedef void* sys_sem_t;
typedef void* sys_mbox_t;
typedef void* sys_thread_t;
typedef int ER;

#define SYS_MBOX_NULL   (sys_mbox_t)0
#define SYS_SEM_NULL    (sys_sem_t)0

#endif

#endif /* SYS_ARCH_H_ */
