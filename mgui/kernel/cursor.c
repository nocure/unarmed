/*
** $Id: cursor.c 8089 2007-11-16 04:35:10Z xwyan $
**
** cursor.c: the Cursor Support module for MiniGUI-Threads.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** All right reserved by Feynman Software.
**
** Current maintainer: Wei Yongming.
**
** Create date: 1999.01.06
*/

#include <stdlib.h>
#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"
#include "gal.h"
#include "dc.h"
#include "memops.h"
#include "inline.h"
#include "gal.h"
#include "cursor.h"
#include "ial.h"
#include "readbmp.h"
#include "misc.h"
#include "cursor.h"
#include "sysres.h"


// ww 2009-0410: added static allocated memory for savebits, increases performance
#define STATIC_SAVEDBITS

#ifdef STATIC_SAVEDBITS
#define CURSOR_BITS_SIZE	4096
static BYTE savedbits_mem[CURSOR_BITS_SIZE] __attribute__ ((aligned (0x10))) ;
static BYTE cursorbits_mem[CURSOR_BITS_SIZE] __attribute__ ((aligned (0x10))) ;
#endif

/* mutex ensuring exclusive access to mouse. */
pthread_mutex_t __mg_mouselock;

static int __mg_cursor_x, __mg_cursor_y;
static int oldx = -1, oldy;

static RECT cliprc = { 0, 0, 0, 0};

#ifdef _CURSOR_SUPPORT

static PCURSOR SysCursor [MAX_SYSCURSORINDEX + 1];
static HCURSOR def_cursor;
static BYTE* savedbits = NULL;
static BYTE* cursorbits = NULL;
static unsigned int csrimgsize;
static unsigned int csrimgpitch;

static int oldboxleft = -100, oldboxtop = -100;
static int nShowCount = 0;
static PCURSOR pCurCsr = NULL;

#ifdef _USE_NEWGAL
Uint8* GetPixelUnderCursor (int x, int y, gal_pixel* pixel)
{
    Uint8* dst = NULL;

    pthread_mutex_lock (&__mg_mouselock);
    if (nShowCount >= 0 && pCurCsr
            && x >= oldboxleft && y >= oldboxtop
            && (x < oldboxleft + CURSORWIDTH)
            && (y < oldboxtop + CURSORHEIGHT)) {
        int _x = x - oldboxleft;
        int _y = y - oldboxtop;

        dst = savedbits + _y * csrimgpitch
                + _x * __gal_screen->format->BytesPerPixel;
        *pixel = _mem_get_pixel (dst, __gal_screen->format->BytesPerPixel);
    }
    pthread_mutex_unlock (&__mg_mouselock);

    return dst;
}
#endif

/* Cursor creating and destroying. */
/* Only called from InitCursor and client code. */
HCURSOR GUIAPI LoadCursorFromFile(const char* filename)
{
    FILE* fp;
    WORD wTemp;
    int  w, h, xhot, yhot, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;
    BYTE* image;
    HCURSOR csr = 0;

    if( !(fp = fopen(filename, "r")) ) return 0;

    fseek(fp, sizeof(WORD), SEEK_SET);

    /* the cbType of struct CURSORDIR. */
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 2) goto error;

    /* skip the cdCount of struct CURSORDIR, we always use the first cursor. */
    fseek(fp, sizeof(WORD), SEEK_CUR);

    /* cursor info, read the members of struct CURSORDIRENTRY. */
    w = fgetc (fp);  /* the width of first cursor. */
    h = fgetc (fp);  /* the height of first cursor. */
    if(w != CURSORWIDTH || h != CURSORHEIGHT) goto error;
    fseek(fp, sizeof(BYTE)*2, SEEK_CUR); /* skip bColorCount and bReserved. */
    wTemp = MGUI_ReadLE16FP (fp);
    xhot = wTemp;
    wTemp = MGUI_ReadLE16FP (fp);
    yhot = wTemp;
    size = MGUI_ReadLE32FP (fp);
    offset = MGUI_ReadLE32FP (fp);

    /* read the cursor image info. */
    fseek(fp, offset, SEEK_SET);
    fseek(fp, sizeof(DWORD), SEEK_CUR); /* skip the biSize member. */
    imagew = MGUI_ReadLE32FP (fp);
    imageh = MGUI_ReadLE32FP (fp);
    /* check the biPlanes member; */
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp != 1) goto error;
    /* check the biBitCount member; */
    wTemp = MGUI_ReadLE16FP (fp);
    if(wTemp > 4) goto error;
    colornum = (int)wTemp;
    fseek(fp, sizeof(DWORD), SEEK_CUR); /* skip the biCompression members. */
    imagesize = MGUI_ReadLE32FP (fp);

    /* skip the rest members and the color table. */
    fseek(fp, sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum), SEEK_CUR);

    /* allocate memory for image. */
    if ((image = (BYTE*)ALLOCATE_LOCAL (imagesize)) == NULL)
        goto error;

    /* read image */
    fread (image, imagesize, fp);

    csr = CreateCursor(xhot, yhot, w, h,
                        image + (imagesize - MONOSIZE), image, colornum);

    DEALLOCATE_LOCAL (image);

error:
    fclose (fp);
    return csr;
}

HCURSOR GUIAPI LoadCursorFromMem (const void* area)
{
    const Uint8* p = (Uint8*)area;
    WORD wTemp;

    int  w, h, xhot, yhot, colornum;
    DWORD size, offset;
    DWORD imagesize, imagew, imageh;

    p += sizeof (WORD);
    wTemp = MGUI_ReadLE16Mem (&p);
    if(wTemp != 2) goto error;

    /* skip the cdCount of struct CURSORDIR, we always use the first cursor. */
    p += sizeof (WORD);

    /* cursor info, read the members of struct CURSORDIRENTRY. */
    w = *p++;  /* the width of first cursor. */
    h = *p++;  /* the height of first cursor. */
    if (w != CURSORWIDTH || h != CURSORHEIGHT)
        goto error;

    /* skip the bColorCount and bReserved. */
    p += sizeof(BYTE)*2;
    xhot = MGUI_ReadLE16Mem (&p);
    yhot = MGUI_ReadLE16Mem (&p);
    size = MGUI_ReadLE32Mem (&p);
    offset = MGUI_ReadLE32Mem (&p);

    /* read the cursor image info. */
    p = (Uint8*)area + offset;

    /* skip the biSize member. */
    p += sizeof (DWORD);
    imagew = MGUI_ReadLE32Mem (&p);
    imageh = MGUI_ReadLE32Mem (&p);

    /* check the biPlanes member; */
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp != 1) goto error;

    /* check the biBitCount member; */
    wTemp = MGUI_ReadLE16Mem (&p);
    if (wTemp > 4) goto error;
    colornum = wTemp;

    /* skip the biCompression members. */
    p += sizeof (DWORD);
    imagesize = MGUI_ReadLE32Mem (&p);

    /* skip the rest members and the color table. */
    p += sizeof(DWORD)*4 + sizeof(BYTE)*(4<<colornum);

    return CreateCursor (xhot, yhot, w, h,
                        p + (imagesize - MONOSIZE), p, colornum);

error:
    return 0;
}

#ifdef _USE_NEWGAL
static BITMAP csr_bmp = {
    BMP_TYPE_NORMAL, 0, 0, 0, 0, CURSORWIDTH, CURSORHEIGHT
};
#endif

/* Only called from InitCursor and client code. */
HCURSOR GUIAPI CreateCursor(int xhotspot, int yhotspot, int w, int h,
                     const BYTE* pANDBits, const BYTE* pXORBits, int colornum)
{
    PCURSOR pcsr;

    if( w != CURSORWIDTH || h != CURSORHEIGHT ) return 0;

    /* allocate memory. */
    if( !(pcsr = (PCURSOR)malloc(sizeof(CURSOR))) ) return 0;
    if( !(pcsr->AndBits = malloc(csrimgsize)) ) {
        free(pcsr);
        return 0;
    }
    if( !(pcsr->XorBits = malloc(csrimgsize)) ) {
        free(pcsr->AndBits);
        free(pcsr);
        return 0;
    }

    pcsr->xhotspot = xhotspot;
    pcsr->yhotspot = yhotspot;
    pcsr->width = w;
    pcsr->height = h;

#ifdef _USE_NEWGAL
    if (colornum == 1) {
        ExpandMonoBitmap (HDC_SCREEN, pcsr->AndBits, csrimgpitch,
                        pANDBits, MONOPITCH, w, h, MYBMP_FLOW_UP,
                        RGB2Pixel (HDC_SCREEN, 0, 0, 0),
                        RGB2Pixel (HDC_SCREEN, 0xFF, 0xFF, 0xFF));
        ExpandMonoBitmap (HDC_SCREEN, pcsr->XorBits, csrimgpitch,
                        pXORBits, MONOPITCH,
                        w, h, MYBMP_FLOW_UP,
                        RGB2Pixel (HDC_SCREEN, 0, 0, 0),
                        RGB2Pixel (HDC_SCREEN, 0xFF, 0xFF, 0xFF));
    }
    else if (colornum == 4) {
        ExpandMonoBitmap (HDC_SCREEN, pcsr->AndBits, csrimgpitch,
                        pANDBits, MONOPITCH,
                        w, h, MYBMP_FLOW_UP,
                        RGB2Pixel (HDC_SCREEN, 0, 0, 0),
                        RGB2Pixel (HDC_SCREEN, 0xFF, 0xFF, 0xFF));
        Expand16CBitmap (HDC_SCREEN, pcsr->XorBits, csrimgpitch, pXORBits,
                        MONOPITCH*4, w, h, MYBMP_FLOW_UP, NULL);
    }
#else
    if(colornum == 1) {
        ExpandMonoBitmap (HDC_SCREEN, w, h, pANDBits, MONOPITCH, MYBMP_FLOW_UP,
                         pcsr->AndBits, csrimgpitch, 0, 0xFFFFFFFF);
        ExpandMonoBitmap (HDC_SCREEN, w, h, pXORBits, MONOPITCH, MYBMP_FLOW_UP,
                         pcsr->XorBits, csrimgpitch, 0, 0xFFFFFFFF);
    }
    else if(colornum == 4) {
        ExpandMonoBitmap (HDC_SCREEN, w, h, pANDBits, MONOPITCH, MYBMP_FLOW_UP,
                         pcsr->AndBits, csrimgpitch, 0, 0xFFFFFFFF);
        Expand16CBitmap (HDC_SCREEN, w, h, pXORBits, MONOPITCH*4, MYBMP_FLOW_UP,
                        pcsr->XorBits, csrimgpitch, NULL);
    }
#endif

    return (HCURSOR)pcsr;
}

/* Only called from client code. */
BOOL GUIAPI DestroyCursor(HCURSOR hcsr)
{
    int i;
    PCURSOR pcsr = (PCURSOR)hcsr;

    if (pcsr == NULL)
        return TRUE;

    for(i = 0; i <= MAX_SYSCURSORINDEX; i++)
    {
        if(pcsr == SysCursor[i])
            return FALSE;
    }

    free(pcsr->AndBits);
    free(pcsr->XorBits);
    free(pcsr);
    return TRUE;
}

/* Only called from client code, and accessed items are not changable ones. */
HCURSOR GUIAPI GetSystemCursor(int csrid)
{
    if(csrid > MAX_SYSCURSORINDEX || csrid < 0)
        return 0;

    return (HCURSOR)(SysCursor[csrid]);
}

HCURSOR GUIAPI GetDefaultCursor (void)
{
    return def_cursor;
}

#define CURSORSECTION   "cursorinfo"

BOOL realInitCursor(void)
{
    char szValue[MAX_NAME + 1];
    int number;
    int i;

#ifdef _USE_NEWGAL
    csrimgsize = GAL_GetBoxSize (__gal_screen, CURSORWIDTH, CURSORHEIGHT,
                    &csrimgpitch);

    csr_bmp.bmBitsPerPixel = __gal_screen->format->BitsPerPixel;
    csr_bmp.bmBytesPerPixel = __gal_screen->format->BytesPerPixel;
    csr_bmp.bmPitch = csrimgpitch;
#else
    csrimgpitch = CURSORWIDTH * BYTESPERPHYPIXEL;
    csrimgsize = csrimgpitch * CURSORHEIGHT;
#endif

#ifdef STATIC_SAVEDBITS

    if (csrimgsize>CURSOR_BITS_SIZE) return FALSE;
    savedbits=savedbits_mem;
    cursorbits=cursorbits_mem;


#else
    if( !(savedbits = malloc(csrimgsize)) )
        return FALSE;

    if( !(cursorbits = malloc(csrimgsize)) )
    {
        free(savedbits);
        savedbits = NULL;
        return FALSE;
    }
#endif

    if( GetMgEtcValue (CURSORSECTION, "cursornumber", szValue, 10) < 0 )
        goto error;

    number = atoi(szValue);

    if(number <= 0)
        return TRUE;

    number = number < (MAX_SYSCURSORINDEX + 1) ?
             number : (MAX_SYSCURSORINDEX + 1);

    for(i = 0; i < number; i++) {
        if ( !(SysCursor[i] = LoadSystemCursor(i)) ) {
        	dbgout("cursor #%d load error, current heap size: %d\n", i, memGetStatus(0));
             goto error;
        }
    }


    return TRUE;
error:
    TerminateCursor();
    return FALSE;
}

/* The following function must be called at last.  */
void TerminateCursor( void )
{
    int i;

    if (!savedbits ) return;

    pthread_mutex_destroy (&__mg_mouselock);

#ifdef STATIC_SAVEDBITS

#else
    free(savedbits);
    free(cursorbits);
#endif

    savedbits = NULL;
    pCurCsr = NULL;
    nShowCount = 0;

    for(i = 0; i<= MAX_SYSCURSORINDEX; i++)
    {
        if( SysCursor[i] ) {
            free(SysCursor[i]->AndBits);
            free(SysCursor[i]->XorBits);
            free(SysCursor[i]);
            SysCursor[i] = NULL;
       }
    }
}

HCURSOR GUIAPI GetCurrentCursor(void)
{
    HCURSOR hcsr;

    pthread_mutex_lock (&__mg_mouselock);

    hcsr = (HCURSOR)pCurCsr;
    pthread_mutex_unlock(&__mg_mouselock);

    return hcsr;
}

/* Cursor pointer shape and hiding and showing. */
static inline int boxleft(void)
{
    if(!pCurCsr) return -100;
    return __mg_cursor_x - pCurCsr->xhotspot;
}
static inline int boxtop(void)
{
    if(!pCurCsr) return -100;
    return __mg_cursor_y - pCurCsr->yhotspot;
}

#ifdef _USE_NEWGAL

static GAL_Rect csr_rect = {0, 0, CURSORWIDTH, CURSORHEIGHT};

static void hidecursor (void)
{
    csr_rect.x = oldboxleft;
    csr_rect.y = oldboxtop;

    csr_bmp.bmBits = savedbits;

    GAL_SetClipRect (__gal_screen, NULL);
    GAL_PutBox (__gal_screen, &csr_rect, &csr_bmp);
    GAL_UpdateRects (__gal_screen, 1, &csr_rect);
}

static void showcursor (void)
{
    int x, y;

    x = boxleft ();
    y = boxtop ();

    csr_rect.x = x;
    csr_rect.y = y;
    csr_bmp.bmBits = savedbits;

    GAL_SetClipRect (__gal_screen, NULL);
    GAL_GetBox (__gal_screen, &csr_rect, &csr_bmp);

    oldboxleft = x;
    oldboxtop = y;

    GAL_memcpy4 (cursorbits, savedbits, csrimgsize >> 2);

#ifdef ASM_memandcpy4
    ASM_memandcpy4 (cursorbits, pCurCsr->AndBits, csrimgsize >> 2);
    ASM_memxorcpy4 (cursorbits, pCurCsr->XorBits, csrimgsize >> 2);
#else
    {
        int i;
        Uint32* andbits = (Uint32*) pCurCsr->AndBits;
        Uint32* xorbits = (Uint32*) pCurCsr->XorBits;
        Uint32* dst = (Uint32*) cursorbits;

        for (i = 0; i < csrimgsize >> 2; i++) {
#if !defined(_EM86_IAL) && !defined(_EM8620_IAL)
            dst [i] &= andbits [i];
            dst [i] ^= xorbits [i];
#else
            dst [i] &= andbits [i];
#endif
            dst [i] |= __gal_screen->format->Amask;
        }
    }
#endif
    csr_bmp.bmBits = cursorbits;
    GAL_PutBox (__gal_screen, &csr_rect, &csr_bmp);
    GAL_UpdateRects (__gal_screen, 1, &csr_rect);
}

#else

static inline void hidecursor(void)
{
    GAL_SetGC(PHYSICALGC);

    GAL_EnableClipping(PHYSICALGC);
    GAL_PutBox(PHYSICALGC, oldboxleft, oldboxtop, CURSORWIDTH, CURSORHEIGHT,
            savedbits);
}

static inline void showcursor(void)
{
    int x, y;

    GAL_SetGC(PHYSICALGC);
    x = boxleft();
    y = boxtop();

    GAL_DisableClipping(PHYSICALGC);
    GAL_GetBox(PHYSICALGC, x, y, CURSORWIDTH, CURSORHEIGHT, savedbits);
    oldboxleft = x;
    oldboxtop = y;

    memcpy(cursorbits, savedbits, csrimgsize);
    {
        int i;
        Uint32* andbits = (Uint32*) pCurCsr->AndBits;
        Uint32* xorbits = (Uint32*) pCurCsr->XorBits;
        Uint32* dst = (Uint32*) cursorbits;

        for (i = 0; i < csrimgsize >> 2; i++) {
            dst [i] &= andbits [i];
            dst [i] ^= xorbits[i];
        }
    }
    GAL_EnableClipping(PHYSICALGC);
    GAL_PutBox(PHYSICALGC, x, y, CURSORWIDTH, CURSORHEIGHT, cursorbits);

}
#endif

HCURSOR GUIAPI SetCursorEx (HCURSOR hcsr, BOOL setdef)
{
    PCURSOR old, pcsr;

    pthread_mutex_lock (&__mg_mouselock);

    if (setdef) {
        old = (PCURSOR) def_cursor;
        def_cursor = hcsr;
    }
    else
        old = pCurCsr;

    if ((PCURSOR)hcsr == pCurCsr) {
        pthread_mutex_unlock(&__mg_mouselock);
        return (HCURSOR) old;
    }
    pthread_mutex_unlock(&__mg_mouselock);

    //pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    pcsr = (PCURSOR)hcsr;

    if (nShowCount >= 0 && pCurCsr)
        hidecursor();

    pCurCsr = pcsr;

    if (nShowCount >= 0 && pCurCsr)
        showcursor();

    pthread_mutex_unlock(&__mg_mouselock);
    //pthread_mutex_unlock(&__mg_gdilock);
    return (HCURSOR) old;
}

void ShowCursorForGDI (BOOL fShow, void *pdc)
{
    int csrleft, csrright, csrtop, csrbottom;
    int intleft, intright, inttop, intbottom;
    PDC cur_pdc = (PDC)pdc;
    const RECT* prc = NULL;

    prc = &cur_pdc->rc_output;

    if (cur_pdc->surface != __gal_screen) {
        if (fShow) {
#ifdef _USE_NEWGAL
            GAL_UpdateRect (cur_pdc->surface,
                            prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
        }
    }
    else {
        if (!fShow)
            pthread_mutex_lock (&__mg_mouselock);

        csrleft = boxleft();
        csrright = csrleft + CURSORWIDTH;
        csrtop = boxtop();
        csrbottom = csrtop + CURSORHEIGHT;

        intleft = (csrleft > prc->left) ? csrleft : prc->left;
        inttop  = (csrtop > prc->top) ? csrtop : prc->top;
        intright = (csrright < prc->right) ? csrright : prc->right;
        intbottom = (csrbottom < prc->bottom) ? csrbottom : prc->bottom;

        if (intleft >= intright || inttop >= intbottom) {
            if (fShow) {
#ifdef _USE_NEWGAL
                GAL_UpdateRect (cur_pdc->surface,
                                prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
                pthread_mutex_unlock(&__mg_mouselock);
            }
            return;
        }

        if (fShow && nShowCount >= 0 && pCurCsr) {
            showcursor();
        }
        if (!fShow && nShowCount >= 0 && pCurCsr) {
            hidecursor();
        }

        if (fShow) {
#ifdef _USE_NEWGAL
            GAL_UpdateRect (cur_pdc->surface,
                            prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
            pthread_mutex_unlock(&__mg_mouselock);
        }
    }
}

int GUIAPI ShowCursor(BOOL fShow)
{
    int count;

    //pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    if(fShow) {
        nShowCount++;
        if (nShowCount == 0 && pCurCsr)
            showcursor();
    }
    else {
        nShowCount--;
        if (nShowCount <= -1 && pCurCsr)
            hidecursor();
    }

    count = nShowCount;

    pthread_mutex_unlock(&__mg_mouselock);
    //pthread_mutex_unlock(&__mg_gdilock);
    return count;
}

#else

void ShowCursorForGDI (BOOL fShow, void* pdc)
{
    PDC cur_pdc = (PDC)pdc;
    const RECT* prc = NULL;

    prc = &cur_pdc->rc_output;

#ifdef _USE_NEWGAL
    if (fShow)
        GAL_UpdateRect (cur_pdc->surface,
                        prc->left, prc->top, RECTWP(prc), RECTHP(prc));
#endif
}

#endif /* _CURSOR_SUPPORT */


BOOL InitCursor (void)
{
    pthread_mutex_init (&__mg_mouselock, NULL);

#ifdef _CURSOR_SUPPORT
    return realInitCursor ();
#else
    return TRUE;
#endif
}

BOOL RefreshCursor(int* x, int* y, int* button)
{
    //pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    //IAL_GetMouseXY (x, y);		// TODO: check if necessary or not
    __mg_cursor_x = *x;
    __mg_cursor_y = *y;
    //*button = IAL_GetMouseButton ();	// TODO: check if necessary or not
    if (oldx != __mg_cursor_x || oldy != __mg_cursor_y) {
#ifdef _CURSOR_SUPPORT
        if(nShowCount >= 0 && pCurCsr) {
            hidecursor();
            showcursor();
        }
#endif
        oldx = __mg_cursor_x;
        oldy = __mg_cursor_y;

        pthread_mutex_unlock(&__mg_mouselock);
        //pthread_mutex_unlock(&__mg_gdilock);
        return TRUE;
    }

    pthread_mutex_unlock(&__mg_mouselock);
    //pthread_mutex_unlock(&__mg_gdilock);
    return FALSE;
}

/* Cursor position. */

void GUIAPI GetCursorPos(POINT* ppt)
{
    pthread_mutex_lock (&__mg_mouselock);
    ppt->x = __mg_cursor_x;
    ppt->y = __mg_cursor_y;
    pthread_mutex_unlock(&__mg_mouselock);
}

void GUIAPI SetCursorPos(int x, int y)
{
    //pthread_mutex_lock (&__mg_gdilock);
    pthread_mutex_lock (&__mg_mouselock);

    //IAL_SetMouseXY (x, y);	// TODO: check if necessary or not
    //IAL_GetMouseXY (&__mg_cursor_x, &__mg_cursor_y);

// ww 2009-3027 {
// no IAL so update the variables directly
    __mg_cursor_x=x;
    __mg_cursor_y=y;
// ww 2009-3027 }

    if (oldx != __mg_cursor_x || oldy != __mg_cursor_y) {
#ifdef _CURSOR_SUPPORT
        if(nShowCount >= 0 && pCurCsr) {
            hidecursor();
            showcursor();
        }
#endif
        oldx = __mg_cursor_x;
        oldy = __mg_cursor_y;
    }

    pthread_mutex_unlock(&__mg_mouselock);
    //pthread_mutex_unlock(&__mg_gdilock);
}

/* Cursor clipping support. */
void GUIAPI ClipCursor(const RECT* prc)
{
    RECT rc;

    pthread_mutex_lock (&__mg_mouselock);

    SetRect(&cliprc, 0, 0, WIDTHOFPHYGC - 1, HEIGHTOFPHYGC - 1);

    if(prc == NULL)
    {
        //IAL_SetMouseRange (0,0,WIDTHOFPHYGC - 1,HEIGHTOFPHYGC - 1);	// TODO: check if necessary or not
        pthread_mutex_unlock(&__mg_mouselock);
        return;
    }

    memcpy(&rc, prc, sizeof(RECT));
    NormalizeRect(&rc);
    IntersectRect(&cliprc, &rc, &cliprc);
    NormalizeRect(&cliprc);

    //IAL_SetMouseRange (cliprc.left,cliprc.top, cliprc.right,cliprc.bottom); 	// TODO: check if necessary or not

    pthread_mutex_unlock(&__mg_mouselock);
}

void GUIAPI GetClipCursor(RECT* prc)
{
    pthread_mutex_lock (&__mg_mouselock);

    if( IsRectEmpty(&cliprc) )
        SetRect(&cliprc, 0, 0, WIDTHOFPHYGC - 1, HEIGHTOFPHYGC - 1);

    memcpy(prc, &cliprc, sizeof(RECT));
    pthread_mutex_unlock(&__mg_mouselock);
}

