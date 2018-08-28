/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003-2005 M. Bakker, Nero AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** The "appropriate copyright message" mentioned in section 2c of the GPLv2
** must read: "Code from FAAD2 is copyright (c) Nero AG, www.nero.com"
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Nero AG through Mpeg4AAClicense@nero.com.
**
** $Id: cfft.h,v 1.24 2007/11/01 12:33:29 menno Exp $
**/

#ifndef __CFFT_H__
#define __CFFT_H__

#if defined(__i386__)
#include <stdint.h>
#elif defined(__ARM_EABI__)	// // ww 2018-0810 added for arm_none_eabi_gcc that has int32_t defined in stdint.h
#include <stdint.h>
#else
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#if !defined(HAVE_INT32_T)
typedef signed int int32_t;
#define HAVE_INT32_T
#endif
typedef signed short int16_t;
typedef signed char int8_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define ALIGN
#define FIXED_POINT

#define INLINE __inline
#define faad_malloc malloc
#define faad_free free


#define REAL_BITS 14 // MAXIMUM OF 14 FOR FIXED POINT SBR
#define REAL_PRECISION (1 << REAL_BITS)
#define RTOD(x) (((double)x)/REAL_PRECISION)

#define FRAC_SIZE 32 /* frac is a 32 bit integer */
#define FRAC_BITS 31
#define FRAC_PRECISION ((uint32_t)(1 << FRAC_BITS))
#define FRAC_MAX 0x7FFFFFFF


#define REAL_CONST(A) (((A) >= 0) ? ((real_t)((A)*(REAL_PRECISION)+0.5)) : ((real_t)((A)*(REAL_PRECISION)-0.5)))
#define FRAC_CONST(A) (((A) == 1.00) ? ((real_t)FRAC_MAX) : (((A) >= 0) ? ((real_t)((A)*(FRAC_PRECISION)+0.5)) : ((real_t)((A)*(FRAC_PRECISION)-0.5))))

#define _MulHigh(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+((int64_t)1 << (FRAC_SIZE-1))) >> FRAC_SIZE)

#define _MulFix(A,B) (int64_t)((  (int64_t)(A)*(int64_t)(B)  ) >> REAL_BITS)

/* multiply with real shift */
#define MUL_R(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+((int64_t)1 << (REAL_BITS-1))) >> REAL_BITS)

#define MUL_F(A,B) (real_t)(((int64_t)(A)*(int64_t)(B)+((int64_t)1 << (FRAC_BITS-1))) >> FRAC_BITS)


typedef int32_t real_t;

typedef real_t complex_t[2];
#define RE(A) A[0]
#define IM(A) A[1]


typedef struct
{
    uint16_t n;
    uint16_t ifac[15];
    complex_t *work;
    complex_t *tab;
} cfft_info;


void cfftf(cfft_info *cfft, complex_t *c);
void cfftb(cfft_info *cfft, complex_t *c);
cfft_info *cffti(uint16_t n);
void cfftu(cfft_info *cfft);

// =======================================

static INLINE void ComplexMult(real_t *y1, real_t *y2, real_t x1, real_t x2, real_t c1, real_t c2) {
    *y1 = (_MulHigh(x1, c1) + _MulHigh(x2, c2))<<(FRAC_SIZE-FRAC_BITS);
    *y2 = (_MulHigh(x2, c1) - _MulHigh(x1, c2))<<(FRAC_SIZE-FRAC_BITS);
}


// =======================================

typedef struct {
    uint16_t N;
    cfft_info *cfft;
    complex_t *sincos;
#ifdef PROFILE
    int64_t cycles;
    int64_t fft_cycles;
#endif
} mdct_info;

//mdct_info *faad_mdct_init(uint16_t N);
//void faad_mdct_end(mdct_info *mdct);


#ifdef __cplusplus
}
#endif
#endif
