#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>

#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "cursor.h"
#include "misc.h"

/******************************* global data *********************************/
RECT g_rcScr;
HWND __mg_hwnd_desktop;                       /* handle of desktop window */

extern char *GAL_video_engine, *GAL_video_mode;
int GUIAPI InitGUI (int argc, const char *argv[]) {
	if (argc<2) return -1;
	GAL_video_engine=(char*)argv[0];
	GAL_video_mode=(char*)argv[1];

    if (!InitFixStr ()) {
        dbgout ("InitGUI: Init Fixed String module failure!\n");
        return -1;
    }
    if (!InitMisc ()) {
        dbgout ("InitGUI: Initialization of misc things failure!\n");
        return -1;
    }
    switch (InitGAL ()) {
    case ERR_CONFIG_FILE:
        dbgout ("InitGUI: Reading configuration failure!\n");
        return -1;

    case ERR_NO_ENGINE:
        dbgout ("InitGUI: No graphics engine defined!\n");
        return -1;

    case ERR_NO_MATCH:
        dbgout ("InitGUI: Can not get graphics engine information!\n");
        return -1;

    case ERR_GFX_ENGINE:
        dbgout ("InitGUI: Can not initialize graphics engine!\n");
        return -1;
    }

    /* Init GDI. */

    if(!InitGDI()) {
        dbgout ("InitGUI: Initialization of GDI failure!\n");
        return -1;
    }

    /* Init Master Screen DC here */

    if (!InitScreenDC (__gal_screen)) {
        dbgout ("InitGUI: Can not initialize screen DC!\n");
        return -1;
    }

    if( !InitCursor() ) {
        dbgout ("InitGUI: Count not init mouse cursor support!\n");
        return -1;
    }

#ifdef ww_no_SystemRes_required
    if (!InitSystemRes ()) {
        dbgout ("InitGUI: Can not initialize system resource!\n");
        return FALSE;
    }
#endif
    g_rcScr.left = g_rcScr.top = 0;
    g_rcScr.right = GetGDCapability (HDC_SCREEN, GDCAP_MAXX) + 1;
    g_rcScr.bottom = GetGDCapability (HDC_SCREEN, GDCAP_MAXY) + 1;

    return 0;
}

