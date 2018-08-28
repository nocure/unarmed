/*
 * crdmem.c
 *
 *  Created on: 2009/4/20
 *      Author: Wesley Wang
 */
#include "platform/YL_SYS.h"
#include <platform/yl_fsio.h>
#include <string.h>
#include "crdm_priv.h"

static RES_INFO_T* crdmemOpen(DEPOT_INFO_T *pDep, DWORD rid) {
RES_INFO_T *pRes;
DWORD resofs;
DWORD reslen;
int numchunks,i;
DWORD chunktbl_size;
WORD *chunksize_tbl;
CRD_INDEX_T *cidx;
DWORD idx;

	pRes=(RES_INFO_T*)malloc(sizeof(RES_INFO_T));
	if (!pRes) return 0;
	memset(pRes,0,sizeof(RES_INFO_T));

	idx=rid&RES_ID_MASK;

	// get resource infomation from index table
	resofs=pDep->priv.index[idx].offset;
	reslen=pDep->priv.index[idx].length;

	if (BIT_VAL(resofs,RES_ATTR_INDEXED)) {
		BIT_SET(pRes->attr,RES_ATTR_INDEXED);
	}

	resofs=(resofs&RES_OFFSET_MASK)+pDep->priv.baseaddr;

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {
		chunksize_tbl=(WORD*)resofs;
		numchunks=reslen/pDep->chunksize;
		if (reslen%pDep->chunksize) numchunks++;
		chunktbl_size=ALIGN4(numchunks*sizeof(WORD));	// 4byte aligned to allocate memory, may waste few bytes but malloc will make it so too anyway...

		cidx=(CRD_INDEX_T*)malloc(chunktbl_size*sizeof(CRD_INDEX_T));
		if (!cidx) goto err1;

		resofs+=chunktbl_size;
		for (i=0;i<numchunks;i++) {
			cidx[i].offset=resofs;
			cidx[i].length=chunksize_tbl[i];
			resofs+=chunksize_tbl[i];
		}

		pRes->chunks=(DWORD)cidx;
	}
	else  {
		pRes->chunks=resofs;
		numchunks=0;
	}

	pRes->resid=rid;
	pRes->dep=pDep;
	pRes->pos=0;		// reset current pointer
	pRes->size=reslen;
	pRes->numchunks=numchunks;

	return pRes;
err1:
	free(pRes);
	return 0;
}


static int crdmemRead (RES_INFO_T *pRes, BYTE* buf, DWORD size) {
DEPOT_INFO_T *pDep;
DWORD byteleft;
DWORD this_read;
CHUNK_CACHE_T *pCache;
DWORD cnum;
CRD_INDEX_T *cidx;
int cptr;
DWORD uncmpz_size;
DWORD actual_chunk_size;
BYTE *cmpz_buf;
DWORD dstofs;

	pDep=pRes->dep;

	if ((pRes->pos+size)>pRes->size) {	// exceed end of file, adjust length to read
		byteleft=pRes->size-pRes->pos;
	}
	else {
		byteleft=size;
	}

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// indexed

		cidx=(CRD_INDEX_T*)pRes->chunks;
		cnum=pRes->pos/pDep->chunksize;
		cptr=pRes->pos%pDep->chunksize;
		dstofs=0;

		while(byteleft) {

			if (cnum==(pRes->numchunks-1)) {	// this is last chunk, take special care, calculate chunk length
				actual_chunk_size=pRes->size-(cnum*pDep->chunksize);
			}
			else {
				actual_chunk_size=pDep->chunksize;
			}

			if ((cptr+byteleft)>actual_chunk_size) { // more then a chunk
				this_read=actual_chunk_size-cptr;
			}
			else {
				this_read=byteleft;
			}
			if (cidx[cnum].length==ALIGN4(actual_chunk_size)) {	// this chunk is uncompressed
				memcpy(buf+dstofs,(void*)(cidx[cnum].offset+cptr),this_read);
			}
			else {	// this chunk is compressed, cache and uncompress !

				// is it in cache already ?
				pCache=crdFindChunkInCache(pRes->resid,cnum);
				if (!pCache) {	// not in cache, cache it
					pCache=crdGetFreeCache();
					if (!pCache) return 0;

					cmpz_buf=malloc(actual_chunk_size);
					if (!cmpz_buf) return 0;

					uncmpz_size=hufDecode((unsigned char *)cidx[cnum].offset,cmpz_buf);
					if (uncmpz_size!=actual_chunk_size) {	// error!
						free(cmpz_buf);
						crdReturnCache(pCache);
						return 0;
					}
					else {
						pCache->pointer=cmpz_buf;
						pCache->resid=pRes->resid;
						pCache->cid=cnum;
						crdAddCache(pCache);
					}
				}
				memcpy(buf+dstofs,(pCache->pointer+cptr),this_read);
			}
			byteleft-=this_read;
			dstofs+=this_read;
			cnum++;
			cptr=0;
		}

	}
	else { // flat, non-indexed
		// TODO:  fix
		memcpy(buf,(void*)pRes->chunks,byteleft);
		byteleft=0;
	}

	this_read=size-byteleft;
	pRes->pos+=this_read;

	return this_read;
}


static void crdmemClose (RES_INFO_T *pRes) {

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// it's indexed, so it has index table allocated.
		free((void*)pRes->chunks);
	}

	// free the structure
	free(pRes);
	return;
}

static void* crdmemLock (RES_INFO_T *pRes) {

	// TODO: support locking indexed resource
	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// it's indexed, can't be locked currently
		return 0;
	}

	BIT_SET(pRes->attr,RES_ATTR_LOCKED);

	return (void*)pRes->chunks;
}


static DEPOT_OPS_T crdmemops={
	.open=crdmemOpen,
	.read=crdmemRead,
	.seek=0,
	.close=crdmemClose,
	.lock=crdmemLock,
};


// =========================================================


int crdRegisterMem(const BYTE* addr) {
CRD_HEADER_T *head;
DEPOT_INFO_T *pDep;

	head=(CRD_HEADER_T*)addr;
	if ((head->sig0!=SIGNATURE_CHAR0)||(head->sig1!=SIGNATURE_CHAR1)||(head->sig1!=SIGNATURE_CHAR1)) return -1;
	pDep=crdGetDepot(head->depid);
	if (BIT_VAL(pDep->flags,DEPOT_FLAG_VALID)) return -1;	// depid is already in used

	pDep->chunksize=head->chunksize;
	pDep->numres=head->numres;
	pDep->type=DEPOT_TYPE_MEMORY;
	pDep->priv.baseaddr=(DWORD)addr;
	pDep->priv.index=(CRD_INDEX_T*)(addr+sizeof(CRD_HEADER_T));
	pDep->ops=&crdmemops;
	BIT_SET(pDep->flags,DEPOT_FLAG_VALID);

	return 0;
}


