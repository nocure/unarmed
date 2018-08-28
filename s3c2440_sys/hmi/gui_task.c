/*
 * gui_task.c
 *
 *  Created on: 2008/12/19
 *      Author:
 */
#include <sct.h>
#include "platform/YL_SYS.h"
#include <platform/YL_HMI.h>
#include <platform/YL_ERRNO.h>
#include <string.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <..\crd\crd01.h> 				// the depot 01 binary
#include <..\crd\crd_def.h>


int guiSendMsg(DWORD msg, DWORD dat) {
int er;
	er=snd_msg(MBXID_GUI,(void*)PACK_HMI_MSG(msg,dat));
	if (er) return FAILED;
	return SUCCESS;
}

int guiRecvMsg(DWORD *msg, DWORD *dat) {
DWORD packed;
int er;

	er=rcv_msg(MBXID_GUI,(void*)&packed);
	if (er) return FAILED;

	*msg=UNPACK_HMI_MSG(packed);
	*dat=UNPACK_HMI_DAT(packed);;

	return SUCCESS;
}


// =======================================================

static void guiInit(void){
int ret;

	SetCursorPos (240 >> 1, 180 >> 1);
	SetCursor (GetSystemCursor (IDC_ARROW));
//	SetMode(M_MAIN_MENU);
//	SetModePtr(GuiMainMenu);
//	GUI_SetAllIconOn();

	ret=crdRegisterMem(crd01);
	if (ret) {
		printf("CRD01 register error\n");
		return;
	}

}

// =======================================================
typedef struct SLIDING_BAND_S {
	DWORD *icons;

	int numicons;
	int icon_pitch;
	const int *speedtbl;
	int fullspeed;
	DWORD opt;
	POINT band_pos;
	SIZE band_size;
	SIZE icon_size;

	DWORD background;
} SLIDING_BAND_T;

typedef struct SLIDING_BAND_PIRV_S {
	BITMAP **icons;

	int numicons;
	int icon_pitch;
	const int *speedtbl;
	int fullspeed;
	DWORD opt;
	POINT band_pos;
	SIZE band_size;
	SIZE icon_size;


	BITMAP *background;
	BITMAP *slideband;
	int num_band_icons;
	int band_icon_pitch;
} SLIDING_BAND_PIRV_T;

#define VERTICAL_BAND		(1<<0)


void FreeSlidingBandStruct(SLIDING_BAND_PIRV_T* sbp) {
int i;

	if (sbp->background) {
		UnloadBitmap(sbp->background);
		free(sbp->background);
	}

	for (i=0;i<sbp->numicons;i++) {
		if (sbp->icons[i]) {
			UnloadBitmap(sbp->icons[i]);
			free(sbp->icons[i]);
		}
	}

	if (sbp->icons) free(sbp->icons);
}


DWORD CreateSlidingBand(SLIDING_BAND_T* sb) {
SLIDING_BAND_PIRV_T *sbp;
int i,ret;
BYTE *buf;
HRES hRes;
int size,iosize;

	sbp=malloc(sizeof(SLIDING_BAND_PIRV_T));
	if (!sbp) goto err0;
	memset(sbp,0,sizeof(SLIDING_BAND_PIRV_T));

	sbp->numicons=sb->numicons;

	sbp->icons=malloc(sizeof(BITMAP *)*sb->numicons);
	if (!sbp->icons) goto err1;

	for (i=0;i<sbp->numicons;i++) {

		hRes=crdOpen(sb->icons[i]);
		if (!hRes) goto err2;
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		if (!buf) goto err2;
		iosize=crdRead(hRes,buf,size);
		if (iosize==size) {
			ret=LoadBitmapFromMem(HDC_SCREEN,sbp->icons[i],buf,size,"bmp");
			if (ret) goto err2;
			sbp->icons[i]->bmColorKey = 0x0000ffff;
			sbp->icons[i]->bmType|= BMP_TYPE_COLORKEY;

		}
		else {
			goto err2;
		}
		free(buf);
		crdClose(hRes);
	}

	sbp->background=malloc(sizeof(BITMAP));
	if (!sbp->background) goto err2;

	hRes=crdOpen(sb->background);
	if (!hRes) goto err2;
	size=crdSize(hRes);
	buf=(BYTE*)malloc(size);
	if (!buf) goto err2;
	iosize=crdRead(hRes,buf,size);
	if (iosize==size) {
		ret=LoadBitmapFromMem(HDC_SCREEN,sbp->background,buf,size,"bmp");
	}
	else {
		goto err2;
	}
	free(buf);
	crdClose(hRes);

	sbp->numicons=sb->numicons;
	sbp->icon_pitch=sb->icon_pitch;
	sbp->speedtbl=sb->speedtbl;
	sbp->fullspeed=sb->fullspeed;
	sbp->opt=sb->opt;
	sbp->band_pos=sb->band_pos;
	sbp->band_size=sb->band_size;
	sbp->icon_size=sb->icon_size;
	if (BIT_VAL(sb->opt,VERTICAL_BAND)) {
		sbp->num_band_icons=sbp->band_size.cy/sbp->band_size.cx;
		sbp->band_icon_pitch=0;
		InitBitmap(HDC_SCREEN,sbp->num_band_icons*sbp->icon_size.cx,sbp->icon_size.cy,sbp->num_band_icons*sbp->icon_size.cx,0,sbp->slideband);
	}
	else {
		sbp->num_band_icons=sbp->band_size.cx/sbp->icon_size.cy;
		InitBitmap(HDC_SCREEN,sbp->num_band_icons*sbp->band_size.cx,sbp->band_size.cy,sbp->num_band_icons*sbp->band_size.cx,0,sbp->slideband);

	}






err2:
	FreeSlidingBandStruct(sbp);
err1:
	free(sbp);
err0:

	return 0;
}



static const DWORD SCR0_ICONS[]={
		SCR0_I1,SCR0_I2 ,SCR0_I3 ,SCR0_I4 ,SCR0_I5 ,SCR0_I6 ,SCR0_I7 ,SCR0_I8 ,
		SCR0_I9,SCR0_I10,SCR0_I11,SCR0_I12,SCR0_I13,SCR0_I14,SCR0_I15,SCR0_I16,
		SCR0_I17,SCR0_I18,SCR0_I19
};
#define NUM_SCR0_ICONS	(sizeof(SCR0_ICONS)/sizeof(DWORD))

static int current_selection=0;
static BITMAP scr0_bmp_icons[NUM_SCR0_ICONS],scr0_bmp_bk,scr0_bmp_slide_band;
//static BITMAP scr0_bmp_band_bk;

void scr0Enter(void) {
int i;
HRES hRes;
int size,iosize;
BYTE *buf;
//HDC hdc;

	hRes=crdOpen(SCR0_BK);
	size=crdSize(hRes);
	buf=(BYTE*)malloc(size);
	iosize=crdRead(hRes,buf,size);
	if (iosize==size) {
		LoadBitmapFromMem(HDC_SCREEN,&scr0_bmp_bk,buf,size,"bmp");
	}
	else {
		printf("error scr0_bk\n");
	}
	free(buf);
	crdClose(hRes);

	for (i=0;i<NUM_SCR0_ICONS;i++) {
		hRes=crdOpen(SCR0_ICONS[i]);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		iosize=crdRead(hRes,buf,size);
		if (iosize==size) {
			LoadBitmapFromMem(HDC_SCREEN,&scr0_bmp_icons[i],buf,size,"bmp");
			scr0_bmp_icons[i].bmColorKey = 0x0000ffff;
			scr0_bmp_icons[i].bmType|= BMP_TYPE_COLORKEY;

		}
		else {
			printf("error scr0_icon_%d\n",i);
		}
		free(buf);
		crdClose(hRes);
	}

	InitBitmap(HDC_SCREEN,5*48,48,5*48,0,&scr0_bmp_slide_band);

//	hdc=CreateMemDCFromBitmap(HDC_SCREEN,&scr0_bmp_bk);
//	GetBitmapFromDC(hdc,0,66,240,48,&scr0_bmp_band_bk);
//	DeleteMemDC(hdc);

}

void scr0Exit(void) {
int i;

//	UnloadBitmap(&scr0_bmp_band_bk);
	UnloadBitmap(&scr0_bmp_slide_band);
	UnloadBitmap(&scr0_bmp_bk);
	for (i=0;i<NUM_SCR0_ICONS;i++) {
		UnloadBitmap(&scr0_bmp_icons[i]);
	}
}


#define ICON_OFFSET	8
void scr0DrawSlideBand(int sel, int offset) {
int i;
int icon_no,xshift;;
int start_icon,num_draw;
HDC hdc;
int diff;


//	hdcBK=CreateMemDCFromBitmap(HDC_SCREEN,&scr0_bmp_slide_band);

	hdc=CreateMemDCFromBitmap(HDC_SCREEN,&scr0_bmp_bk);
	GetBitmapFromDC(hdc,0,66,240,48,&scr0_bmp_slide_band);
	DeleteMemDC(hdc);

//	memcpy(scr0_bmp_slide_band.bmBits,scr0_bmp_band_bk.bmBits,scr0_bmp_slide_band.bmPitch*scr0_bmp_slide_band.bmHeight);

	hdc=CreateMemDCFromBitmap(HDC_SCREEN,&scr0_bmp_slide_band);

	xshift=offset;
	start_icon=sel;
	if (offset==0) {
		sel=2;
		num_draw=5;
	}
	else {
		if (offset<0) {
			sel=3;
			xshift+=48;
			offset=-offset;
		}
		else sel=2;
		num_draw=6;
	}
	start_icon-=sel;

	for (i=0;i<num_draw;i++) {
		icon_no=start_icon+i;
		if (icon_no<0) icon_no+=NUM_SCR0_ICONS;
		else if (icon_no>=NUM_SCR0_ICONS) icon_no-=NUM_SCR0_ICONS;

		if (i==sel) {
			diff=(24-offset)*8/24;
			FillBoxWithBitmap(hdc,(i*48)+ICON_OFFSET-xshift-diff,ICON_OFFSET-diff,32+diff+diff,32+diff+diff,&scr0_bmp_icons[icon_no]);
		}
		else {
			FillBoxWithBitmap(hdc,(i*48)+ICON_OFFSET-xshift,ICON_OFFSET,0,0,&scr0_bmp_icons[icon_no]);
		}
	}

	DeleteMemDC(hdc);

	FillBoxWithBitmap(HDC_SCREEN,0,66,0,0,&scr0_bmp_slide_band);
}


static const int Speed_Ctrl[][2]={
		{5,50},
		{35,200},
		{8,80},
		{0,0}
};


void scr0MoveItem(int cur,int ofs) {
QWORD tick_start,tick_past;
int dir,steps;
int i,t,ms_dot,shift,twin,ttwin,tdot;

	if (ofs==0) return;
	else if (ofs>0) {
		dir=1;
		steps=ofs;
	}
	else {
		dir=-1;
		steps=-ofs;
	}

	tick_start=tq_timerGetTimerTick();
	scr0DrawSlideBand(cur, 0);

	while (steps) {
		while (1) {
			tick_past=tq_timerGetTimerTick()-tick_start;
			ttwin=0;
			tdot=0;

			for (i=0;;i++) {
				if (!(t=Speed_Ctrl[i][0])) break;
				ms_dot=1000/Speed_Ctrl[i][1];
				twin=ms_dot*t;
				if (tick_past<=ttwin+twin) {
					shift=((tick_past-ttwin)/ms_dot)+tdot;
					break;
				}
				ttwin+=twin;
				tdot+=Speed_Ctrl[i][0];
				shift=48;
			}

			if (shift<24) {
				scr0DrawSlideBand(cur, shift*dir);
			}
			else {
				scr0DrawSlideBand(cur+dir, (shift-48)*dir);
			}

			if (shift>=48) break;;
		}
		steps--;
	}



}


void gui_tsk(ULONG thread_input) {
int ret;
DWORD type;
DWORD data;
QWORD tick_start,tick_end;
int shift;
int timeleft;

	guiInit();

	scr0Enter();

	FillBoxWithBitmap(HDC_SCREEN,0,0,0,0,&scr0_bmp_bk);
	scr0DrawSlideBand(current_selection,0);

	while (1) {
		ret=guiRecvMsg(&type,&data);
		if (ret) {
			rot_rdq();	// let other tasks to run
			continue;
		}
		switch (type) {
			case HMI_KEY_DOWN:
				switch(data) {
				case VKEY_6:	// left
					scr0MoveItem(current_selection,-1);
					current_selection--;
					if (current_selection<0) current_selection=NUM_SCR0_ICONS-1;
					break;

				case VKEY_8:	// right
					scr0MoveItem(current_selection,1);
					current_selection++;
					if (current_selection>=NUM_SCR0_ICONS) current_selection=0;
					break;

				}
				break;
		}


	}


	current_selection=0;
	shift=0;
	while (1) {

		tick_start=tq_timerGetTimerTick();
		scr0DrawSlideBand(current_selection, shift);
		shift+=6;
		if (shift>=24) {
			shift=-24;
			current_selection++;
			if (current_selection>=NUM_SCR0_ICONS) {
				current_selection=0;
			}
		}
		tick_end=tq_timerGetTimerTick();
		timeleft=33-(tick_end-tick_start);
		if (timeleft>0) dly_tsk(timeleft);



#if 0
		ret=guiRecvMsg(&type,&data);
		if (ret) {
			rot_rdq(255);	// let other tasks to run
			continue;
		}
#endif

	}


}
