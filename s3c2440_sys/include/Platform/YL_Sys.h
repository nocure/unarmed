/*
 * yl_sys.h
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */

#ifndef yl_sys_H_
#define yl_sys_H_

#include <stdlib.h>
#include <string.h>
#include "yl_oswrap.h"

/******************************************************************************
 * System functions
 * ============================================================================
 *
 * 2008-0918 wsw created
 * 		added some YL platform provided system calls
 * 		moved time marco here
 *
 * 2008-0915 added time comparison macros using uItron get_tim
 * 2008-0918 added systick
 *
 *****************************************************************************/

#define __LUSIMUS__
//#define OS_MEM_MGR		// use OS memory management

// ==== time comparison macros ===========================
// OSTICK --> uItron internal tick, mili-second, 32bit width only
// gets overflowed quickly
#define OSTICK get_tim()

#define typecheck(type,x) ({ \
	type __dummy; \
	typeof(x) __dummy2; \
	(void)((type*)&__dummy == (type*)&__dummy2); \
	1; \
})

#define time_after(a,b) ( \
    typecheck(unsigned long, a) && \
     typecheck(unsigned long, b) && \
     ( ((long)(b) - (long)(a)) < 0) \
)

#define time_before(a,b)        time_after(b,a)

// SYSTICK --> System maintained tick, 50 mili-second resolution, 64bit width
// can't use time_after & time_before, these are 32-bit
QWORD tq_timerGetTimerTick(void);
#define SYSTICK tq_timerGetTimerTick()

#if __GNUC__ > 4

#ifndef strcmpi
#define strcmpi strcasecmp
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strncmpi
#define strncmpi strncasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#endif

// ==== utils/ucon.c =================================
typedef void (*CMDPROC)( int, char ** );

typedef struct _UCON_ARG_CMD {
	CMDPROC func;			// Pointer to func
	const char*   cmd;		// Pointer to command string
	const char*   desc;		// Pointer to description
} UCON_ARG_CMD;

void uconLineReset(void);
int uconInputCheck(void);
char* uconGetCmdBuf(void);
int uconExec(void);
char* uconAsk(void);


// ==== task lpi
/**@brief 傳回 LPI 所需配置空間
 *
 * 每個 Task 均會紀錄自身之當前路徑，為Task's local path infomation 將於 task creation 時配置，依照搭配之檔案系統不同，系統需提供所需空間以供作業系統配置。
 * @ref cre_tsk 提供選項是否要配置 LPI
 *
 * @return 所需空間 (Bytes)
 */
inline int lpiSize(void);


/**@brief 取得 Task 當前工作路徑
 *
 * @return File system 當前工作路徑，型態取決於使用之 File system。
 */
inline void* lpiGetCWD();


/**@brief 取得 Task 當前使用中之 File system handle
 *
 * File system handle 之型態及是否需要此 handle 取決於所使用之檔案系統架構。
 *
 * @return File system handle 指標
 */
inline void* lpiGetFSHandle();


/**@brief 設定 Task 當前使用中之 File system handle
 *
 * File system handle 之型態及是否需要此 handle 取決於所使用之檔案系統架構。
 *
 * @param ptr system handle 指標
 */
inline void lpiSetFSHandle(void* ptr);

/**@} GROUP_SYS_SERVICE *********************************************************************/



// ==== utils/string-util.c =================================
char *yl_strdup(const char *str);

ucs2_t* yl_wconcat(ucs2_t* absPath,const ucs2_t* relPath);
ucs2_t* yl_wstrcpy(ucs2_t* dst,const ucs2_t* src);
ucs2_t *yl_wstrdup(const ucs2_t *str);
int yl_wstrlen(const ucs2_t* uni);
int yl_strtoaddr(char* str,DWORD *value);
int yl_strtol(char* str,int *value);
int yl_strtoul(char* str,DWORD *value);
double yl_strtod(const char *str, char **endptr);

#define strtod  yl_strtod
#define atof(x) yl_strtod(x,0)

#define	strdup	yl_strdup

#define wcscat	yl_wconcat
#define	wcslen	yl_wstrlen
#define wcscpy	yl_wstrcpy
#define wcsdup	yl_wstrdup
// ==== utils/getopt.c =================================
int yl_getopt(int argc, char **argv, char *optionS);
extern char *yl_optarg;
extern int yl_optind;

#define getopt yl_getopt
#define optarg yl_optarg
#define optind yl_optind


// ==== utils/printf-stdarg.c =================================
int yl_uartPrintf(const char *szFmt, ...);
int yl_sprintf(char *out, const char *format, ...);
int tq_uartPutChar(DWORD c);
int tq_uartGetChar(void);

#ifdef DBGOUT_ON
#define dbgout				yl_uartPrintf
#else
#define dbgout(...)
#endif

#define printf				yl_uartPrintf
#define sprintf				yl_sprintf
#define putch(c)			({int x=c; if (x=='\n') tq_uartPutChar('\r');tq_uartPutChar(x);}) // ww 2014-0123 added variable x.2014-0121 putch is modified to interpret \n as \r\n
#define getch				tq_uartGetChar

// ==== utils/malloc.c ======================================
#ifdef OS_MEM_MGR
void* tx_os_malloc(DWORD size);
void* tx_os_calloc(DWORD num, DWORD size);
void* tx_os_realloc(void *mptr, DWORD size);
void tx_os_free(void* memptr);
DWORD tx_os_mleft(void);

#define malloc				tx_os_malloc
#define realloc				tx_os_realloc
#define calloc				tx_os_calloc
#define free				tx_os_free
#define mleft				tx_os_mleft
#endif

// ==== stdlib.h ===========================================
void srand(unsigned __seed);
int rand(void);
unsigned long strtoul(const char *__n, char **__end_PTR, int __base);


// ==== util/memmgr.c =======================================
#ifndef OS_MEM_MGR
/** @brief 記憶體狀態結構
 *
 * @sa memGetStatus
 */
typedef struct {
	DWORD used;		///< 已使用之記憶體(byte)
	DWORD segs;		///< 非連接的記憶體區塊數量(片段)
	DWORD largest;	///< 最大的自由記憶體區塊
	DWORD mcbs;		///< 記憶體控制區塊(Memory Control Block)的數量
} MEMORY_STATUS_T;


/**@brief 初始化記憶體功能
 *
 * 記憶體將會由記憶體管理程式負責配置使用，程式若覆寫此記憶體空間，將損毀配置紀錄區塊，造成記憶體管理程式停止運作
 *
 * @param base 空白的記憶體空間
 * @param size 此記憶體空間的大小
 */
void memInit(DWORD base, DWORD size);


/**@fn void* malloc(DWORD size_requested);
 * @brief 要求配置記憶體空間
 *
 * @param size_requested 所要求的記憶體空間大小
 *
 * @return 配置好的記憶體空間位址
 * @cond INTERNAL_USE
 */
void* memAlloc(DWORD size_requested);
#define malloc				memAlloc
///@endcond


/**@fn void* realloc(void* memory, DWORD size_requested);
 * @brief 重新配置舊的記憶體空間
 *
 * @param memory 此記憶體空間的所在位址
 * @param size_requested 要求重新配置的記憶體空間大小
 *
 * @return 配置好的記憶體空間位址
 * @cond INTERNAL_USE
 */
void* memRealloc(void* memory, DWORD size_requested);
#define realloc				memRealloc
///@endcond


/**@fn void free(void* memory);
 * @brief 釋放所指定的記憶體空間
 *
 * @param memory 欲釋放之記憶體空間的所在位址
 * @cond INTERNAL_USE
 */
void memFree(void* memory);
#define free				memFree
///@endcond


/**@fn void *malloca(DWORD size, int alignbits);
 * @brief 向系統要求配置對齊位址之記憶體空間
 *
 * @param size 欲配置之記憶體大小
 * @param alignbits 限制記憶體位址必須要對齊在alignbits上(0~31 bit)
 *
 * @return 配置好的記憶體空間位址
 * @cond INTERNAL_USE
 */
void *memAlignedAlloc(DWORD size_requested, int alignbits);
#define malloca				memAlignedAlloc
///@endcond


/**@fn void *mallocr(DWORD size, int alignbits);
 * @brief 向系統要求配置記憶體空間，並且由足夠大 size_requested 之最小自由空間中底端配置
 *
 * @param size_requested 欲配置之記憶體大小
 *
 * @return 配置好的記憶體空間位址
 * @cond INTERNAL_USE
 */
void* memRevAlloc(DWORD size_requested);
#define mallocr				memRevAlloc
///@endcond


/**@brief 取得目前記憶體之狀態
 *
 * @param pms 呼叫者提供 @ref MEMORY_STATUS_T 型態之結構指標，或 null 不使用。
 *
 * @retval pms memGetStatus 將會填入記憶體相關資訊。
 *
 * @return 剩餘未使用之記憶體空間。
 */
DWORD memGetStatus(MEMORY_STATUS_T* pms);


/**@fn void* calloc(DWORD num, DWORD size);
 * @brief 動態配置記憶體空間, 並將配置後的空間資料全都設為0
 *
 * @param num 欲配置的區塊個數
 * @param size 欲配置區塊個數的大小
 *
 * @return 配置完成之記憶體位址
 *
 * @sa malloc, realloc
 * @cond INTERNAL_USE
 */
#define calloc(num,size)			({ \
	register void* p; \
	register unsigned long t=num*size; \
	p=malloc(t); \
	if (p) memset(p,0,t); \
	p; \
})
///@endcond


#endif

/**@} GROUP_SYS_LIBC *********************************************************************/


// ==== Driver Task message structure =========================
typedef struct drvmsg_struct {
	void* tskid;			///< The task id of caller
	void *mptr;				///< The media pointer of file system to be processed
} DRVMSG, *DRVMSG_PTR;


// ==== Central Resource Depot Management =======================
typedef DWORD HRES;
#define CRD_SEEKSET			0
#define CRD_SEEKCUR			1
#define CRD_SEEKEND			2


int crdRegisterFile(char* file);
int crdRegisterMem(const BYTE* addr);
int crdRegisterNAND(int startBlock);
int crdInit(void);
HRES crdOpen(DWORD resid);
int crdRead(HRES hRes, BYTE* buf, DWORD size);
int crdSeek(HRES hRes, int off, DWORD where);
int crdTell(HRES hRes);
int crdEOF(HRES hRes);
void crdClose(HRES hRes);
void* crdLock(HRES hRes);
DWORD crdSize(HRES hRes);


// ==== mem trace ====
void istracemem(void* p);
void untracemem(void* p);
void tracemem(void* p);


// ============================================================================================================================
// ============================================================================================================================
// ============================================================================================================================
#define ASSERT(s,a)	{if (!(a)) {int yl_uartPrintf(const char *format, ...);YL_PSR_SAVE_AREA;XDISABLE;yl_uartPrintf("**ASSERT**\n%s\n",s);while(1);XRESTORE;}}


#endif /* yl_sys_H_ */
