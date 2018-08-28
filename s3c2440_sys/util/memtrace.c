/*
 * memtrace.c
 *
 *  Created on: 2009/11/12
 *      Author: Wesley Wang
 */
#include "platform/YL_SYS.h"

#if 0
#define MAX_TRACES	1024

static void *mem_records[MAX_TRACES];

void tracemem(void* p) {
int i;
TX_INTERRUPT_SAVE_AREA;
	TX_DISABLE;
	for (i=0;i<MAX_TRACES;i++) {
//		if (mem_records[i]==p) break;
		if (!mem_records[i]) break;
	}
//	ASSERT(mem_records[i]!=p);
	ASSERT("",i<MAX_TRACES);
	mem_records[i]=p;
	TX_RESTORE;
}


void untracemem(void* p) {
int i;
TX_INTERRUPT_SAVE_AREA;
	TX_DISABLE;
	for (i=0;i<MAX_TRACES;i++) {
		if (mem_records[i]==p) break;
	}
	ASSERT("",i<MAX_TRACES);
	mem_records[i]=0;
	TX_RESTORE;
}


void istracemem(void* p) {
int i;
TX_INTERRUPT_SAVE_AREA;
	TX_DISABLE;
	for (i=0;i<MAX_TRACES;i++) {
		if (mem_records[i]==p) break;
	}
	ASSERT("",i>=MAX_TRACES);
	TX_RESTORE;
}
#endif


#if 0
if (mbox==102) {
	if (*((DWORD*)(msg-0x0c))!=0x87654321) {
		er=1;
	}
}
#endif


