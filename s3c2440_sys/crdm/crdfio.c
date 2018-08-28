/*
 * crdfio.c
 *
 *  Created on: 2009/4/24
 *      Author: Chris Chiu
 */

#include "platform/YL_SYS.h"
#include <platform/yl_fsio.h>
#include <string.h>
#include "crdm_priv.h"

static RES_INFO_T* crdFileOpen(DEPOT_INFO_T *pDep, DWORD rid) {
RES_INFO_T *pRes;
DWORD resofs;
DWORD reslen;
int numchunks,i;
DWORD chunktbl_size;
WORD *chunksize_tbl = NULL;
CRD_INDEX_T *cidx = NULL;
DWORD idx;

	pRes=(RES_INFO_T*)malloc(sizeof(RES_INFO_T));
	if (!pRes) return 0;
	memset(pRes,0,sizeof(RES_INFO_T));

	idx=rid&RES_ID_MASK;

	// get resource infomation from index table
	resofs=pDep->priv.index[idx].offset;
	reslen=pDep->priv.index[idx].length;

	printf("rid: %08x, resofs: %d reslen: %d\n", rid, resofs, reslen);

	if (BIT_VAL(resofs,RES_ATTR_INDEXED)) {
		BIT_SET(pRes->attr,RES_ATTR_INDEXED);
	}

	resofs=resofs&RES_OFFSET_MASK;

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {
		numchunks=reslen/pDep->chunksize;
		if (reslen%pDep->chunksize) numchunks++;
		chunktbl_size=ALIGN4(numchunks*sizeof(WORD));	// 4byte aligned to allocate memory, may waste few bytes but malloc will make it so too anyway...
		chunksize_tbl=(WORD *)malloc(chunktbl_size);
		if (!chunksize_tbl) goto err1;
		fseek(pDep->priv.fp, resofs, SEEK_SET);
		fread(chunksize_tbl, chunktbl_size, pDep->priv.fp);

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
		cidx = malloc(sizeof(CRD_INDEX_T));
		cidx->length = reslen;
		cidx->offset = resofs;
		pRes->chunks=(DWORD)cidx;
		numchunks=0;
	}

	pRes->resid=rid;
	pRes->dep=pDep;
	pRes->pos=0;		// reset current pointer
	pRes->size=reslen;
	pRes->numchunks=numchunks;

	if (chunksize_tbl)	free(chunksize_tbl);
	return pRes;
err1:
	free(chunksize_tbl);
	free(cidx);
	free(pRes);
	return 0;
}


static int crdFileRead (RES_INFO_T *pRes, BYTE* buf, DWORD size) {
DEPOT_INFO_T *pDep;
DWORD byteleft;
DWORD this_read;
CHUNK_CACHE_T *pCache;
CRD_INDEX_T *cidx;

	pDep=pRes->dep;

	if ((pRes->pos+size)>pRes->size) {	// exceed end of file, adjust length to read
		byteleft=pRes->size=pRes->pos;
	}
	else {
		byteleft=size;
	}

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// indexed
	DWORD cnum;
	int cptr;
	DWORD uncmpz_size;
	DWORD actual_chunk_size;
	BYTE *cmpz_buf;
	DWORD dstofs;

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
				fseek(pDep->priv.fp, cidx[cnum].offset+cptr, SEEK_SET);
				fread(buf+dstofs, this_read, pDep->priv.fp);
			}
			else {	// this chunk is compressed, cache and uncompress !

				// is it in cache already ?
				pCache=crdFindChunkInCache(pRes->resid,cnum);
				if (!pCache) {	// not in cache, cache it
					BYTE	*srcbuf = (BYTE *)malloc(cidx[cnum].length);
					if (!srcbuf) return 0;
					pCache=crdGetFreeCache();
					if (!pCache) return 0;

					cmpz_buf=malloc(actual_chunk_size);
					if (!cmpz_buf) return 0;

					fseek(pDep->priv.fp, cidx[cnum].offset, SEEK_SET);
					fread(srcbuf, cidx[cnum].length, pDep->priv.fp);
					//uncmpz_size=hufDecode((unsigned char *)cidx[cnum].offset,cmpz_buf);
					uncmpz_size=hufDecode(srcbuf, cmpz_buf);
					free(srcbuf);	/* release the storage for compressed data */
#if 1
					printf("cnum: %d uncmpx size: %d offset %d length %d\n",
							cnum, uncmpz_size, cidx[cnum].offset, cidx[cnum].length);
#endif
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
#if 1
		int		i, cptr;
		DWORD 	cnum, total_chunks;
		DWORD 	actual_chunk_size;
		size_t  flat_size;
		BYTE	*cached_buf = NULL;
		DWORD 	dstofs;

		cidx=(CRD_INDEX_T*)pRes->chunks;
		total_chunks = cidx->length / pDep->chunksize;
		cnum=pRes->pos/pDep->chunksize;
		cptr=pRes->pos%pDep->chunksize;
		dstofs = 0;

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

			pCache=crdFindChunkInCache(pRes->resid,cnum);
			if (!pCache) {
				fseek(pDep->priv.fp, cidx->offset, SEEK_SET);
				flat_size = fread(buf, cidx->length, pDep->priv.fp);
#if 1
				printf("fread size %d byte total len %d\n", flat_size, cidx->length);
#endif
				if (flat_size != cidx->length) return 0;

				cached_buf = malloc(flat_size);
				memcpy(cached_buf, buf, flat_size);

				for (i = 0; i < total_chunks; i ++) {
					pCache=crdGetFreeCache();
					if (!pCache) return 0;
					pCache->pointer=cached_buf + (i * pDep->chunksize);
					pCache->resid=pRes->resid;
					pCache->cid=i;
					crdAddCache(pCache);
				}
				buf += pRes->pos;
				byteleft = 0;
			}
			else {
				memcpy(buf+dstofs,(pCache->pointer + cptr), this_read);
				byteleft-=this_read;
				dstofs+=this_read;
				cnum++;
				cptr=0;
			}
		}
#else
		cidx=(CRD_INDEX_T*)pRes->chunks;
		fseek(pDep->priv.fp, cidx->offset, SEEK_SET);
		fread(buf, byteleft, pDep->priv.fp);
		byteleft=0;
#endif
	}

	this_read=size-byteleft;
	pRes->pos+=this_read;

	return this_read;
}


static void crdFileClose (RES_INFO_T *pRes) {
DEPOT_INFO_T *pDep = pRes->dep;

	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// it's indexed, so it has index table allocated.
		free((void*)pRes->chunks);
	}

	fclose(pDep->priv.fp);
	// free the structure
	free(pDep->priv.index);
	if (pDep->priv.fillbuf)  free(pDep->priv.fillbuf);
	free((void *)pRes->chunks);
	free(pRes);
	return;
}

static void* crdFileLock (RES_INFO_T *pRes) {
DEPOT_INFO_T *pDep;
CRD_INDEX_T  *cidx;

	pDep=pRes->dep;
	// TODO: support locking indexed resource
	if (BIT_VAL(pRes->attr,RES_ATTR_INDEXED)) {	// it's indexed, can't be locked currently
		return 0;
	}

	BIT_SET(pRes->attr,RES_ATTR_LOCKED);

	cidx=(CRD_INDEX_T*)pRes->chunks;
	fseek(pDep->priv.fp, cidx->offset, SEEK_SET);
	pDep->priv.fillbuf = malloc(pRes->size);
	fread(pDep->priv.fillbuf, pRes->size, pDep->priv.fp);

	return (void*)pDep->priv.fillbuf;
}


static DEPOT_OPS_T crdfilops={
	.open=crdFileOpen,
	.read=crdFileRead,
	.seek=0,
	.close=crdFileClose,
	.lock=crdFileLock,
};


// =========================================================


int crdRegisterFile(char* crdFname) {
CRD_HEADER_T *head = NULL;
DEPOT_INFO_T *pDep = NULL;
FILE		 *crdfp = NULL;
BYTE		 buf[32];
DWORD		 idxtbl_size = 0;

	crdfp = fopen(crdFname, "r");
	if (!crdfp) return -1;
	fread(buf, sizeof(CRD_HEADER_T), crdfp);
	head=(CRD_HEADER_T*)buf;
	if ((head->sig0!=SIGNATURE_CHAR0)||(head->sig1!=SIGNATURE_CHAR1)||(head->sig1!=SIGNATURE_CHAR1)) return -1;
	idxtbl_size = (head->numres + 1) * sizeof(CRD_INDEX_T);
	pDep=crdGetDepot(head->depid);
	if (BIT_VAL(pDep->flags,DEPOT_FLAG_VALID)) return -1;	// depid is already in used

	pDep->chunksize=head->chunksize;
	pDep->numres=head->numres;
	pDep->type=DEPOT_TYPE_FILE;
	pDep->priv.fp=crdfp;

	pDep->priv.index = (CRD_INDEX_T *)malloc(idxtbl_size);
	if (!pDep->priv.index)	return -1;	/* malloc fail */
	fread(pDep->priv.index, idxtbl_size, crdfp);
	pDep->ops=&crdfilops;
	BIT_SET(pDep->flags,DEPOT_FLAG_VALID);

	return 0;
}


