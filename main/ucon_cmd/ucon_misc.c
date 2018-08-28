/*
 * ucon_misc.c
 *
 *  Created on: 2010/8/6
 *      Author: Wesley Wang
 */

#include "platform/YL_Sys.h"
#include "ucos_ii.h"


void uconCPUUsage(int argc, char** argv) {
	/* in extreme load situation, the cpu stat counter does't even have a
	 * chance to update stat, render this quite useless.
	 * on no load, 1%, and on high load still 1%.
	 */
	printf("CPU usage: %d%%\n",OSCPUUsage);
}

