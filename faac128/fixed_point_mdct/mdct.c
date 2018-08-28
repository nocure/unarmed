/*
 * mdct.c
 *
 *  Created on: 2011/5/26
 *      Author: Wesley Wang
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "cfft.h"
#include "mdct_tab.h"

static mdct_info *mdct_info_long,*mdct_info_short;


static mdct_info *faad_mdct_init(uint16_t N)
{
    mdct_info *mdct = (mdct_info*)faad_malloc(sizeof(mdct_info));

    assert(N % 8 == 0);

    mdct->N = N;

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

#ifdef PROFILE
    mdct->cycles = 0;
    mdct->fft_cycles = 0;
#endif

    return mdct;
}

static void faad_mdct_end(mdct_info *mdct)
{
    if (mdct != NULL)
    {
#ifdef PROFILE
        printf("MDCT[%.4d]:         %I64d cycles\n", mdct->N, mdct->cycles);
        printf("CFFT[%.4d]:         %I64d cycles\n", mdct->N/4, mdct->fft_cycles);
#endif

        cfftu(mdct->cfft);

        faad_free(mdct);
    }
}


static void faad_mdct(mdct_info *mdct, real_t *X_in, real_t *X_out)
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


// =================================================================================
// === APIs ========================================================================
// =================================================================================

// inplace
inline void fixp_mdct_long( real_t *data) {
	faad_mdct(mdct_info_long,data,data);
}


// inplace
inline void fixp_mdct_short( real_t *data) {
	faad_mdct(mdct_info_short,data,data);
}


void fixed_point_init(void) {
	mdct_info_long=faad_mdct_init(2048); 	// long
	mdct_info_short=faad_mdct_init(256);	// short

}


void fixed_point_term(void) {
	faad_mdct_end(mdct_info_short);
	faad_mdct_end(mdct_info_long);
}
