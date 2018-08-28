/*
** $Id: newgal.c 7811 2007-10-11 05:41:32Z weiym $
**
** The New Graphics Abstract Layer of MiniGUI.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/07
*/

#include <stdlib.h>
#include <platform/yl_sys.h>
#include <string.h>
//#include <hmi/osd.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include "newgal.h"
#include "misc.h"

GAL_Surface* __gal_screen;

#define LEN_ENGINE_NAME 10

BOOL GAL_ParseVideoMode (const char* mode, int* w, int* h, int* depth)
{
    const char* tmp;

    *w = atoi (mode);

    tmp = strchr (mode, 'x');
    if (tmp == NULL)
        return FALSE;
    *h = atoi (tmp + 1);

    tmp = strrchr (mode, '-');
    if (tmp == NULL)
        return FALSE;

    *depth = atoi (tmp + 1);

    return TRUE;
}


//static const char *video_engine="rgb32:0";
char *GAL_video_engine, *GAL_video_mode;
int InitGAL (void)
{
    int i;
    int w, h, depth;

    if (GAL_VideoInit (GAL_video_engine, 0)) {
        GAL_VideoQuit ();
        dbgout ("NEWGAL: Does not find matched engine: %s.\n", GAL_video_engine);
        return ERR_NO_MATCH;
    }

    if (!GAL_ParseVideoMode (GAL_video_mode, &w, &h, &depth)) {
        GAL_VideoQuit ();
        dbgout ("NEWGAL: bad video mode parameter: %s.\n", GAL_video_mode);
        return ERR_CONFIG_FILE;
    }

    /* TODO: configurable */
//    w = VSCR0_XSIZE_TFT;
//    h = VSCR0_YSIZE_TFT;
//    depth = 32;

    if (!(__gal_screen = GAL_SetVideoMode (w, h, depth, GAL_HWPALETTE))) {
        GAL_VideoQuit ();
        dbgout ("NEWGAL: Set video mode failure.\n");
        return ERR_GFX_ENGINE;
    }

    for (i = 0; i < 17; i++) {
        SysPixelIndex [i] = GAL_MapRGB (__gal_screen->format,
                        SysPixelColor [i].r,
                        SysPixelColor [i].g,
                        SysPixelColor [i].b);
    }
    return 0;
}

