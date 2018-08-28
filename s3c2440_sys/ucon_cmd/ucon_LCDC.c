/*
 * ucon_LCDC.c
 *
 *  Created on: 2009/4/9
 *      Author: Wesley Wang
 */

#include "platform/YL_SYS.h"
#include <../bsp/2440addr.h>
#include <string.h>

#define LCD_XSIZE_TFT	320
#define LCD_YSIZE_TFT	240

void uconlcd(int argc, char** argv) {
DWORD front,width,back;

    if (argc<5) goto showusage;

    if (strcmpi(argv[1],"vd")==0) {
		if (yl_strtoul(argv[2],&front)) return;
		if (yl_strtoul(argv[3],&width)) return;
		if (yl_strtoul(argv[4],&back)) return;
		rLCDCON2=(back<<24)|((LCD_YSIZE_TFT-1)<<14)|(front<<6)|(width);
    }
    else if (strcmpi(argv[1],"hd")==0) {
        if (yl_strtoul(argv[2],&front)) return;
        if (yl_strtoul(argv[3],&width)) return;
        if (yl_strtoul(argv[4],&back)) return;
    	rLCDCON3=(back<<19)|((LCD_XSIZE_TFT-1)<<8)|(front);
    	rLCDCON4=width;
    }
    else goto showusage;

	return;

showusage:
	printf("Usage:\n");
	printf("\t %s <vd|hd> <Front> <Width> <Back>\n\n",argv[0]);
	return;
}


