/*
 * ucon_heap.c
 *
 *  Created on: 2018¦~8¤ë23¤é
 *      Author: ww
 */

#include <stdlib.h>
#include <string.h>
#include <platform/yl_Sys.h>

void uconheapmem(int argc, char** argv) {
DWORD free_size;
MEMORY_STATUS_T ms;

	free_size=memGetStatus(&ms);
	printf(
			"Free:		%d bytes\n"
			"Used:		%d bytes\n"
			"Largest:	%d bytes\n"
			"Segments:	%d\n"
			"MCBs:		%d\n"
			,free_size,ms.used,ms.largest,ms.segs,ms.mcbs
	);
}


