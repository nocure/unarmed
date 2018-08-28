/*
 * ucon_crdm.c
 *
 *  Created on: 2009/4/20
 *      Author: Wesley Wang
 */

#include <string.h>
#include "platform/YL_SYS.h"
#include <..\crd\crd_def.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>

void uconcrd(int argc, char** argv) {
HRES hRes;
int size,iosize;
BYTE *buf;
BITMAP bmp;

	if (argc<2) {
		goto show_usage;
	}

	/*
	if (!stricmp(argv[1],"init")) {
	int ret;
		ret=crdRegisterMem(crd01);
		if (ret) {
			printf("register error\n");
			return;
		}

		return;
	}
*/

	if (!stricmp(argv[1],"mem")) {

		return;
	}

	void crdDisplayCacheStatus(void);
	if (!stricmp(argv[1],"cs")) {
		crdDisplayCacheStatus();
		return;
	}

	if (!stricmp(argv[1],"t1")) {
		hRes=crdOpen(RLP_1);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		iosize=crdRead(hRes,buf,size);
		if (iosize!=size) {
			printf("error\n");
			goto t1_out1;
		}

		PutRLP(HDC_SCREEN, 50, 30, (BYTE*)buf);

	t1_out1:
		free(buf);
		crdClose(hRes);
		return;
	}

	if (!stricmp(argv[1],"t2")) {
		hRes=crdOpen(RLP_2);
		buf=crdLock(hRes);

		PutRLP(HDC_SCREEN, 50, 30, (BYTE*)buf);

		crdClose(hRes);

		return;
	}


	if (!stricmp(argv[1],"t3")) {
		hRes=crdOpen(SCR0_I1);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		iosize=crdRead(hRes,buf,size);
		if (iosize!=size) {
			printf("error\n");
			goto t3_out1;
		}

		LoadBitmapFromMem(HDC_SCREEN,&bmp,buf,size,"bmp");
		FillBoxWithBitmap(HDC_SCREEN,130,20,0,0,&bmp);
		UnloadBitmap(&bmp);

	t3_out1:
		free(buf);
		crdClose(hRes);
		return;
	}


	if (!stricmp(argv[1],"t4")) {
		hRes=crdOpen(SCR0_I9);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		iosize=crdRead(hRes,buf,size);
		if (iosize!=size) {
			printf("error\n");
			goto t4_out1;
		}

		LoadBitmapFromMem(HDC_SCREEN,&bmp,buf,size,"bmp");
		FillBoxWithBitmap(HDC_SCREEN,100,50,0,0,&bmp);
		UnloadBitmap(&bmp);

	t4_out1:
		free(buf);
		crdClose(hRes);
		return;
	}

	if (!stricmp(argv[1],"t5")) {
	QWORD tick_start,tick_end;

		tick_start=tq_timerGetTimerTick();

		hRes=crdOpen(JPG_1);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);

		crdSeek(hRes,3456,CRD_SEEKSET);
		iosize=crdRead(hRes,buf+3456,1000);
		if (iosize!=1000) {
			printf("error1\n");
			goto t5_out1;
		}

		iosize=crdRead(hRes,buf+4456,size-4456);
		if (iosize!=(size-4456)) {
			printf("error2\n");
			goto t5_out1;
		}

		crdSeek(hRes,0,CRD_SEEKSET);
		iosize=crdRead(hRes,buf,3456);
		if (iosize!=3456) {
			printf("error3\n");
			goto t5_out1;
		}

		tick_end=tq_timerGetTimerTick();
		printf("time: %dms\n",tick_end-tick_start);

		LoadBitmapFromMem(HDC_SCREEN,&bmp,buf,size,"jpg");
		FillBoxWithBitmap(HDC_SCREEN,20,20,200,140,&bmp);
		UnloadBitmap(&bmp);

	t5_out1:
		free(buf);
		crdClose(hRes);
		return;
	}

	if (!stricmp(argv[1],"t6")) {
		hRes=crdOpen(SCR0_BK);
		size=crdSize(hRes);
		buf=(BYTE*)malloc(size);
		iosize=crdRead(hRes,buf,size);
		if (iosize!=size) {
			printf("error\n");
			goto t6_out1;
		}

		LoadBitmapFromMem(HDC_SCREEN,&bmp,buf,size,"bmp");
		FillBoxWithBitmap(HDC_SCREEN,0,0,0,0,&bmp);
		UnloadBitmap(&bmp);

	t6_out1:
		free(buf);
		crdClose(hRes);
		return;
	}



show_usage:
	printf(
		"%s <init><cs><t1><t2>\n"
	,argv[0]);
	return;
}
