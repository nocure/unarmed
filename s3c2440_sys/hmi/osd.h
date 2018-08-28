/*
 * OSD.h
 *
 *  Created on: 2008/12/17
 *      Author: ww
 */

#ifndef OSD_H_
#define OSD_H_


// ==== osd.c ==========================================
#define SCR_XSIZE_TFT 		(240)
#define SCR_YSIZE_TFT 		(320)
#define VSCR0_XSIZE_TFT 	(240)
#define VSCR0_YSIZE_TFT 	(180)
#define VSCR1_XSIZE_TFT 	(240)
#define VSCR1_YSIZE_TFT 	(140)

void osdInit(void);
void osdClearScr( DWORD c);

#endif /* OSD_H_ */
