/*
 * YL_OSWrap_priv.h
 *
 *  Created on: 2012/8/17
 *      Author: User
 */

#ifndef YL_OSWRAP_PRIV_H_
#define YL_OSWRAP_PRIV_H_

#define MEMDEF(t,sz) \
		t=ptr=((void*)ptr)+pinc; \
		pinc=sz


int yl_oswrap_errcode(INT8U err);
int is_released(void);

#endif /* YL_OSWRAP_PRIV_H_ */
