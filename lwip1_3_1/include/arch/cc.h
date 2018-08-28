/*
 * cc.h
 *
 *  Created on: 2009/8/29
 *      Author: ww
 */

#ifndef CC_H_
#define CC_H_

#include <platform/yl_sys.h>

#define BYTE_ORDER LITTLE_ENDIAN
#define LWIP_PROVIDE_ERRNO

typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned int u32_t;
typedef signed int s32_t;
typedef unsigned int mem_ptr_t;

// non-fatal, print a message.
#define LWIP_PLATFORM_DIAG(x) {yl_uartPrintf x;}

// fatal, print message and abandon execution.
#define LWIP_PLATFORM_ASSERT(x)	{yl_uartPrintf("ASSERT: line %d in %s\n", __LINE__, __FILE__);yl_uartPrintf(x);yl_uartPrintf("\n");while(1);}

#define U16_F "u"
#define S16_F "d"
#define X16_F "04x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "08x"
#define SZT_F "d"

#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END




#define SYS_ARCH_DECL_PROTECT(x) YL_PSR_SAVE_AREA
#define SYS_ARCH_PROTECT(x) XDISABLE
#define SYS_ARCH_UNPROTECT(x) XRESTORE

#endif /* CC_H_ */
