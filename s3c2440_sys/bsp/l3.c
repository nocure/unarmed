/*
 * L3 code
 *
 *  Copyright (C) 2008, Christian Pellegrin <chripell@evolware.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * based on:
 *
 * L3 bus algorithm module.
 *
 *  Copyright (C) 2001 Russell King, All Rights Reserved.
 *
 *
 */

#include "platform/tq_bsp.h"

/*
 * Send one byte of data to the chip.  Data is latched into the chip on
 * the rising edge of the clock.
 */
static void sendbyte(unsigned int byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		l3_setclk(0);
		udelay(1);	/* data hold */
		l3_setdat(byte & 1);
		udelay(1);	/* data setup */
		l3_setclk(1);
		udelay(1);	/* clock high */
		byte >>= 1;
	}
}

/*
 * Send a set of bytes to the chip.  We need to pulse the MODE line
 * between each byte, but never at the start nor at the end of the
 * transfer.
 */
static void sendbytes(const U8 *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i) {
			udelay(1);		/* mode hold */
			l3_setmode(0);
			udelay(1);		/* mode */
		}
		l3_setmode(1);
		udelay(1);			/* mode setup */
		sendbyte(buf[i]);
	}
}

int l3_write(U8 addr, U8 *data, int len)
{
	l3_setclk(1);
	l3_setdat(1);
	l3_setmode(1);
	udelay(1);		/* mode */

	l3_setmode(0);
	udelay(1);		/* mode setup */
	sendbyte(addr);
	udelay(1);		/* mode hold */

	sendbytes(data, len);

	l3_setclk(1);
	l3_setdat(1);
	l3_setmode(0);

	return len;
}
