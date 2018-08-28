/*
 * consumer_producer..c
 *
 *  Created on: 2010/1/19
 *      Author: Wesley Wang
 *
 *  2012-0413 ww
 *  - modified to use mutex to be absolutely correct scenario
 *  threadx 5.0 has mutex, old thread 3.0 has no mutex, used to
 *  utilitize a semaphore with 1 count to simulate mutex.
 *  - added buffer flush at stop
 *
 */
#include "sct.h"
#include <platform/YL_Sys.h>

typedef int ITEM_T;

// =========================================
#define BUFFER_SIZE	(1<<3)
static ITEM_T item_buffer[BUFFER_SIZE];
static int item_count=0;


static int buffer_wptr=0;
static int buffer_rptr=0;

static void print_buffer() {
int rptr;
int i;
	rptr=buffer_rptr;
	for (i=0;i<item_count;i++) {
		putch(item_buffer[rptr++]);
		rptr&=(BUFFER_SIZE-1);
	}
	for (;i<BUFFER_SIZE;i++) putch('_');

}

void put_item_to_buffer(ITEM_T item) {
void* tskid;

	if (item_count>=BUFFER_SIZE) {
		printf("OVERFLOW\n");
		return;
	}

	item_buffer[buffer_wptr++]=item;
	buffer_wptr&=(BUFFER_SIZE-1);
	item_count++;

	tskid=get_tid();
	printf("(%08x->%c) [",tskid,item);
	print_buffer();
	printf("]\n");

}


ITEM_T get_item_from_buffer(void) {
void* tskid;
ITEM_T item;

	if (item_count<=0) {
		printf("UNDERFLOW\n");
		return 0;
	}

	item=item_buffer[buffer_rptr++];
	buffer_rptr&=(BUFFER_SIZE-1);
	item_count--;

	tskid=get_tid();
	printf("(%08x<-%c) [",tskid,item);
	print_buffer();
	printf("]\n");
	return item;
}


// =========================================
static int cur_item=0;
ITEM_T produce_item(void) {
ITEM_T item;

	item=cur_item+'A';
	if (cur_item<25) cur_item++;
	else cur_item=0;
	return item;
}

void consume_item(ITEM_T item) {
}

// =========================================
void* sem_buffer_ready;
void* sem_buffer_slot;
void* mtx_buffer_access;

void producer_tsk(DWORD thread_input) {
ITEM_T item;
	while(1) {
		item=produce_item();
		wai_sem(sem_buffer_slot);
		get_mtx(mtx_buffer_access);
		put_item_to_buffer(item);
		put_mtx(mtx_buffer_access);
		sig_sem(sem_buffer_ready);
		dly_tsk(thread_input);
	}
}


void consumer_tsk(DWORD thread_input) {
ITEM_T item;
	while(1) {
		wai_sem(sem_buffer_ready);
		get_mtx(mtx_buffer_access);
		item=get_item_from_buffer();
		put_mtx(mtx_buffer_access);
		sig_sem(sem_buffer_slot);
		consume_item(item);
		dly_tsk(thread_input);
	}
}

// =========================================
#if 1
void ucon_test_pc(void) {
static int pc_started=0;
static void* tsk_p1=0,*tsk_p2;
static void* tsk_c1,*tsk_c2;

	printf("\n\n");

	if (!tsk_p1) {
		cre_sem(&sem_buffer_slot,BUFFER_SIZE,BUFFER_SIZE);
		cre_sem(&sem_buffer_ready,0,BUFFER_SIZE);
		cre_mtx(&mtx_buffer_access);

		//======================
		cre_tsk(&tsk_p1,producer_tsk,2048,21,0);
		cre_tsk(&tsk_p2,producer_tsk,2048,22,0);
		// ----
		cre_tsk(&tsk_c1,consumer_tsk,2048,23,0);
		cre_tsk(&tsk_c2,consumer_tsk,2048,24,0);
	}

	if (!pc_started) {

		sta_tsk(tsk_p1, 350);
		sta_tsk(tsk_p2, 250);
		sta_tsk(tsk_c1, 700);
		sta_tsk(tsk_c2, 200);
		put_mtx(mtx_buffer_access);
		pc_started=1;
	}
	else {
		pc_started=0;
		get_mtx(mtx_buffer_access);
		ter_tsk(tsk_c2);
		ter_tsk(tsk_c1);
		ter_tsk(tsk_p2);
		ter_tsk(tsk_p1);
		printf("flushing buffer...\n");
		while (pwai_sem(sem_buffer_ready)==0) {
			get_item_from_buffer();
			sig_sem(sem_buffer_slot);
		}
		printf("done.\n");

	}
}
#else
#include <platform/yl_fsio.h>
void ucon_test_pc(void) {
FILE* of;

	of=fopen("test.bin","w");
	fputc('a',of);
	fputc('b',of);
	fputc('c',of);
	fputc('d',of);
	fclose(of);

}
#endif
