#include <stdlib.h>
#include "platform/YL_SYS.h"
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

//MG_EXPORT void PutRLP1(HDC hdc, int x, int y, BYTE* pattern);


const unsigned char test_icon[597]={
		0x055,0x002,0x004,0x040,0x040,
		0x0dc,0x003,0x0f5,0x076,0x052,0x041,0x036,0x03d,
		0x0be,0x068,0x000,0x000,0x000,0x000,0x000,0x000,
		0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,
		0x0f0,0x0f0,0x0f0,0x0f0,0x030,0x0f1,0x0f1,0x0f1,
		0x081,0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,
		0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,0x0f2,
		0x0f2,0x0f2,0x082,0x050,0x001,0x032,0x043,0x004,
		0x042,0x045,0x032,0x006,0x047,0x0f2,0x0a2,0x050,
		0x001,0x022,0x003,0x058,0x032,0x055,0x006,0x022,
		0x069,0x0f2,0x092,0x050,0x001,0x022,0x003,0x058,
		0x032,0x055,0x006,0x022,0x069,0x0f2,0x092,0x050,
		0x001,0x022,0x003,0x058,0x032,0x055,0x006,0x022,
		0x069,0x0f2,0x092,0x050,0x001,0x022,0x003,0x058,
		0x032,0x055,0x006,0x022,0x069,0x0f2,0x092,0x050,
		0x001,0x022,0x003,0x058,0x032,0x055,0x006,0x022,
		0x069,0x0f2,0x092,0x050,0x001,0x022,0x003,0x058,
		0x032,0x055,0x032,0x069,0x0f2,0x092,0x050,0x001,
		0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,0x0f2,0x0f2,
		0x0f2,0x082,0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,
		0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,
		0x022,0x007,0x059,0x032,0x058,0x004,0x022,0x065,
		0x042,0x0f9,0x009,0x032,0x050,0x001,0x022,0x007,
		0x059,0x032,0x058,0x004,0x022,0x065,0x042,0x0f9,
		0x009,0x032,0x050,0x001,0x022,0x007,0x059,0x032,
		0x058,0x004,0x022,0x065,0x042,0x0f9,0x009,0x032,
		0x050,0x001,0x022,0x007,0x059,0x032,0x058,0x004,
		0x022,0x065,0x042,0x0f9,0x009,0x032,0x050,0x001,
		0x022,0x007,0x059,0x032,0x058,0x004,0x022,0x065,
		0x042,0x0f9,0x009,0x032,0x050,0x001,0x022,0x007,
		0x059,0x032,0x058,0x004,0x022,0x065,0x042,0x0f9,
		0x009,0x032,0x050,0x001,0x032,0x046,0x042,0x054,
		0x042,0x046,0x052,0x0f9,0x009,0x032,0x050,0x001,
		0x0f2,0x0f2,0x032,0x0f9,0x009,0x032,0x050,0x001,
		0x0f2,0x0f2,0x032,0x0f9,0x009,0x032,0x050,0x001,
		0x0f2,0x0f2,0x032,0x0f9,0x009,0x032,0x050,0x001,
		0x032,0x055,0x032,0x059,0x032,0x003,0x048,0x003,
		0x042,0x0f9,0x009,0x032,0x050,0x001,0x022,0x065,
		0x032,0x059,0x006,0x022,0x068,0x042,0x0f9,0x009,
		0x032,0x050,0x001,0x022,0x065,0x032,0x059,0x006,
		0x022,0x068,0x042,0x0f9,0x009,0x032,0x050,0x001,
		0x022,0x065,0x032,0x059,0x006,0x022,0x068,0x042,
		0x0f9,0x009,0x032,0x050,0x001,0x022,0x065,0x032,
		0x059,0x006,0x022,0x068,0x042,0x0f9,0x009,0x032,
		0x050,0x001,0x022,0x065,0x032,0x059,0x006,0x022,
		0x068,0x042,0x0f9,0x009,0x032,0x050,0x001,0x032,
		0x055,0x032,0x059,0x032,0x003,0x048,0x003,0x042,
		0x0f9,0x007,0x032,0x050,0x001,0x0f2,0x0f2,0x0f2,
		0x082,0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,
		0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,0x0f2,
		0x0f2,0x0f2,0x082,0x050,0x001,0x0f2,0x0f2,0x0f2,
		0x082,0x050,0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,
		0x008,0x032,0x050,0x001,0x0d2,0x0f5,0x005,0x042,
		0x0f8,0x008,0x032,0x050,0x001,0x0d2,0x0f5,0x005,
		0x042,0x0f8,0x008,0x032,0x050,0x001,0x0d2,0x0f5,
		0x005,0x042,0x0f8,0x008,0x032,0x050,0x001,0x0d2,
		0x0f5,0x005,0x042,0x0f8,0x008,0x032,0x050,0x001,
		0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,0x032,0x050,
		0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,0x032,
		0x050,0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,
		0x032,0x050,0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,
		0x008,0x032,0x050,0x001,0x0d2,0x0f5,0x005,0x042,
		0x0f8,0x008,0x032,0x050,0x001,0x0d2,0x0f5,0x005,
		0x042,0x0f8,0x008,0x032,0x050,0x001,0x0d2,0x0f5,
		0x005,0x042,0x0f8,0x008,0x032,0x050,0x001,0x0d2,
		0x0f5,0x005,0x042,0x0f8,0x008,0x032,0x050,0x001,
		0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,0x032,0x050,
		0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,0x032,
		0x050,0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,0x008,
		0x032,0x050,0x001,0x0d2,0x0f5,0x005,0x042,0x0f8,
		0x008,0x032,0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,
		0x050,0x001,0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,
		0x0f2,0x0f2,0x0f2,0x082,0x050,0x001,0x0f2,0x0f2,
		0x0f2,0x082,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,
		0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x0f0,0x020,0x000
};


static void rlptest(void) {
RECT rc;
QWORD tick_start,tick_end;
int i;

	rc.left = 50; rc.top = 50; rc.right = 240; rc.bottom = 180;
	SelectClipRect(0,&rc);	// Set clip area to rc

	printf("Testing...");
	tick_start=tq_timerGetTimerTick();
	for (i=0;i<100;i++) {

		SetBkMode(HDC_SCREEN,BM_OPAQUE);
		PutRLP(HDC_SCREEN, 160, 30, (BYTE*)test_icon);
		PutRLP(HDC_SCREEN, 30, 150, (BYTE*)test_icon);

		SetBkMode(HDC_SCREEN,BM_TRANSPARENT);
		PutRLP(HDC_SCREEN, 140, 20, (BYTE*)test_icon);
		PutRLP(HDC_SCREEN, 120, 110, (BYTE*)test_icon);
		PutRLP(HDC_SCREEN, 200, 150, (BYTE*)test_icon);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d loops, time: %dms\n",i,tick_end-tick_start);
}

#if 0
static void rlp1test(void) {
RECT rc;
QWORD tick_start,tick_end;
int i;

	rc.left = 50; rc.top = 50; rc.right = 240; rc.bottom = 180;
	SelectClipRect(0,&rc);	// Set clip area to rc

	printf("Testing...");
	tick_start=tq_timerGetTimerTick();
	for (i=0;i<5000;i++) {

		SetBkMode(HDC_SCREEN,BM_OPAQUE);
		PutRLP1(HDC_SCREEN, 160, 30, (BYTE*)test_icon);
		PutRLP1(HDC_SCREEN, 30, 150, (BYTE*)test_icon);

		SetBkMode(HDC_SCREEN,BM_TRANSPARENT);
		PutRLP1(HDC_SCREEN, 140, 20, (BYTE*)test_icon);
		PutRLP1(HDC_SCREEN, 120, 110, (BYTE*)test_icon);
		PutRLP1(HDC_SCREEN, 200, 150, (BYTE*)test_icon);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d loops, time: %dms\n",i,tick_end-tick_start);

}
#endif

void getRandomXY(int* x, int* y) {
	*x=rand()%240;
	*y=rand()%180;
}

void getRandomWH(int* w, int* h) {
	*w=(rand()&0x1f)+5;
	*h=(rand()&0x1f)+5;
}

DWORD getRandomC(void) {
	return (rand()&0xffff)|((rand()&0xff)<<16);
}


static void boxtest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<5000;i++) {

		SetPenColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		Rectangle(0,x,y,x+w-1,y+h-1);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}



static void fboxtest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<5000;i++) {

		SetBrushColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		FillBox(0,x,y,w,h);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void circletest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<1000;i++) {

		SetPenColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		Circle(0,x,y,w);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void fcircletest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<1000;i++) {

		SetBrushColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		FillCircle(0,x,y,w);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void ellipstest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<1000;i++) {

		SetPenColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		Ellipse(0,x,y,w,h);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void fellipstest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,w,h;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<1000;i++) {

		SetBrushColor(0,getRandomC());
		getRandomXY(&x,&y);
		getRandomWH(&w,&h);
		FillEllipse(0,x,y,w,h);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void linetest(void) {
QWORD tick_start,tick_end;
int i;
int x,y;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	getRandomXY(&x,&y);
	MoveTo(0,x,y);
	for (i=0;i<1000;i++) {

		SetPenColor(0,getRandomC());
		getRandomXY(&x,&y);
		LineTo(0,x,y);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void slinetest(void) {
QWORD tick_start,tick_end;
int i;
int x,y,x1,y1;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<5000;i++) {
		getRandomXY(&x,&y);
		MoveTo(0,x,y);
		SetPenColor(0,getRandomC());
		getRandomXY(&x1,&y1);
		if (x1&1) {
			LineTo(0,x,y1);
		}
		else {
			LineTo(0,x1,y);
		}
	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}


static void pixeltest(void) {
QWORD tick_start,tick_end;
int i;
int x,y;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<5000;i++) {

		getRandomXY(&x,&y);
		SetPixel(0,x,y,getRandomC());

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}

static void fonttest(void) {
QWORD tick_start,tick_end;
PLOGFONT pfont,oldfont;
const char* szBuff1="中文字測試 1234 ABCD １２３４　ＡＢＣＤ";
RECT rc1;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();
// =======

	pfont=CreateLogFontByName("rbf-ming-rrncnn-12-12-big5");
	oldfont=SelectFont(0,pfont);

	rc1.left = 1; rc1.top = 1; rc1.right = 239; rc1.bottom = 42;
	SetBkColor (HDC_SCREEN, COLOR_lightwhite);
	Rectangle (HDC_SCREEN, rc1.left, rc1.top, rc1.right, rc1.bottom);
	InflateRect (&rc1, -1, -1);
	DrawText (HDC_SCREEN, szBuff1, -1, &rc1, DT_NOCLIP | DT_CENTER | DT_WORDBREAK);

	TextOut(HDC_SCREEN,10,50,szBuff1);
	SelectFont(0,oldfont);
	DestroyLogFont(pfont);

	pfont=CreateLogFontByName("rbf-ming-rrncnn-16-16-big5");
	oldfont=SelectFont(0,pfont);
	TextOut(HDC_SCREEN,10,65,"中文字測試 1234 ABCD １２３４　ＡＢＣＤ");
	SelectFont(0,oldfont);
	DestroyLogFont(pfont);


// =======
	tick_end=tq_timerGetTimerTick();
	printf("time: %dms\n",tick_end-tick_start);

}



#if 0
// _ADV_2DAPI
static void arctest(void) {
QWORD tick_start,tick_end;
int i;
int x,y;

	SelectClipRect(0,0);	// Set clip area to rc
	printf("Testing...");

	tick_start=tq_timerGetTimerTick();

	for (i=0;i<50000;i++) {

		SetPenColor(0,getRandomC());
		getRandomXY(&x,&y);
		CircleArc(0,x,y,(rand()&0x1f)+5,(rand()%360)*64,(5+(rand()%(360-5)))*64);

	}
	tick_end=tq_timerGetTimerTick();
	printf("%d items, time: %dms\n",i,tick_end-tick_start);

}
#endif

void uconGDI(int argc, char** argv) {

	if (argc<2) {
		goto show_usage;
	}

	if (!stricmp(argv[1],"cls")) {
		SelectClipRect(0,0);	// Set clip area to rc
		SetBrushColor(0,0);
		FillBox(0,0,0,240,180);
		return;
	}
	if (!stricmp(argv[1],"rlp")) {
		rlptest();
		return;
	}
#if 0
	if (!stricmp(argv[1],"rlp1")) {
		rlp1test();
		return;
	}
#endif
	if (!stricmp(argv[1],"box")) {
		boxtest();
		return;
	}
	if (!stricmp(argv[1],"fbox")) {
		fboxtest();
		return;
	}
	if (!stricmp(argv[1],"circle")) {
		circletest();
		return;
	}
	if (!stricmp(argv[1],"fcircle")) {
		fcircletest();
		return;
	}
	if (!stricmp(argv[1],"ellips")) {
		ellipstest();
		return;
	}
	if (!stricmp(argv[1],"fellips")) {
		fellipstest();
		return;
	}
	if (!stricmp(argv[1],"line")) {
		linetest();
		return;
	}
	if (!stricmp(argv[1],"sline")) {
		slinetest();
		return;
	}
	if (!stricmp(argv[1],"pixel")) {
		pixeltest();
		return;
	}
	if (!stricmp(argv[1],"font")) {
		fonttest();
		return;
	}

	if (!stricmp(argv[1],"cursor")) {
		if (!stricmp(argv[2],"on")) {
			ShowCursor(1);
			return;
		}
		else if (!stricmp(argv[2],"off")) {
			ShowCursor(0);
			return;
		}
		printf("cursor <on/off>\n");
		return;
	}


#if 0
	if (!stricmp(argv[1],"arc")) {
		arctest();
		return;
	}
#endif



//	if (yl_strtoaddr(argv[optind],(DWORD*)&addr)) return;
// 	if (yl_strtoul(argv[optind],&len)) return;


show_usage:
	printf(
		"%s <cls|cursor|rlp|box|fbox|circle|fcircle|ellips|fellips|line|sline|pixel|font><arc>\n"
	,argv[0]);
	return;
}
