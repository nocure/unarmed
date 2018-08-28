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
** $Id: mdct.c,v 1.47 2007/11/01 12:33:31 menno Exp $
**/

/*
 * Fast (I)MDCT Implementation using (I)FFT ((Inverse) Fast Fourier Transform)
 * and consists of three steps: pre-(I)FFT complex multiplication, complex
 * (I)FFT, post-(I)FFT complex multiplication,
 *
 * As described in:
 *  P. Duhamel, Y. Mahieux, and J.P. Petit, "A Fast Algorithm for the
 *  Implementation of Filter Banks Based on 'Time Domain Aliasing
 *  Cancellation?" IEEE Proc. on ICASSP?1, 1991, pp. 2209-2212.
 *
 *
 * As of April 6th 2002 completely rewritten.
 * This (I)MDCT can now be used for any data size n, where n is divisible by 8.
 *
 */

#include "common.h"
#include "structs.h"

#include <stdlib.h>
#if defined(_WIN32_WCE) || defined(__arm__) || defined(__sparc__)
#define assert(x)
#else
#include <assert.h>
#endif

#include "cfft.h"
#include "mdct.h"
#include "mdct_tab.h"

#if defined(__sparc__) && defined(IMDCT_YLHW_SUPPORTED)
#include <platform/YL_HAL.h>
#elif defined(__ARM_EABI__) //ww 2018-0101 added for eabi compiler & skyeye
#else
#define malloca(a,b) 0
#endif

// ww 2010-0927 added -->
#define REORDER(idx,shift) ({ \
register DWORD result; \
	__asm__ __volatile__( \
		"add %2,-1,%0\n"			/* shift - 1  -> %0 */ \
		"mov 1,%%l2\n" \
		"sll %%l2, %0, %%l3\n"		/* 1 << shift -1 -> l3 */ \
		"mov 2, %%l2\n"				/* 2 -> l2 */ \
		"clr %0\n" \
		"mov %2,%%l4\n" 			/* shift -> l4 */ \
	"loop:\n" \
		"btst %1, %%l2\n" \
		"bne,a notone\n" \
		"or %0, %%l3, %0\n" \
	"notone:\n" \
		"sll %%l2,1,%%l2\n" \
		"deccc %%l4\n" \
		"bne,a loop\n" \
		"srl %%l3,1,%%l3\n" \
	: "=r"(result) \
	: "r" (idx), "r" (shift) \
	: "cc","l2","l3","l4");			/* PSR's condition code is changed, so "cc" to notify compiler */ \
	result; \
})
// ww 2010-0927 added <--


// ww 2010-0927 modified -->
//mdct_info *faad_mdct_init(uint16_t N)
mdct_info *faad_mdct_init(uint16_t N,uint8_t hw_imdct)
// ww 2010-0927 modified <--
{
    mdct_info *mdct = (mdct_info*)faad_malloc(sizeof(mdct_info));

    assert(N % 8 == 0);

    mdct->N = N;

// ww 2010-0927 modified, added hw option -->
    if (hw_imdct&&((N==2048)||(N==256))) {
    	mdct->hw_imdct=1;
    	mdct->hw_in=malloca(N*2,12);
    	mdct->hw_out=malloca(N*2,12);
    }
    else {
    	mdct->hw_imdct=0;
// ww 2010-0927 modified <--

		/* NOTE: For "small framelengths" in FIXED_POINT the coefficients need to be
		 * scaled by sqrt("(nearest power of 2) > N" / N) */

		/* RE(mdct->sincos[k]) = scale*(real_t)(cos(2.0*M_PI*(k+1./8.) / (real_t)N));
		 * IM(mdct->sincos[k]) = scale*(real_t)(sin(2.0*M_PI*(k+1./8.) / (real_t)N)); */
		/* scale is 1 for fixed point, sqrt(N) for floating point */
    	switch (N)
		{
    	case 2048: mdct->sincos = (complex_t*)mdct_tab_2048; break;
		case 256:  mdct->sincos = (complex_t*)mdct_tab_256;  break;
#ifdef LD_DEC
		case 1024: mdct->sincos = (complex_t*)mdct_tab_1024; break;
#endif
#ifdef ALLOW_SMALL_FRAMELENGTH
		case 1920: mdct->sincos = (complex_t*)mdct_tab_1920; break;
		case 240:  mdct->sincos = (complex_t*)mdct_tab_240;  break;
#ifdef LD_DEC
		case 960:  mdct->sincos = (complex_t*)mdct_tab_960;  break;
#endif
#endif
#ifdef SSR_DEC
		case 512:  mdct->sincos = (complex_t*)mdct_tab_512;  break;
		case 64:   mdct->sincos = (complex_t*)mdct_tab_64;   break;
#endif
		}

		/* initialise fft */
		mdct->cfft = cffti(N/4);
    }
#ifdef PROFILE
    mdct->cycles = 0;
    mdct->fft_cycles = 0;
#endif

    return mdct;
}

void faad_mdct_end(mdct_info *mdct)
{
    if (mdct != NULL)
    {
#ifdef PROFILE
        printf("MDCT[%.4d]:         %I64d cycles\n", mdct->N, mdct->cycles);
        printf("CFFT[%.4d]:         %I64d cycles\n", mdct->N/4, mdct->fft_cycles);
#endif
// ww 2010-0927 modified -->
        if (mdct->hw_imdct) {
        	faad_free(mdct->hw_in);
        	faad_free(mdct->hw_out);
        }
        else {
        	cfftu(mdct->cfft);
        }
// ww 2010-0927 modified <--
        faad_free(mdct);
    }
}

void faad_imdct(mdct_info *mdct, real_t *X_in, real_t *X_out)
{
// ww 2010-0927 modified -->
	ALIGN complex_t Z1[512];
	uint16_t N   = mdct->N; 	// 2048 ; 256
	uint16_t N2  = N >> 1;  	// 1024 ; 128
	uint16_t N4  = N >> 2;  	// 512  ;  64
	uint16_t N8  = N >> 3;		// 256  ;  32
	uint16_t k;
#if defined(__sparc__) && defined(IMDCT_YLHW_SUPPORTED)
	if (mdct->hw_imdct) {
		int ofs,shift;
		int er;
		uint16_t N16 = N >> 4;		// 128  ;  16
		uint16_t N32 = N >> 5;		// 64	;   8


		if (N==2048) {
			yl_imdctConfig(mdct->hw_in,mdct->hw_out,1);	// 1024 mode
			shift=9;
		}
		else {
			yl_imdctConfig(mdct->hw_in,mdct->hw_out,0);	// 128 mode
			shift=6;
		}

		memcpy(mdct->hw_in,X_in, (N * 2));		// Store input data.

		yl_imdctStart();
		if ((er = yl_imdctWait())) {
			yl_uartPrintf("\nIMDCT interrupt error \n");
			return;
		}
#if 1
		uint16_t N64 = N >> 6;		// 32	;   4
		for (k=0;k<N2;k+=32) {
			ofs=REORDER(k,shift);
			RE(Z1[								ofs])	= mdct->hw_out[k+0];
			IM(Z1[								ofs])	= mdct->hw_out[k+1];

			RE(Z1[N8 +							ofs])	= mdct->hw_out[k+2];
			IM(Z1[N8 +							ofs]) 	= mdct->hw_out[k+3];

			RE(Z1[		N16 +					ofs]) 	= mdct->hw_out[k+4];
			IM(Z1[		N16 +					ofs]) 	= mdct->hw_out[k+5];

			RE(Z1[N8 + 	N16 +					ofs]) 	= mdct->hw_out[k+6];
			IM(Z1[N8 + 	N16 +					ofs]) 	= mdct->hw_out[k+7];

			RE(Z1[				N32 +			ofs])	= mdct->hw_out[k+8];
			IM(Z1[				N32 +			ofs])	= mdct->hw_out[k+9];

			RE(Z1[N8 +			N32 +			ofs])	= mdct->hw_out[k+10];
			IM(Z1[N8 +			N32 +			ofs]) 	= mdct->hw_out[k+11];

			RE(Z1[		N16 +	N32 +			ofs]) 	= mdct->hw_out[k+12];
			IM(Z1[		N16 +	N32 +			ofs]) 	= mdct->hw_out[k+13];

			RE(Z1[N8 + 	N16 +	N32 +			ofs]) 	= mdct->hw_out[k+14];
			IM(Z1[N8 + 	N16 +	N32 +			ofs]) 	= mdct->hw_out[k+15];

			RE(Z1[						N64 +	ofs])	= mdct->hw_out[k+16];
			IM(Z1[						N64 +	ofs])	= mdct->hw_out[k+17];

			RE(Z1[N8 +					N64 +	ofs])	= mdct->hw_out[k+18];
			IM(Z1[N8 +					N64 +	ofs]) 	= mdct->hw_out[k+19];

			RE(Z1[		N16 +			N64 +	ofs]) 	= mdct->hw_out[k+20];
			IM(Z1[		N16 +			N64 +	ofs]) 	= mdct->hw_out[k+21];

			RE(Z1[N8 + 	N16 +			N64 +	ofs]) 	= mdct->hw_out[k+22];
			IM(Z1[N8 + 	N16 +			N64 +	ofs]) 	= mdct->hw_out[k+23];

			RE(Z1[				N32 +	N64 +	ofs])	= mdct->hw_out[k+24];
			IM(Z1[				N32 +	N64 +	ofs])	= mdct->hw_out[k+25];

			RE(Z1[N8 +			N32 +	N64 +	ofs])	= mdct->hw_out[k+26];
			IM(Z1[N8 +			N32 +	N64 +	ofs]) 	= mdct->hw_out[k+27];

			RE(Z1[		N16 +	N32 +	N64 +	ofs]) 	= mdct->hw_out[k+28];
			IM(Z1[		N16 +	N32 +	N64 +	ofs]) 	= mdct->hw_out[k+29];

			RE(Z1[N8 + 	N16 +	N32 +	N64 +	ofs]) 	= mdct->hw_out[k+30];
			IM(Z1[N8 + 	N16 +	N32 +	N64 +	ofs]) 	= mdct->hw_out[k+31];
		}
#else
		register int Compe = 0,Compe1 = 0;
		ofs=0;
		// Output data re-ordering
		for (k = 0; k < N4; k+= (N > 256? (N32 /4) : N32)) {
			if ( ((k & 0x7) == 0) && (k > 0)) {	// "k" should be the mutiple of 8.
				if (N > 256) {			// 1024 mode?
					if ((k & (N8 - 1)) == 0 ) {	// mutiple of N8?
						Compe1 += 2;
						ofs = Compe = Compe1;
					}
					else {
						if ((k & (N16 - 1)) == 0) {	// mutiple of N16?
							Compe -= (N32 >> 4);
							ofs = Compe;
						}
						else {
							if ((k & (N32 - 1)) == 0) {	// mutiple of N32?
								Compe += (N32 >> 3);
								ofs = Compe;
							}
							else {
								if ((k & ((N32 >> 1) - 1)) == 0) {	// mutiple of (N32 / 2) ?
									ofs -= (N32 >> 2);
								}
								else {
									if ((k & ((N32 >> 2) - 1)) == 0) {	// mutiple of (N32 / 4) ?
										ofs += (N32 >> 1);
									}
								}
							}
						}
					}
				}
				else {	// 128 mode?
					if ((k & (N8 - 1)) == 0) {	// mutiple of N8?
						Compe1 += 2;
						ofs = Compe1;
					}
					else {
						if ((k & (N16 - 1)) == 0) {	// mutiple of N16?
							ofs -= (N32 >> 1);
						}
						else {
							if ((k & (N32 - 1)) == 0) {	// mutiple of N32?
								ofs += N32;
							}
						}
					}
				}
			}


			RE(Z1[ofs]) 				= mdct->hw_out[k];
			IM(Z1[ofs]) 				= mdct->hw_out[k+1];

			RE(Z1[N8 + ofs]) 		= mdct->hw_out[k+2];
			IM(Z1[N8 + ofs]) 		= mdct->hw_out[k+3];

			RE(Z1[N16 + ofs]) 		= mdct->hw_out[k+4];
			IM(Z1[N16 + ofs]) 		= mdct->hw_out[k+5];

			RE(Z1[N8 + N16 + ofs]) 	= mdct->hw_out[k+6];
			IM(Z1[N8 + N16 + ofs]) 	= mdct->hw_out[k+7];

			if (N > 256) {
				RE(Z1[N32 + ofs]) = 				mdct->hw_out[k+8];
				IM(Z1[N32 + ofs]) = 				mdct->hw_out[k+9];

				RE(Z1[N8 + N32 + ofs]) = 		mdct->hw_out[k+10];
				IM(Z1[N8 + N32 + ofs]) = 		mdct->hw_out[k+11];

				RE(Z1[N16 + N32 + ofs]) = 		mdct->hw_out[k+12];
				IM(Z1[N16 + N32 + ofs]) = 		mdct->hw_out[k+13];

				RE(Z1[N8 + N16 + N32 + ofs]) = 	mdct->hw_out[k+14];
				IM(Z1[N8 + N16 + N32 + ofs]) = 	mdct->hw_out[k+15];
			}

			RE(Z1[ofs + 1]) = 			mdct->hw_out[N4+k];
			IM(Z1[ofs + 1]) = 			mdct->hw_out[N4+k+1];

			RE(Z1[N8 + ofs + 1]) = 		mdct->hw_out[N4+k+2];
			IM(Z1[N8 + ofs + 1]) = 		mdct->hw_out[N4+k+3];

			RE(Z1[N16 + ofs + 1]) = 		mdct->hw_out[N4+k+4];
			IM(Z1[N16 + ofs + 1]) = 		mdct->hw_out[N4+k+5];

			RE(Z1[N8 + N16 + ofs + 1]) = mdct->hw_out[N4+k+6];
			IM(Z1[N8 + N16 + ofs + 1]) = mdct->hw_out[N4+k+7];

			if (N > 256) {
				RE(Z1[N32 + ofs + 1]) = 				mdct->hw_out[N4+k+8];
				IM(Z1[N32 + ofs + 1]) = 				mdct->hw_out[N4+k+9];

				RE(Z1[N8 + N32 + ofs + 1]) = 		mdct->hw_out[N4+k+10];
				IM(Z1[N8 + N32 + ofs + 1]) = 		mdct->hw_out[N4+k+11];

				RE(Z1[N16 + N32 + ofs + 1]) = 		mdct->hw_out[N4+k+12];
				IM(Z1[N16 + N32 + ofs + 1]) = 		mdct->hw_out[N4+k+13];

				RE(Z1[N8 + N16 + N32 + ofs + 1]) = 	mdct->hw_out[N4+k+14];
				IM(Z1[N8 + N16 + N32 + ofs + 1]) = 	mdct->hw_out[N4+k+15];
			}
		}

#endif
	}
	else {
#endif

// ww 2010-0927 modified <--

		complex_t x;
#ifdef ALLOW_SMALL_FRAMELENGTH
#ifdef FIXED_POINT
		real_t scale=0, b_scale = 0;
#endif
#endif
		complex_t *sincos = mdct->sincos;

//    	store_data(X_in,sizeof(real_t),N2,0);
#ifdef PROFILE
    int64_t count1, count2 = faad_get_ts();
#endif

#ifdef ALLOW_SMALL_FRAMELENGTH
#ifdef FIXED_POINT
    /* detect non-power of 2 */
		if (N & (N-1))
		{
			/* adjust scale for non-power of 2 MDCT */
			/* 2048/1920 */
			b_scale = 1;
			scale = COEF_CONST(1.0666666666666667);
		}
#endif
#endif

		/* pre-IFFT complex multiplication */
		for (k = 0; k < N4; k++)
		{
			ComplexMult(&IM(Z1[k]), &RE(Z1[k]),
				X_in[2*k], X_in[N2 - 1 - 2*k], RE(sincos[k]), IM(sincos[k]));
		}

#ifdef PROFILE
    count1 = faad_get_ts();
#endif

		/* complex IFFT, any non-scaling FFT can be used here */
		cfftb(mdct->cfft, Z1);

#ifdef PROFILE
    count1 = faad_get_ts() - count1;
#endif

    /* post-IFFT complex multiplication */
		for (k = 0; k < N4; k++)
		{
			RE(x) = RE(Z1[k]);
			IM(x) = IM(Z1[k]);
			ComplexMult(&IM(Z1[k]), &RE(Z1[k]),
				IM(x), RE(x), RE(sincos[k]), IM(sincos[k]));

#ifdef ALLOW_SMALL_FRAMELENGTH
#ifdef FIXED_POINT
			/* non-power of 2 MDCT scaling */
			if (b_scale)
			{
				RE(Z1[k]) = MUL_C(RE(Z1[k]), scale);
				IM(Z1[k]) = MUL_C(IM(Z1[k]), scale);
			}
#endif
#endif
		}
#if defined(__sparc__) && defined(IMDCT_YLHW_SUPPORTED)
	}
#endif
    /* reordering */
    for (k = 0; k < N8; k+=2)
    {
        X_out[              2*k] =  IM(Z1[N8 +     k]);
        X_out[          2 + 2*k] =  IM(Z1[N8 + 1 + k]);

        X_out[          1 + 2*k] = -RE(Z1[N8 - 1 - k]);
        X_out[          3 + 2*k] = -RE(Z1[N8 - 2 - k]);

        X_out[N4 +          2*k] =  RE(Z1[         k]);
        X_out[N4 +    + 2 + 2*k] =  RE(Z1[     1 + k]);

        X_out[N4 +      1 + 2*k] = -IM(Z1[N4 - 1 - k]);
        X_out[N4 +      3 + 2*k] = -IM(Z1[N4 - 2 - k]);

        X_out[N2 +          2*k] =  RE(Z1[N8 +     k]);
        X_out[N2 +    + 2 + 2*k] =  RE(Z1[N8 + 1 + k]);

        X_out[N2 +      1 + 2*k] = -IM(Z1[N8 - 1 - k]);
        X_out[N2 +      3 + 2*k] = -IM(Z1[N8 - 2 - k]);

        X_out[N2 + N4 +     2*k] = -IM(Z1[         k]);
        X_out[N2 + N4 + 2 + 2*k] = -IM(Z1[     1 + k]);

        X_out[N2 + N4 + 1 + 2*k] =  RE(Z1[N4 - 1 - k]);
        X_out[N2 + N4 + 3 + 2*k] =  RE(Z1[N4 - 2 - k]);

    }

#ifdef PROFILE
    count2 = faad_get_ts() - count2;
    mdct->fft_cycles += count1;
    mdct->cycles += (count2 - count1);
#endif
}

#ifdef LTP_DEC
void faad_mdct(mdct_info *mdct, real_t *X_in, real_t *X_out)
{
    uint16_t k;

    complex_t x;
    ALIGN complex_t Z1[512];
    complex_t *sincos = mdct->sincos;

    uint16_t N  = mdct->N;
    uint16_t N2 = N >> 1;
    uint16_t N4 = N >> 2;
    uint16_t N8 = N >> 3;

#ifndef FIXED_POINT
	real_t scale = REAL_CONST(N);
#else
	real_t scale = REAL_CONST(4.0/N);
#endif

#ifdef ALLOW_SMALL_FRAMELENGTH
#ifdef FIXED_POINT
    /* detect non-power of 2 */
    if (N & (N-1))
    {
        /* adjust scale for non-power of 2 MDCT */
        /* *= sqrt(2048/1920) */
        scale = MUL_C(scale, COEF_CONST(1.0327955589886444));
    }
#endif
#endif

    /* pre-FFT complex multiplication */
    for (k = 0; k < N8; k++)
    {
        uint16_t n = k << 1;
        RE(x) = X_in[N - N4 - 1 - n] + X_in[N - N4 +     n];
        IM(x) = X_in[    N4 +     n] - X_in[    N4 - 1 - n];

        ComplexMult(&RE(Z1[k]), &IM(Z1[k]),
            RE(x), IM(x), RE(sincos[k]), IM(sincos[k]));

        RE(Z1[k]) = MUL_R(RE(Z1[k]), scale);
        IM(Z1[k]) = MUL_R(IM(Z1[k]), scale);

        RE(x) =  X_in[N2 - 1 - n] - X_in[        n];
        IM(x) =  X_in[N2 +     n] + X_in[N - 1 - n];

        ComplexMult(&RE(Z1[k + N8]), &IM(Z1[k + N8]),
            RE(x), IM(x), RE(sincos[k + N8]), IM(sincos[k + N8]));

        RE(Z1[k + N8]) = MUL_R(RE(Z1[k + N8]), scale);
        IM(Z1[k + N8]) = MUL_R(IM(Z1[k + N8]), scale);
    }

    /* complex FFT, any non-scaling FFT can be used here  */
    cfftf(mdct->cfft, Z1);

    /* post-FFT complex multiplication */
    for (k = 0; k < N4; k++)
    {
        uint16_t n = k << 1;
        ComplexMult(&RE(x), &IM(x),
            RE(Z1[k]), IM(Z1[k]), RE(sincos[k]), IM(sincos[k]));

        X_out[         n] = -RE(x);
        X_out[N2 - 1 - n] =  IM(x);
        X_out[N2 +     n] = -IM(x);
        X_out[N  - 1 - n] =  RE(x);
    }
}
#endif
