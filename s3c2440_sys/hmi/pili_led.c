#include "platform/TQ_BSP.h"

static const DWORD g_Led[]={LED_2,LED_3,LED_4};
static int g_CurLit=-1;
static int g_Dir=1;

void cycPiliLEDHandler(unsigned long thread_input) {
DWORD oldc;
	if ( ((g_Dir>0)&&(g_CurLit>=2)) || ((g_Dir<0)&&(g_CurLit<=0)) ) {
		g_Dir=-g_Dir;
	}

	if (g_CurLit>=0) oldc=g_Led[g_CurLit];
	else oldc=0;
	g_CurLit+=g_Dir;
	tq_gpioClr(g_Led[g_CurLit]);	// new one on
	if (oldc) tq_gpioSet(oldc);	// old one off

}

