#include <sct.h>
#include "platform/YL_SYS.h"
#include <string.h>

DWORD g_CurrentUsed=0;
DWORD g_MaximumUsed=0;

#ifdef OS_MEM_MGR
static DWORD get_block_size(VP memptr) {
	return (*(DWORD*)(memptr-8))-(DWORD)memptr;
}

void* tx_os_malloc(DWORD size) {
register ER er;
VP memptr;
	// time-out 1000ms
	er = tget_blk((VP *)&memptr, MPLID_MALLOC, size, 1000);
	if (er) return (void*)0;

//	g_CurrentUsed+=ALIGN4(size);
	size=get_block_size(memptr);
	g_CurrentUsed+=size;
	if (g_MaximumUsed<g_CurrentUsed) g_MaximumUsed=g_CurrentUsed;
	return (void*)memptr;
}


void tx_os_free(void* memptr) {
DWORD s;
	rel_blk(MPLID_MALLOC, (VP)memptr);
	s=get_block_size(memptr);
	g_CurrentUsed-=s;
}

void *tx_os_calloc(DWORD num, DWORD size) {
void *p;
DWORD s;
	s=num * size;
	p=tx_os_malloc(s);
	if (p) memset(p,0,s);
	return p;
}


void *tx_os_realloc(void *mptr, DWORD size) {
//register ER er;
void * memptr;
DWORD s;


	if (!mptr) return tx_os_malloc(size);
	if (!size) {
		tx_os_free(mptr);
		return 0;
	}

//	s=(*(DWORD*)(mptr-8))-(DWORD)mptr;
	s=get_block_size(mptr);

	tx_os_free(mptr);

	memptr=tx_os_malloc(size);
	if (!memptr) return 0;
	if (memptr!=mptr) {
		if (s>size) s=size;
		memcpy(memptr,mptr,s);
	}

	return (void*)memptr;
}

DWORD tx_os_mleft(void) {
T_RMPL rmpl;

	ref_mpl(&rmpl,MPLID_MALLOC);
	return rmpl.frsz;
}
#endif
