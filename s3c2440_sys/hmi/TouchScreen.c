/*
 * SimKey_screen.c
 *
 *  Created on: 2009/3/20
 *      Author: Wesley Wang
 */
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/YL_HMI.h>
#include <platform/yl_errno.h>
#include <string.h>
#include "osd.h"
#include "TouchScreen.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>

// ==== Screen SimKey ==========================================================
struct TS_SIMKEY {
	U32 x;
	U32 y;
	U32 w;
	U32 h;
	char *text;
} const TSKeys[] = {
//	{ 65,195, 30, 30,  "K5" },
//
//	{ 20,240, 30, 30,  "K6" },
//	{ 65,240, 30, 30,  "K7" },
//	{110,240, 30, 30,  "K8" },
//
//	{ 65,285, 30, 30,  "K9" },
//
//	{175,230, 50, 50,  "Fire"}
	{ 65,195, 30, 30,  "上" },

	{ 20,240, 30, 30,  "左" },
	{ 65,240, 30, 30,  "中" },
	{110,240, 30, 30,  "右" },

	{ 65,285, 30, 30,  "下" },

	{175,230, 50, 50,  "發射"}
};

#define NUM_TSKEYS (sizeof(TSKeys)/sizeof(struct TS_SIMKEY))

static HDC g_hdcSimKey;
static PLOGFONT g_pButtonFont;

void tsDrawkey(U32 Key,U32 kmode) {
int len;
int tx,ty;
int x1,y1,x2,y2;
SIZE size;

#define light 0xc0c0c0
#define gray1 0x909090
#define gray2 0x606060
#define dark 0x404040

DWORD ltc=0,rbc=0;
PLOGFONT oldpf;

	if (Key>=NUM_TSKEYS) return;

	x1=TSKeys[Key].x;
	y1=TSKeys[Key].y-VSCR0_YSIZE_TFT;
	x2=TSKeys[Key].w+x1;
	y2=TSKeys[Key].h+y1;
	SetPenColor(g_hdcSimKey, gray2);
	Rectangle(g_hdcSimKey,x1,y1,x2,y2);

	oldpf=SelectFont(g_hdcSimKey,g_pButtonFont);
	len = strlen(TSKeys[Key].text);
	GetTextExtent(g_hdcSimKey,TSKeys[Key].text,len,&size);
	tx = (TSKeys[Key].w- size.cx)>>1;
	if(tx<0) tx = 0;
	ty = (TSKeys[Key].h- size.cy)>>1;
	if(ty<0) ty = 0;

	len=2;

	switch(kmode){
		case TSKEY_DOWN:
			ltc=dark;
			rbc=light;
			tx+=(x1+len);
			ty+=(y1+len);
			break;

		case TSKEY_UP:
			ltc=light;
			rbc=dark;
			tx+=x1;
			ty+=y1;
			break;
	}

	while (len) {
		x1++;y1++;x2--;y2--;
		len--;
		MoveTo(g_hdcSimKey,x2,y1);
		SetPenColor(g_hdcSimKey, ltc);
		LineTo(g_hdcSimKey,x1,y1);
		LineTo(g_hdcSimKey,x1,y2);
		SetPenColor(g_hdcSimKey, rbc);
		LineTo(g_hdcSimKey,x2,y2);
		LineTo(g_hdcSimKey,x2,y1);
	}
	x1++;y1++;x2--;y2--;
	SetBrushColor(g_hdcSimKey, gray1);
	FillBox(g_hdcSimKey,x1,y1,x2-x1+1,y2-y1+1);
	SetBkMode(g_hdcSimKey,BM_TRANSPARENT);
//	SetBkColor(g_hdcSimKey,gray);
	SetTextColor(g_hdcSimKey,0x00c0f033);
	TextOut(g_hdcSimKey,tx,ty,TSKeys[Key].text);
	SelectFont(g_hdcSimKey,oldpf);

}

/* Font Style
 * rrncnn -> 012345
 * 0: c FONT_WEIGHT_BLACK
 *    b FONT_WEIGHT_BOLD
 *    k FONT_WEIGHT_BOOK
 *    d FONT_WEIGHT_DEMIBOLD
 *    l FONT_WEIGHT_LIGHT
 *    m FONT_WEIGHT_MEDIUM
 *    r FONT_WEIGHT_REGULAR
 *    s FONT_WEIGHT_SUBPIXEL
 *    * FONT_WEIGHT_ALL
 *
 * 1: i FONT_SLANT_ITALIC
 *    o FONT_SLANT_OBLIQUE
 *    r FONT_SLANT_ROMAN
 *    * FONT_SLANT_ALL
 *
 * 2: H FONT_FLIP_HORZ
 *    V FONT_FLIP_VERT
 *    T FONT_FLIP_HORZVERT
 *
 * 3: unused
 *
 * 4: u FONT_UNDERLINE_LINE
 *    * FONT_UNDERLINE_ALL
 *    n FONT_UNDERLINE_NONE
 *
 * 5: s FONT_STRUCKOUT_LINE
 *    * FONT_STRUCKOUT_ALL
 *    n FONT_STRUCKOUT_NONE
 */
void tsPrepareKeyScreen(void) {
int i=0;

	g_hdcSimKey=InitSlaveScreen("RGB32:1","240x140-32");
	if (!g_hdcSimKey) return;
//	g_pButtonFont=CreateLogFontByName("rbf-fixed-bincnn-8-16-ISO8859-1");
	g_pButtonFont=CreateLogFontByName("rbf-ming-rrncnn-12-12-big5");

	SetPenColor(g_hdcSimKey, 0xff00ff);
	Rectangle(g_hdcSimKey,0,0,239,139);

	for(i=0;i<NUM_TSKEYS;i++){
		tsDrawkey(i,TSKEY_UP);
	}
}


void cycScrClockHandler(unsigned long thread_input) {
struct tm tm;
char buf[24];
DWORD c;
#if 1
	tq_rtcGetTime(&tm);

	sprintf(buf,"%04d/%02d/%02d %02d:%02d:%02d",
		tm.tm_year,
		tm.tm_mon,
		tm.tm_mday,
		tm.tm_hour,
		tm.tm_min,
		tm.tm_sec
	);

	c=((rand()<<16)^rand())&0xffffff;
	SetBkMode(g_hdcSimKey,BM_OPAQUE);
	SetBkColor(g_hdcSimKey,c);
//	SetTextColor(g_hdcSimKey,0xc08040);
	SetTextColor(g_hdcSimKey,~c);
	TextOut(g_hdcSimKey,110,10,buf);
#endif
}


// =====================================================
#define TS_YP1 50
#define TS_YP2 950
#define TS_XP1 50
#define TS_XP2 950

#define TS_VALID_Y_START 148 //vertical
#define TS_VALID_Y_END 912
#define TS_VALID_X_START 87 //horizontal
#define TS_VALID_X_END 891
#define TS_HORIZONTAL_LENGTH (TS_VALID_Y_END-TS_VALID_Y_START)
#define TS_VERTICAL_LENGTH (TS_VALID_X_END-TS_VALID_X_START)

#define TS_SAMPLE_SHIFT				3
#define TS_SAMPLE_NUM 				(1<<TS_SAMPLE_SHIFT)
#define TS_SAMPLE_REMAINING_MASK	(TS_SAMPLE_NUM-1)


static int TS_Buffer[TS_SAMPLE_NUM][2];
static int tsFIFOIndex = -1;


// ==== Touch Screen Coordinate FIFO ========
int tsPushPointToFifo(int x,int y) {
	if(y<940) {	// check if it is touched
		tsFIFOIndex++;
		int d = tsFIFOIndex&TS_SAMPLE_REMAINING_MASK;
		TS_Buffer[d][0]=x;
		TS_Buffer[d][1]=y;
		if (tsFIFOIndex>=TS_SAMPLE_NUM) return SUCCESS;
	}
	else {
		tsFIFOIndex = -1;
	}

	return FAILED;
}


void tsGetScreenXY(int *x,int *y) {
int i=0;
int tx=0,ty=0;

	for(i=0;i<TS_SAMPLE_NUM;i++){
		tx = tx+TS_Buffer[i][0];
		ty = ty+TS_Buffer[i][1];
	}
	tx>>=TS_SAMPLE_SHIFT;
	ty>>=TS_SAMPLE_SHIFT;

	*x = ((ty-TS_VALID_Y_START)*SCR_XSIZE_TFT)/TS_HORIZONTAL_LENGTH;
	*y = SCR_YSIZE_TFT - ((tx-TS_VALID_X_START)*SCR_YSIZE_TFT/TS_VERTICAL_LENGTH);

	return;
}


// x,y is screen coordinates
int tsIsInsideKeyRange(int x,int y) {
	int i=0;
	int rtnkey;
	int x2,y2;

	rtnkey = VKEY_UNKNOWN;
	for(i=0;i<NUM_TSKEYS;i++){
		x2=TSKeys[i].x+TSKeys[i].w-1;
		y2=TSKeys[i].y+TSKeys[i].h-1;
		if((TSKeys[i].x<x)&&(x<x2)&&(TSKeys[i].y<y)&&(y<y2)){
			rtnkey = i+1;
//			GDI_PutPixel(x,y,0xffff00ff);
			break;
		}
	}
	return rtnkey;
}


//int abs(int);
static DWORD tsPenDown=0;
static int lastx=-1,lasty;
void tsGenHMIEvent(int x,int y) {
	if((x>=0)&&(x<VSCR0_XSIZE_TFT)&&(y>=0)&&(y<VSCR0_YSIZE_TFT)) {	// if it's within range, then the pen is touched
		if (tsPenDown) {
			if ((lastx!=x)||(lasty!=y)) {
//			if ((abs(lastx-x)>1) || (abs(lasty-y)>1)) {
				hmiSendMsg(HMI_TOUCH_X,x);
				hmiSendMsg(HMI_TOUCH_MOVE,y);
				lastx=x;
				lasty=y;
			}
		}
		else {
			hmiSendMsg(HMI_TOUCH_X,x);
			hmiSendMsg(HMI_TOUCH_DOWN,y);
			lastx=x;
			lasty=y;
			tsPenDown=1;
		}
	}
	else {
		if (tsPenDown) {
			hmiSendMsg(HMI_TOUCH_X,lastx);
			hmiSendMsg(HMI_TOUCH_UP,lasty);
			tsPenDown=0;
			lastx=-1;
		}
	}
}



