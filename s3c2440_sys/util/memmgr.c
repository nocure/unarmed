/*
 * memmgr.c
 *
 *  Created on: 2009/4/2
 *      Author: Wesley Wang
 *
 *   Memory Manager
 *
 *   ww 2009-1016:	modified a bit to be more multi-task safe
 *   ww 2009-1113:	implemented a different approach
 *   				Added condition of selectable UITRON/THREADX mutex
 *   ww 2010-08xx:	Added fragment pool for fast searching free space
 *   ww 2010-0908:	Fixed a bug when aligned allocation, the blockade may place in a different fragment.
 *   ww 2010-0909:	Added reverse allocation
 *   ww 2010-0909:	Fixed incorrect next MCB's prev pointer on malloc
 *   ww 2014-0102:	Added CORRUPTION_DETECTION, Occuping twice as required size if enalbed.
 *   ww 2014-0102:	Fixed possible bugs.
 *   ww 2014-0115:	Fixed error in realloc which casuses data in old pointer being written with an MCB
 *   ww 2014-0115:	NULL pointer will not cause free to hang now. It simply returns.
 *   ww 2014-0115:	Modified realloc to stay in same address as long as possible.
 */
//#include <tx_port.h>
//#include <tx_api.h>
#include "platform/YL_SYS.h"

//#define USE_MUTEX
//#define UITRON
#define CORRUPTION_DETECTION
#define CLEAR_UNUSED_MCB

#define MEMMGR_TYPE1		// memmgr
//#define MEMMGR_TYPE2		// bytepool

// =============================================================
#if defined(USE_MUTEX)

#if defined(UITRON)
#include "sct.h"
//SEM_PRIVMEM
#define os_mutex_on wai_sem(SEM_PRIVMEM,TX_WAIT_FOREVER);
#define os_mutex_off sig_sem(SEM_PRIVMEM);
#define os_save_var
#else
#define os_mutex_on tx_mutex_get(&mutex,TX_WAIT_FOREVER);
#define os_mutex_off tx_mutex_put(&mutex);
#define os_save_var
#endif

#else
#define os_mutex_off XRESTORE
#define os_mutex_on XDISABLE
#define os_save_var	YL_PSR_SAVE_AREA
#endif

static const char* MCB_corrupted="MCB corrupted";

#if defined(USE_MUTEX)
TX_MUTEX mutex;
#endif


#if defined(MEMMGR_TYPE1)
#define MCB_MARKER_FREE				0x01c0eef4	//0xf4eec001	changed byte order for easier reading little endian in memory,
#define MCB_MARKER_USED				0xe2fe0fac	//0xac0ffee2

typedef struct MCB_S {
	struct MCB_S *blkNext;
	struct MCB_S *blkPrev;
	DWORD blkStatus;
	DWORD blkSize;
}MCB_T;

typedef struct SEG_POOL_S {
	MCB_T *seg;
	int next;
} SEG_POOL_T;

#define SEG_POOL_SIZE		256

static MCB_T *memoryStart;
static MCB_T *memoryEnd;
static SEG_POOL_T SegmentPool[SEG_POOL_SIZE];
static int seg_pool_head=-1;
static int seg_pool_free=-1;
static int cur_seg_pool_index=-1;
//static int seg_pool_used=0;

// =====================================================================
// ==== fragment management ============================================
// =====================================================================
static void memAddToPool_I(void* p) {
int cur;
	cur=seg_pool_head;
	while(cur>=0) {
		if (SegmentPool[cur].seg==p) return;
		cur=SegmentPool[cur].next;
	}
	cur=seg_pool_free;
	ASSERT("Too fragmented",cur>=0);
	seg_pool_free=SegmentPool[seg_pool_free].next;
	SegmentPool[cur].seg=p;
	SegmentPool[cur].next=seg_pool_head;
	seg_pool_head=cur;
//	seg_pool_used++;
}


static int memDelFromPool_I(void* p) {
int cur,prev;

	prev=-1;
	cur=seg_pool_head;
	while(cur>=0) {
		if (SegmentPool[cur].seg==p) {
			if (prev<0) {
				seg_pool_head=SegmentPool[cur].next;
			}
			else {
				SegmentPool[prev].next=SegmentPool[cur].next;
			}
			SegmentPool[cur].next=seg_pool_free;
			seg_pool_free=cur;
//			seg_pool_used--;
			return 1;
		}
		prev=cur;
		cur=SegmentPool[cur].next;
	}
	return 0;
}


static void* memGetSeg_I(int reset) {

	if (reset) cur_seg_pool_index=seg_pool_head;
	else cur_seg_pool_index=SegmentPool[cur_seg_pool_index].next;
	if (cur_seg_pool_index<0) return 0;
	return SegmentPool[cur_seg_pool_index].seg;
}


/* **WARNING** No error checking at all, the parameter MUST be correct
 * pmcb MUST be a valid MCB, size_needed MUST be 16 byte aligned.
 * the return pointer is a piece of memory that size is ( size_needed - sizeof(MCB) )
 */
static void* memMCBAlloc_I(MCB_T* pmcb, DWORD size_needed) {
MCB_T *newmcb;

// Create a free memory block with the memory left - if enough!
//	size_needed+=sizeof(MCB_T);
	memDelFromPool_I(pmcb);
	ASSERT("memMCBAlloc",pmcb->blkSize>= (size_needed));
	if(pmcb->blkSize> (size_needed)) {	// if block size equals to size_needed, no need to create new MCB
//		newmcb = (void*)pmcb + size_needed;
		newmcb = (void*)pmcb + sizeof(MCB_T) + size_needed;
		newmcb->blkNext = pmcb->blkNext;
		newmcb->blkPrev = pmcb;
		newmcb->blkSize = (void*)newmcb->blkNext-(void*)newmcb- sizeof(MCB_T);
//		newmcb->blkSize = (void*)newmcb->blkNext-(void*)newmcb;
		newmcb->blkStatus = MCB_MARKER_FREE;
		pmcb->blkNext->blkPrev=newmcb;
		pmcb->blkSize = (void*)newmcb-(void*)pmcb - sizeof(MCB_T);
		pmcb->blkNext = newmcb;
		memAddToPool_I(newmcb);
	}
	pmcb->blkStatus = MCB_MARKER_USED;

	return ((void*) pmcb) + sizeof(MCB_T);

}


static void* memGetBestMCB_I(DWORD size_needed) {
MCB_T *pmcb;
MCB_T *newmcb;

	// get the best block
	pmcb = 0;
	newmcb = memGetSeg_I(1);
	while (newmcb != 0) {
		ASSERT(MCB_corrupted,newmcb->blkStatus==MCB_MARKER_FREE);
		// is the block fit the memory requirement ?
		if (newmcb->blkSize>= size_needed) { // yes, this is a candidate
			// is there a best yet ?
			if (!pmcb) { // no
				pmcb = newmcb;
			}
			else { // yes,but this is better
				if (newmcb->blkSize < pmcb->blkSize) {
					pmcb = newmcb;
				}
			}
		}
		newmcb=memGetSeg_I(0);
	}
	return pmcb;
}

// =====================================================================
// =====================================================================
void memInit(DWORD base, DWORD size) {
DWORD abase;
os_save_var;

#if defined(USE_MUTEX)&& !defined(UITRON)
UINT ret;
	ret=tx_mutex_create(&mutex,0,TX_NO_INHERIT);
	ASSERT("memmgr: mutex creation",!ret);
#endif

	os_mutex_on;

	for (abase=0;abase<SEG_POOL_SIZE;abase++) {
		SegmentPool[abase].next=seg_pool_free;
		seg_pool_free=abase;
	}

	abase=ALIGN16(base);
	size-=(abase-base);
	size=ALIGN16(size);

	memoryStart = (MCB_T *) abase;

	// zero this memory block
//    memset(memoryStart, 0xfd, size);

	// Build the first free memory block & end mcb
	memoryEnd=(void*)memoryStart+size-sizeof(MCB_T);

	memoryStart->blkSize = (void*)memoryEnd - (void*)memoryStart - sizeof(MCB_T);
	memoryStart->blkStatus = MCB_MARKER_FREE;
	memoryStart->blkPrev=0;
	memoryStart->blkNext=memoryEnd;

	memoryEnd->blkSize=0;
	memoryEnd->blkStatus=MCB_MARKER_USED;
	memoryEnd->blkPrev=memoryStart;
	memoryEnd->blkNext=0;

	memAddToPool_I(memoryStart);

	os_mutex_off;

}


static void* memAlloc_I(DWORD size_needed) {
MCB_T *pmcb;

	pmcb=memGetBestMCB_I(size_needed);
	if(!pmcb) return 0;
	pmcb=memMCBAlloc_I(pmcb,size_needed);

	return pmcb;
}

static MCB_T* prevUnused_I(MCB_T* pmcb) {
#if defined(CLEAR_UNUSED_MCB)
MCB_T *t;
#endif

	while (pmcb->blkPrev !=0) {
		if (pmcb->blkPrev->blkStatus == MCB_MARKER_FREE) {
			memDelFromPool_I(pmcb);
#if defined(CLEAR_UNUSED_MCB)
			t=pmcb;
#endif
			pmcb = pmcb->blkPrev;
#if defined(CLEAR_UNUSED_MCB)
			t->blkNext=(void*)0x78563411;
			t->blkPrev=(void*)0x78563422;
			t->blkStatus=0x78563433;
			t->blkSize=0x78563444;
#endif
		}
		else {
//			ASSERT(MCB_corrupted,pmcb->blkPrev->blkStatus==MCB_MARKER_USED);
			break;
		}
	}
	return pmcb;
}


static MCB_T* nextUnused_I(MCB_T* next_block) {
#if defined(CLEAR_UNUSED_MCB)
MCB_T *t;
#endif

	while (next_block) {
		ASSERT(MCB_corrupted,ALIGN16((DWORD)next_block)==(DWORD)next_block);
		if (next_block->blkStatus == MCB_MARKER_USED) {
			break;
		}
		else ASSERT(MCB_corrupted,next_block->blkStatus==MCB_MARKER_FREE);
		memDelFromPool_I(next_block);
#if defined(CLEAR_UNUSED_MCB)
		t=next_block;
#endif
		next_block = next_block->blkNext;
#if defined(CLEAR_UNUSED_MCB)
		t->blkNext=(void*)0x88563411;
		t->blkPrev=(void*)0x88563422;
		t->blkStatus=0x88563433;
		t->blkSize=0x88563444;
#endif
	}
	return next_block;
}


static void memFree_I(void *memory) {
MCB_T *pmcb;
MCB_T *next_block;
	ASSERT(MCB_corrupted,ALIGN16((DWORD)memory)==(DWORD)memory);
	pmcb = (MCB_T *) (((unsigned char *) memory) - sizeof(MCB_T));
	ASSERT(MCB_corrupted,pmcb->blkStatus==MCB_MARKER_USED);
	pmcb->blkStatus = MCB_MARKER_FREE;
	next_block=pmcb->blkNext;

	// at this point, if there are unused mcbs over the one pmcb to be freed. it will be merged together.
	// next_block is next mcb, check the mcb and following mcbs if it's unused,  will be merged together too.
	pmcb=prevUnused_I(pmcb);
	next_block=nextUnused_I(next_block);

	pmcb->blkNext = next_block;
	next_block->blkPrev = pmcb;
	pmcb->blkSize = (void*)next_block - (void*)pmcb - sizeof(MCB_T);
	memAddToPool_I(pmcb);

	return;
}


#if defined(CORRUPTION_DETECTION)
#define STUFFING_MAGIC	(0xaa5555aa)
#define EMPTY_MAGIC		(0xf0debc0a)
static void placeStuffing(DWORD *p, DWORD s) {
	s/=8;
	p+=s;
	while(s--) *p++=STUFFING_MAGIC;
}


static void checkStuffing(DWORD *p) {
	DWORD s;

	s=((MCB_T*)( (void*)p - sizeof(MCB_T) )) ->blkSize/8;
	p+=s;
	while(s--) {
		ASSERT("Corrupted",*p==STUFFING_MAGIC);
//		*p++=EMPTY_MAGIC;
	}
}
#endif

// ==================================================================
// ==================================================================
void* memAlloc(DWORD size_requested) {
os_save_var;
DWORD size_needed;
MCB_T *pmcb;

	// Make sure that the memory requested in aligned on 16 bytes.
	size_needed = ALIGN16(size_requested);
	// ww 2014-0102 added { memory corruption detection
#if defined(CORRUPTION_DETECTION)
	size_needed*=2;
#endif
	// ww 2014-0102 added }

	os_mutex_on;
	pmcb=memAlloc_I(size_needed);
	os_mutex_off;

#if defined(CORRUPTION_DETECTION)
	placeStuffing((DWORD*)pmcb,size_needed);
#endif

	return pmcb;
}


void memFree(void *memory) {
os_save_var;

	if (!memory) return;
	os_mutex_on;
#if defined(CORRUPTION_DETECTION)
	checkStuffing(memory);
#endif
	memFree_I(memory);
	os_mutex_off;
}

#if 0
void* memRealloc(void* memory, DWORD size_requested) {
void * p;
DWORD s;
MCB_T *pmcb;
os_save_var;

	if (!memory) return memAlloc(size_requested);
	if (!size_requested) {
		memFree(memory);
		return 0;
	}
	size_requested = ALIGN16(size_requested);

	os_mutex_on;
	pmcb = (MCB_T *) (memory - sizeof(MCB_T));
	ASSERT(MCB_corrupted,pmcb->blkStatus==MCB_MARKER_USED);
	// ww 2014-0102 added { memory corruption detection
	s=pmcb->blkSize;
#if defined(CORRUPTION_DETECTION)
	size_requested*=2;
#endif
	// ww 2014-0102 added }


	p=memAlloc_I(size_requested);
	ASSERT(MCB_corrupted,p);
	if (s>size_requested) s=size_requested;
#if defined(CORRUPTION_DETECTION)
	memcpy(p,memory,s/2);
#else
	memcpy(p,memory,s);
#endif
	memFree_I(memory);

#if defined(CORRUPTION_DETECTION)
	placeStuffing(p,size_requested);
#endif

	os_mutex_off;
	return (void*)p;
}
#else
void* memRealloc(void* memory, DWORD size_requested) {
void * p;
DWORD s;
MCB_T *pmcb;
MCB_T *newmcb;
os_save_var;

	if (!memory) return memAlloc(size_requested);
	if (!size_requested) {
		memFree(memory);
		return 0;
	}
	size_requested = ALIGN16(size_requested);

	os_mutex_on;
	pmcb = (MCB_T *) (memory - sizeof(MCB_T));
	ASSERT(MCB_corrupted,pmcb->blkStatus==MCB_MARKER_USED);
	// ww 2014-0102 added { memory corruption detection
	s=pmcb->blkSize;
#if defined(CORRUPTION_DETECTION)
	size_requested*=2;
#endif
	// ww 2014-0102 added }

	if (size_requested==s) return memory;
	if (size_requested<s) {	// shrink
		pmcb->blkSize=size_requested;
		newmcb=(void*)pmcb + sizeof(MCB_T) + size_requested;
		newmcb->blkPrev=pmcb;
		newmcb->blkStatus=MCB_MARKER_FREE;
		if (pmcb->blkNext->blkStatus==MCB_MARKER_USED) {
			// the following mcb is used, so make a fragment here
			newmcb->blkNext=pmcb->blkNext;
		}
		else {
			memDelFromPool_I(pmcb->blkNext);
			// the following mcb is free, merge them.
			newmcb->blkNext=pmcb->blkNext->blkNext;
		}
		newmcb->blkNext->blkPrev=newmcb;
		pmcb->blkNext=newmcb;
		newmcb->blkSize=(void*)newmcb->blkNext-(void*)newmcb-sizeof(MCB_T);
		memAddToPool_I(newmcb);
		p=(void*)pmcb+sizeof(MCB_T);
	}
	else {	// enlarge
		if ((pmcb->blkNext->blkStatus==MCB_MARKER_FREE)&&( (s+pmcb->blkNext->blkSize+sizeof(MCB_T) ) >=  size_requested  ) )	{	// following mcb is free & enough ?
			pmcb->blkSize=size_requested;
			newmcb=(void*)pmcb + sizeof(MCB_T) + size_requested;
			memDelFromPool_I(pmcb->blkNext);
			if (newmcb!=pmcb->blkNext->blkNext) {		// if newmcb is not at the edge, make a new fragment
				newmcb->blkNext=pmcb->blkNext->blkNext;
				newmcb->blkNext->blkPrev=newmcb;
				newmcb->blkSize=(void*)newmcb->blkNext-(void*)newmcb-sizeof(MCB_T);
				newmcb->blkStatus=MCB_MARKER_FREE;
				memAddToPool_I(newmcb);
			}
			newmcb->blkPrev=pmcb;
			pmcb->blkNext=newmcb;
			p=(void*)pmcb+sizeof(MCB_T);
		}
		else {	// not enough, do it conventional way
				p=memAlloc_I(size_requested);
				ASSERT(MCB_corrupted,p);
				if (s>size_requested) s=size_requested;
#if defined(CORRUPTION_DETECTION)
				memcpy(p,memory,s/2);
#else
				memcpy(p,memory,s);
#endif
				memFree_I(memory);

		}

	}

#if defined(CORRUPTION_DETECTION)
	placeStuffing(p,size_requested);
#endif

	os_mutex_off;
	return (void*)p;
}
#endif

DWORD memGetStatus(MEMORY_STATUS_T* pms) {
MCB_T *pmcb;
DWORD free_size;
DWORD used_size;
DWORD largest_free;
int num_segment,seg,mcbs;
os_save_var;

	os_mutex_on;
	pmcb = memoryStart;
	free_size=0;
	used_size=0;
	largest_free=0;
	num_segment=0;
	seg=0;
	mcbs=0;
	while (pmcb) {
		if (pmcb->blkStatus==MCB_MARKER_FREE) {
			free_size+=pmcb->blkSize;
			if (pmcb->blkSize>largest_free) largest_free=pmcb->blkSize;
			if (seg) num_segment++;
			seg=0;
			mcbs++;
		}
		else if (pmcb->blkStatus==MCB_MARKER_USED) {
			used_size+=pmcb->blkSize;
			seg=1;
			mcbs++;
		}
		else { // MCB corrupted !!!
			break;
		}
		pmcb=pmcb->blkNext;
	}

	os_mutex_off;

	if (pms) {
		pms->used=used_size;
		pms->segs=num_segment;
		pms->largest=largest_free;
		pms->mcbs=mcbs;
	}

	return free_size;

}


static int aalloc_enter_count;
static void *memAlignedAlloc_r_I(DWORD size, int align_bit) {
void *p;
DWORD align_offset;
DWORD blockade_size;
MCB_T *blockade;

	aalloc_enter_count++;
	blockade=0;

	p = memAlloc_I(size);
	align_offset=((DWORD)p)&(align_bit-1);
	if (!align_offset) goto err0;
	memFree_I(p);

	blockade_size=align_bit-align_offset-sizeof(MCB_T);

	blockade=p-sizeof(MCB_T);
	if (blockade->blkSize<blockade_size) {
		blockade_size=blockade->blkSize;
	}

	blockade = memMCBAlloc_I(blockade,blockade_size);
	ASSERT("aalloc: Unable to place blockade",blockade==p);
	if (aalloc_enter_count<=5) {
		p=memAlignedAlloc_r_I(size,align_bit);
	}
	else p=0;
	memFree_I(blockade);
err0:
	aalloc_enter_count--;
	return p;
}


void *memAlignedAlloc(DWORD size_requested, int alignbits) {
void *p;
os_save_var;
	aalloc_enter_count=0;
#if defined(CORRUPTION_DETECTION)
	size_requested*=2;
#endif
	os_mutex_on;
	p=memAlignedAlloc_r_I(size_requested,(1<<alignbits));
	os_mutex_off;
#if defined(CORRUPTION_DETECTION)
	placeStuffing(p,size_requested);
#endif
	return p;
}


// allocate a memory block reversely, from the end of memory
void* memRevAlloc(DWORD size_requested) {
MCB_T *pmcb;
DWORD size_needed;
DWORD blockade_size;
MCB_T *newmcb;
os_save_var;

	// Make sure that the memory requested in aligned on 16 bytes.
	size_needed = ALIGN16(size_requested);
	os_mutex_on;

	pmcb=memGetBestMCB_I(size_needed);
	if(!pmcb) {
		os_mutex_off;
		return 0;
	}
	// ww 2014-0102 added { memory corruption detection
#if defined(CORRUPTION_DETECTION)
	size_needed*=2;
#endif
	// ww 2014-0102 added }
	blockade_size=pmcb->blkSize-size_needed;

	newmcb=(void*)pmcb+blockade_size;
	newmcb->blkNext=pmcb->blkNext;
	newmcb->blkPrev=pmcb;
	newmcb->blkSize=size_needed;		//(void*)newmcb->blkNext - (void*)newmcb - sizeof(MCB_T);
	newmcb->blkStatus=MCB_MARKER_USED;
	pmcb->blkNext->blkPrev=newmcb;
	pmcb->blkSize-= (size_needed + sizeof(MCB_T));
//	pmcb->blkSize=(void*)newmcb-(void*)pmcb - sizeof(MCB_T);
	pmcb->blkNext=newmcb;

	os_mutex_off;

	pmcb=(void*)newmcb + sizeof(MCB_T);
#if defined(CORRUPTION_DETECTION)
	placeStuffing((DWORD*)pmcb,size_needed);
#endif
	return pmcb;
}



#elif defined(MEMMGR_TYPE2)



static TX_BYTE_POOL priv_mem;

void memInit(DWORD base, DWORD size) {
    tx_byte_pool_create(&priv_mem, 0, (void*)base, size);
}


void* memAlloc(DWORD size_requested) {
UINT ret;
void* memptr=0;
	ret=tx_byte_allocate(&priv_mem, &memptr, size_requested, TX_NO_WAIT);
	if (ret) return 0;

	return memptr;
}


void* memRealloc(void* memory, DWORD size_requested) {
void* memptr;
	memptr=memAlloc(size_requested);
	if (!memptr) return 0;
	memcpy(memptr,memory,size_requested);
	free(memory);
	return memptr;
}


void memFree(void* memory) {
    tx_byte_release(memory);
}


static void *memAlignedAlloc_r(U32 size, int alignbits) {
DWORD align_bit, align_mask;
void *p,*blockade;
DWORD align_offset;

	blockade=0;
	align_bit=(1<<alignbits);
	align_mask=(align_bit-1);

	p = memAlloc(size);
	align_offset=((DWORD)p)&align_mask;
	if (!align_offset) return p;
	memFree(p);
	blockade = memAlloc(align_bit-align_offset-8);
	ASSERT("aalloc: Unable to place blockade",blockade==p);

	p=memAlignedAlloc_r(size,alignbits);
	memFree(blockade);
	return p;
}


void *memAlignedAlloc(U32 size, int alignbits) {
TX_INTERRUPT_SAVE_AREA
void *p;
	TX_DISABLE
	p=memAlignedAlloc_r(size,alignbits);
	TX_RESTORE
	return p;
}


DWORD memGetStatus(MEMORY_STATUS_T* pms) {
	if (pms) {
		pms->segs=0;
		pms->used=priv_mem.tx_byte_pool_size-priv_mem.tx_byte_pool_available;
		pms->largest=0;
		pms->mcbs=priv_mem.tx_byte_pool_fragments;
	}
	return priv_mem.tx_byte_pool_available;
}


#endif

