/*
 ** $Id: mgetc.c 7590 2007-09-17 09:20:06Z xwyan $
 **
 ** mgetc.c: definitions for incore resource.
 **
 ** Copyright (C) 2003 ~ 2007 Feynman Software.
 **
 ** Create date: 2003/09/22
 */

#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>

#ifdef _INCORE_RES

#define _ETC_CONFIG_EVENT

/*
 * Please modify this file to meet your customer's board
 * system configuration options.
 */
#include "mgetc-custom.h"


#ifndef _SYS_CFG_INCLUDED
/* system configure files for boards running Linux */
#   ifdef _IPAQ_IAL
#include "mgetc-ipaq.c"
#   endif
#   ifdef _FIGUEROA_IAL
#include "mgetc-figueroa.c"
#   endif
#   ifdef _FFT7202_IAL
#include "mgetc-fft7202.c"
#   endif
#   ifdef _DM270_IAL
#include "mgetc-dm270.c"
#   endif
#   ifdef _EVMV10_IAL
#include "mgetc-xscale.c"
#   endif
#   ifdef _EMBEST2410_IAL
#include "mgetc-embest2410.c"
#   endif
#   ifdef _FXRM9200_IAL
#include "mgetc-rm9200.c"
#   endif
#   ifdef _HH2410R3_IAL
#include "mgetc-hh2410r3.c"
#   endif
#   ifdef _HH2410R3_IAL
#include "mgetc-hh2440.c"
#   endif

//#include "mgetc-pc.c"

#endif /* !_SYS_CFG_INCLUDED */

static char *CURSORINFO_KEYS[] = {"cursornumber"};
//static char *CURSORINFO_VALUES[] = {"23"};
static char *CURSORINFO_VALUES[] = {"2"};		// TODO: restore to 23

static char *ICONINFO_KEYS[] = {"iconnumber"};
static char *ICONINFO_VALUES[] = {"5"};

static char *BITMAPINFO_KEYS[] = {"bitmapnumber"};
static char *BITMAPINFO_VALUES[] = {"6"};
static char *BGPICTURE_KEYS[] = {"position", "file"};

#ifdef BACKGROUND_IMAGE_FILE
static char *BGPICTURE_VALUES[] = {"center", BACKGROUND_IMAGE_FILE};
#else
static char *BGPICTURE_VALUES[] = {"none", ""};
#endif

static char *EVENT_KEYS[] = {"timeoutusec", "repeatusec"};
static char *EVENT_VALUES[] = {"300000", "50000"};

#if (defined(_TTF_SUPPORT) || defined(_FT2_SUPPORT)) && defined (__TARGET_MGDESKTOP__)
static char* TTFINFO_KEYS[] = {"font_number", "name0", "fontfile0", "name1", "fontfile1"};
static char* TTFINFO_VALUES[] = {"2",
        "ttf-arial-rrncnn-0-0-ISO8859-1", "/usr/local/lib/minigui/res/font/arial.ttf",
        "ttf-times-rrncnn-0-0-ISO8859-1", "/usr/local/lib/minigui/res/font/times.ttf"
};
#endif

static ETCSECTION mgetc_sections [] =
{
  {0, TABLESIZE(SYSTEMFONT_KEYS), "systemfont",  SYSTEMFONT_KEYS, SYSTEMFONT_VALUES},
  {0, 1, "cursorinfo",   CURSORINFO_KEYS, CURSORINFO_VALUES},
  {0, 1, "iconinfo",     ICONINFO_KEYS,   ICONINFO_VALUES},
  {0, 1, "bitmapinfo",   BITMAPINFO_KEYS, BITMAPINFO_VALUES},
/* optional sections */
  {0, 2, "bgpicture", BGPICTURE_KEYS, BGPICTURE_VALUES},
  /*
  {1, "mouse", MOUSE_KEYS, MOUSE_VALUES},
  */
  {0, 2, "event", EVENT_KEYS, EVENT_VALUES},
#if (defined (_TTF_SUPPORT) || defined (_FT2_SUPPORT)) && defined (__TARGET_MGDESKTOP__)
  {0, TABLESIZE(TTFINFO_KEYS), "truetypefonts", TTFINFO_KEYS, TTFINFO_VALUES},
#endif
};

static ETC_S MGETC = {0, TABLESIZE (mgetc_sections), mgetc_sections};

GHANDLE __mg_get_mgetc (void)
{
	return (GHANDLE) &MGETC;
}

#endif /* _INCORE_RES */

