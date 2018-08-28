/*
 * osd.c
 *
 *  Created on: 2008/12/17
 *      Author: ww
 */
#include "platform/TQ_BSP.h"
#include "osd.h"

unsigned int * const LCD_BUFFER=(unsigned int *)(0);

void osdClearScr( DWORD c) {
unsigned int i ;
unsigned int *p=LCD_BUFFER;

	i=SCR_XSIZE_TFT*SCR_YSIZE_TFT;
	while (i--) {
		*p++=c;
	}

}


void osdInit(void) {
	tq_lcdcInit();
	tq_lcdcPowerEnable(0, 1);
	tq_lcdcEnvidOnOff(1);		//turn on vedio
	tq_lcdcBkLtSet( 70 ) ;

//	osdClearScr( (DWORD)0x00000000);	// 00RRGGBB
//	GDI_SetFont(GDI_Courier);

}



