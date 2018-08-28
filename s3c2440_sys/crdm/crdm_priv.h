/*
 * crdm_priv.h
 *
 *  Created on: 2009/4/21
 *      Author: Wesley Wang
 */

#ifndef CRDM_PRIV_H_
#define CRDM_PRIV_H_

#define MAX_SUPPORTED_DEPOT	256

#define DEPOT_TYPE_MEMORY	1
#define DEPOT_TYPE_FILE		2
#define DEPOT_TYPE_NAND		3

#define DEPOT_FLAG_VALID	(1<<0)

#define RES_ATTR_INDEXED	(1<<0)			// indicates that the resource has an index table
#define RES_ATTR_LOCKED		(1<<1)			// resource has been locked in memory, special care is required when closing
#define RES_OFFSET_MASK		0xfffffffc	// offset is always 4 byte aligned, lower 2 bits are attributes

#define DEPOT_ID_SHIFT		24
#define RES_ID_MASK			0x00ffffff

#define NUM_CHUNK_CACHE		64

typedef struct _CRD_INDEX_T {
	DWORD offset;		// offset of the resource in CRD, from beginning of CRD
	DWORD length;		// total length of the resource
} CRD_INDEX_T;


//struct _FILE;
typedef union {
    struct {
        DWORD baseaddr;
        CRD_INDEX_T* index;
    };
    struct {
//        struct _FILE *fp;
    	FILE *fp;
        CRD_INDEX_T	 *index1;
        void		 *fillbuf;
    };
    struct {
    	DWORD startblock;
    };
} DEPOT_PRIVATE_T;


struct _DEPOT_OPS_T;
typedef struct _DEPOT_INFO_T {
	DWORD type;
	DWORD numres;
	DWORD chunksize;
	DWORD flags;

	struct _DEPOT_OPS_T *ops;
	DEPOT_PRIVATE_T priv;
} DEPOT_INFO_T;


typedef struct _RES_INFO_T {
	DWORD resid;		// resource id
	int pos;			// current pointer
	DWORD size;
	DWORD attr;			// attribute
	DWORD numchunks;

	DWORD chunks;		// chunk offset table

	DEPOT_INFO_T *dep;
	DEPOT_PRIVATE_T priv;
} RES_INFO_T;


typedef struct _CHUNK_CACHE_T {
	DWORD resid;			// resource id
	DWORD cid;			// chunk number, units in chunk size;
	void* pointer;
	struct _CHUNK_CACHE_T *next;
} CHUNK_CACHE_T;


typedef struct _DEPOT_OPS_T {
	RES_INFO_T* (*open) (DEPOT_INFO_T *pDep, DWORD idx);
	int (*read) (RES_INFO_T *pRes, BYTE* buf, DWORD size);
	int (*seek) (RES_INFO_T *pRes, int off, DWORD where);
	void (*close) (RES_INFO_T *pRes);
	void* (*lock) (RES_INFO_T *pRes);
} DEPOT_OPS_T;


typedef struct _CRD_HEADER_T {
	BYTE sig0;			// signature char 0
	BYTE sig1;			// signature char 1
	BYTE sig2;			// signature char 2
	BYTE depid;			// Depot ID
	DWORD numres;		// number of resource
	DWORD depsize;		// total size of depot file
	DWORD chunksize;	// chunk size
} CRD_HEADER_T;


#define SIGNATURE_CHAR0		'C'
#define SIGNATURE_CHAR1		'R'
#define SIGNATURE_CHAR2		'D'

unsigned long hufDecode(unsigned char *huf,unsigned char *dat);

void crdAddCache(CHUNK_CACHE_T* pcc);
void crdReturnCache(CHUNK_CACHE_T* pcc);
int crdFreeResCache(DWORD resid);
CHUNK_CACHE_T * crdFindChunkInCache(DWORD resid, DWORD cid);
CHUNK_CACHE_T* crdGetFreeCache(void);

DEPOT_INFO_T* crdGetDepot(DWORD number);

#endif /* CRDM_PRIV_H_ */
