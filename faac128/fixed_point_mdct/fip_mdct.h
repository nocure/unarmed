/*
 * fip_mdct.h
 *
 *  Created on: 2011/9/8
 *      Author: Administrator
 *
 *      Fixed point MDCT routines
 */

#ifndef FIP_MDCT_H_
#define FIP_MDCT_H_

#include "cfft.h"
void fixed_point_term(void);
void fixed_point_init(void);
inline void fixp_mdct_long( real_t *data);
inline void fixp_mdct_short( real_t *data);

#endif /* FIP_MDCT_H_ */
