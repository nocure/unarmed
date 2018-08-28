/******************************************************************************
 * Basic type & macro defines
 * ============================================================================
 *
 * 2008-0708 wsw created
 *
 * 2008-0911 added endian conversion macros & type check for min()
 *
 *****************************************************************************/
#ifndef YL_TYPE_H_
#define YL_TYPE_H_

#define DBGOUT_ON

// ============================================================================
//#include <wchar.h>
#include <ctype.h>

typedef unsigned char 		BYTE;
typedef unsigned short 		WORD;
typedef unsigned int 		DWORD;
typedef unsigned long long	QWORD;

typedef unsigned char		uint8;
typedef char				int8;
typedef unsigned short		uint16;
typedef short				int16;
typedef unsigned int		uint32;
typedef int					int32;
typedef unsigned long long	uint64;
typedef signed long long	int64;
#if 0
typedef unsigned long long	uint64_t;
typedef signed long long 	int64_t;
typedef unsigned int		uint32_t;
typedef signed int			int32_t;
typedef unsigned short		uint16_t;
typedef signed short		int16_t;
typedef unsigned char		uint8_t;
typedef signed char			int8_t;
#endif

typedef wchar_t				ucs4_t; // 16-bit UCS type
typedef unsigned short		ucs2_t; // 16-bit UCS type
typedef long				off_t;
typedef long long			off64_t;
typedef long				time_t;
typedef unsigned int		mode_t;

typedef unsigned char		U8;
typedef unsigned char		u8;
typedef unsigned short		U16;
typedef unsigned short		u16;
typedef unsigned int		U32;
typedef unsigned int		u32;
typedef signed int			S32;
typedef signed int			s32;

typedef unsigned char		UCHAR;
typedef unsigned long		ULONG;
typedef unsigned int		UINT;

#define FALSE           	0
#define TRUE            	1

#ifndef NULL
#define NULL				((void*)0)
#endif

#ifndef EOF
#define EOF					-1
#endif

#define REG32(a)	(*((volatile DWORD*)(a)))
#define REG16(a)	(*((volatile WORD*)(a)))
#define REG8(a)		(*((volatile BYTE*)(a)))

#define BIT_SET(var,bits) (var)|= (bits)
#define BIT_CLR(var,bits) (var)&=~(bits)
#define BIT_AND(var,bits) (var)&= (bits)
#define BIT_VAL(var,bits) ((var)&(bits))

#ifndef min
#define min(x,y) ({ \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
        (void) (&_x == &_y); \
        _x < _y ? _x : _y; })
#endif

#ifndef max
#define max(x,y) ({ \
        typeof(x) _x = (x); \
        typeof(y) _y = (y); \
        (void) (&_x == &_y); \
        _x > _y ? _x : _y; })
#endif

#define ASC2BIN(a)	((a >= '0' && a <= '9') ? (a - '0') : (a - 'A' + 10))

#define HIWORD(a)       (((DWORD)(a)) >> 16)
#define LOWORD(a)       (((DWORD)(a)) & 0xFFFF)

#define UNUSED_VARIABLE(expr) do { (void)(expr); } while (0)

// ==== endian conversion =========================
#define swab16(x) \
        ((WORD)( \
                (((WORD)(x) & (WORD)0x00ffU) << 8) | \
                (((WORD)(x) & (WORD)0xff00U) >> 8) ))
#define swab32(x) \
        ((DWORD)( \
                (((DWORD)(x) & (DWORD)0x000000ffUL) << 24) | \
                (((DWORD)(x) & (DWORD)0x0000ff00UL) <<  8) | \
                (((DWORD)(x) & (DWORD)0x00ff0000UL) >>  8) | \
                (((DWORD)(x) & (DWORD)0xff000000UL) >> 24) ))

#define cpu_to_le16(x) ({ WORD _x = x; swab16(_x); })
#define cpu_to_le32(x) ({ DWORD _x = x; swab32(_x); })
#define le32_to_cpu(x) cpu_to_le32(x)
#define le16_to_cpu(x) cpu_to_le16(x)

#define isdelim(c)	((c&0xff)=='/')
#define	wisspace(c)	((c<0x100)&&isspace(c))
#define wisdelim(c)	((c<0x100)&&isdelim(c))
#define	wisalpha(c)	((c<0x100)&&isalpha(c))

struct tm {
  int	tm_sec;
  int	tm_min;
  int	tm_hour;
  int	tm_mday;
  int	tm_mon;
  int	tm_year;
  int	tm_wday;
//  int	tm_yday;
//  int	tm_isdst;
};


// ==== Alignment ===========================
#define ALIGN4(p) ((p+0x03)&(0xfffffffc))
#define ALIGN8(p) ((p+0x07)&(0xfffffff8))
#define ALIGN16(p) ((p+0x0f)&(0xfffffff0))


// ==== Q-point (float-point in fixed point varable) =========================
#define QSHIFT(n)			(1 << (n))

#define Qninc( Qval, n, m ) ((Qval) << ((m) - (n)))	// Qninc(val,8,9): to convert Q8 val to Q9
#define Qndec( Qval, m, n ) ((Qval) >> ((m) - (n)))	// Qndec(val,9,8): to convert Q9 val to Q8

#define mulQn( Val, Qval, n ) (((Val) * (Qval)) >> (n))
#define divQn( Val, Qval, n ) (((Val) << (n)) / (Qval))

#define Qn2double(  Qval, n ) ( (double) (Qval) / QSHIFT(n)  )
#define double2U32Qn( db, n ) ( (U32)   ((db)   * QSHIFT(n)) )
#define double2S32Qn( db, n ) ( (S32)   ((db)   * QSHIFT(n)) )

// ==== Q8 ====
typedef U32						U32Q8;
typedef S32						S32Q8;

#define U32Q8_1 				((U32Q8) QSHIFT(8))
#define S32Q8_1 				((S32Q8) QSHIFT(8))

#define mulQ8( Val, Qval ) 		mulQn( Val, Qval, 8 )
#define divQ8( Val, Qval ) 		divQn( Val, Qval, 8 )
#define Q8DBL( Qval )			Qn2double(  Qval, 8 )
#define U32Q8( db )				double2U32Qn( db, 8 )
#define S32Q8( db )				double2S32Qn( db, 8 )


#endif /*YL_TYPE_H_*/
