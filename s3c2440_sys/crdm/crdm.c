/*
 * crdm.c
 * Central Resource Depot Manager
 *
 *  Created on: 2009/4/7
 *      Author: Wesley Wang
 */

#include "platform/YL_SYS.h"
#include <platform/yl_fsio.h>
#include <string.h>
#include "crdm_priv.h"

static DEPOT_INFO_T g_CRD[MAX_SUPPORTED_DEPOT];
CHUNK_CACHE_T *g_UsedCacheList,*g_FreeCacheList;


void crdDisplayCacheStatus(void) {
CHUNK_CACHE_T *pcc;
int cu,cf;

	cu=cf=0;

	pcc=g_UsedCacheList;
	while (pcc) {
		printf("resid: 0x%08x chunk: %d\n",pcc->resid,pcc->cid);
		cu++;
		pcc=pcc->next;
	}

	pcc=g_FreeCacheList;
	while (pcc) {
		cf++;
		pcc=pcc->next;
	}
	printf("%d used, %d free\n",cu,cf);

}


int crdInit(void) {
int i;
CHUNK_CACHE_T *pcc,*n;

	for (i=0;i<MAX_SUPPORTED_DEPOT;i++) memset(&g_CRD[i],0,sizeof(DEPOT_INFO_T));

	for (i=0;i<NUM_CHUNK_CACHE;i++) {
		pcc=(CHUNK_CACHE_T*)malloc(sizeof(CHUNK_CACHE_T));
		if (pcc) {
			memset(pcc,0,sizeof(CHUNK_CACHE_T));
			pcc->next=g_FreeCacheList;
			g_FreeCacheList=pcc;
		}
		else {	// error occured !!
			pcc=g_FreeCacheList;
			while (pcc) {
				n=pcc->next;
				free(pcc);
				pcc=n;
			}
			return -1;
		}
	}
	return 0;
}


DEPOT_INFO_T* crdGetDepot(DWORD number) {
	if (number>=MAX_SUPPORTED_DEPOT) return 0;
	return &g_CRD[number];
}


CHUNK_CACHE_T * crdFindChunkInCache(DWORD resid, DWORD cid) {
CHUNK_CACHE_T *pcc,*p1,*p2;

	pcc=g_UsedCacheList;
	p1=0;
	p2=0;

	while (pcc) {
		if ((pcc->resid==resid)&&(pcc->cid==cid)) {
			if (p2) {
				p2->next=pcc;
				p1->next=pcc->next;
				pcc->next=p1;

			}
			else if (p1) {
				p1->next=pcc->next;
				pcc->next=p1;
				g_UsedCacheList=pcc;
			}
			return pcc;
		}
		p2=p1;
		p1=pcc;
		pcc=pcc->next;
	}

	return 0;
}


void crdAddCache(CHUNK_CACHE_T* pcc) {
	pcc->next=g_UsedCacheList;
	g_UsedCacheList=pcc;
}


void crdReturnCache(CHUNK_CACHE_T* pcc) {
//	if (pcc->pointer) free(pcc->pointer);
	memset(pcc,0,sizeof(CHUNK_CACHE_T));
	pcc->next=g_FreeCacheList;
	g_FreeCacheList=pcc;
}


CHUNK_CACHE_T* crdGetFreeCache(void) {
CHUNK_CACHE_T *pcc,*p1,*p2;

	pcc=g_FreeCacheList;

	if (pcc) {
		g_FreeCacheList=pcc->next;
	}
	else {
		p2=0;
		p1=0;
		pcc=g_UsedCacheList;
		while(pcc) {
			p2=p1;
			p1=pcc;
			pcc=pcc->next;
		}
		p2->next=0;
		pcc=p1;
	}

	// clear content
	memset(pcc,0,sizeof(CHUNK_CACHE_T));
	return pcc;
}


/*
 * It's cache, it is meant to be in memory !!
 * So no free necessary
*/
/*
int crdFreeResCache(DWORD resid) {
CHUNK_CACHE_T *pcc,*p1;
int found;

	pcc=g_UsedCacheList;
	p1=0;
	found=0;
	while (pcc) {
		if (pcc->resid==resid) {
			if (p1) {
				p1->next=pcc->next;
			}
			else {
				g_UsedCacheList=pcc->next;
			}
			if (pcc->pointer) free(pcc->pointer);
			memset(pcc,0,sizeof(CHUNK_CACHE_T));
			pcc->next=g_FreeCacheList;
			g_FreeCacheList=pcc;

			if (p1) {
				pcc=p1->next;
			}
			else {
				pcc=g_UsedCacheList;
			}
			found++;
		}
		else {
			p1=pcc;
			pcc=pcc->next;
		}
	}

	return found;
}
*/

// ==============================================================

HRES crdOpen(DWORD resid) {
DEPOT_INFO_T *pDep;

	pDep=crdGetDepot(resid>>DEPOT_ID_SHIFT);
	if (!BIT_VAL(pDep->flags,DEPOT_FLAG_VALID)) return 0;

	if (!pDep->ops->open) return 0;

	return (HRES)pDep->ops->open(pDep,resid);
}


int crdRead(HRES hRes, BYTE* buf, DWORD size) {
RES_INFO_T* pRes;

	if (!hRes) return 0;

	pRes=(RES_INFO_T*)hRes;
	if (pRes->pos>=pRes->size) return 0;

	if (!pRes->dep->ops->read) return 0;

	return pRes->dep->ops->read(pRes,buf,size);
}


int crdSeek(HRES hRes, int off, DWORD where) {
RES_INFO_T* pRes;
DWORD newp;

	if (!hRes) return -1;

	pRes=(RES_INFO_T*)hRes;

	switch(where) {
		case CRD_SEEKSET:
			newp=off;
		break;

		case CRD_SEEKCUR:
			newp=pRes->pos+off;
		break;

		case CRD_SEEKEND:
			newp=pRes->size+off;
		break;

		default:
			return -1;
	}
	if (newp<0) pRes->pos=0;
	else if (newp>pRes->size) pRes->pos=pRes->size;
	else pRes->pos=newp;

	if (pRes->dep->ops->seek) return pRes->dep->ops->seek(pRes,off,where);

	return 0;
}


int crdTell(HRES hRes) {
RES_INFO_T* pRes;

	if (!hRes) return -1;
	pRes=(RES_INFO_T*)hRes;
	return pRes->pos;
}


DWORD crdSize(HRES hRes) {
RES_INFO_T* pRes;

	if (!hRes) return -1;
	pRes=(RES_INFO_T*)hRes;
	return pRes->size;
}


int crdEOF(HRES hRes) {
RES_INFO_T* pRes;

	if (!hRes) return -1;
	pRes=(RES_INFO_T*)hRes;
	return (pRes->pos>=pRes->size);
}


void crdClose(HRES hRes) {
RES_INFO_T* pRes;

	if (!hRes) return;

	pRes=(RES_INFO_T*)hRes;
	if (!pRes->dep->ops->close) return;

	return pRes->dep->ops->close(pRes);
}


void* crdLock(HRES hRes) {
RES_INFO_T* pRes;

	if (!hRes) return 0;

	pRes=(RES_INFO_T*)hRes;
	if (!pRes->dep->ops->lock) return 0;

	return pRes->dep->ops->lock(pRes);

}


