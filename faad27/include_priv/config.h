/*
 * config.h
 *
 *  Created on: 2009/7/14
 *      Author: Wesley Wang
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define HAVE_SYS_TYPES_H	1
#define HAVE_SYS_STAT_H		0
#define STDC_HEADERS		1
#define HAVE_STRING_H		1
#define HAVE_STRINGS_H		0
#define HAVE_INTTYPES_H		0
#define HAVE_STDINT_H		0

#if defined(__sparc__)
//#define IMDCT_YLHW_SUPPORTED		// Jon defined. 2010-0906
#endif

#if !defined(__i386__)
#define fprintf(out,fmt...) 	 	printf(fmt)
#endif

#endif /* CONFIG_H_ */
