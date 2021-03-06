/*
** $Id: pixel.c 7359 2007-08-16 05:08:40Z xgwang $
**
** pixel.c: drawing of pixel.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/10/18, derived from original draw.c
*/

#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "cliprect.h"
#include "gal.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "pixel_ops.h"
#include "cursor.h"

/*********************** Generl drawing support ******************************/
void _set_pixel_helper (PDC pdc, int x, int y)
{
    SetRect (&pdc->rc_output, x - 1, y - 1, x + 1, y + 1);

    ENTER_DRAWING (pdc);

    if (PtInRect (&pdc->rc_output, x, y) && PtInRegion (&pdc->ecrgn, x, y)) {
        pdc->move_to (pdc, x, y);
        pdc->set_pixel (pdc);
    }

    LEAVE_DRAWING (pdc);
}

static void _my_set_pixel (PDC pdc, int x, int y, gal_pixel pixel)
{
    if (dc_IsGeneralDC (pdc)) {
        LOCK_GCRINFO (pdc);
        if (!dc_GenerateECRgn (pdc, FALSE)) {
            UNLOCK_GCRINFO (pdc);
            return;
        }
    }

    coor_LP2SP (pdc, &x, &y);
    pdc->cur_pixel = pixel;
    _set_pixel_helper (pdc, x, y);

    UNLOCK_GCRINFO (pdc);
}

void GUIAPI SetPixel (HDC hdc, int x, int y, gal_pixel pixel)
{
    _my_set_pixel (dc_HDC2PDC (hdc), x, y, pixel);
}

gal_pixel GUIAPI SetPixelRGB (HDC hdc, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = GAL_MapRGB (pdc->surface->format, r, g, b);
    _my_set_pixel (pdc, x, y, pixel);

    return pixel;
}

gal_pixel GUIAPI SetPixelRGBA (HDC hdc, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = GAL_MapRGBA (pdc->surface->format, r, g, b, a);
    _my_set_pixel (pdc, x, y, pixel);

    return pixel;
}

/* FIXME: I do not know why this can not work in MiniGUI-Lite. :( */
gal_pixel _dc_get_pixel_cursor (PDC pdc, int x, int y)
{
    gal_pixel pixel = 0;
    Uint8* dst = NULL;

    if (x >= pdc->DevRC.left && y >= pdc->DevRC.top
                && x < pdc->DevRC.right && y < pdc->DevRC.bottom) {
#ifdef _CURSOR_SUPPORT
        if (!dc_IsMemDC (pdc)) {
            dst = GetPixelUnderCursor (x, y, &pixel);
        }

        if (dst == NULL) {
#endif
            dst = _dc_get_dst (pdc, x, y);
            pixel = _mem_get_pixel (dst, pdc->surface->format->BytesPerPixel);
#ifdef _CURSOR_SUPPORT
        }
#endif
    }

    return pixel;
}

static gal_pixel get_pixel (PDC pdc, int x, int y)
{
    gal_pixel pixel;

    if (dc_IsGeneralDC (pdc)) {
        LOCK_GCRINFO (pdc);
        dc_GenerateECRgn (pdc, FALSE);
    }

    coor_LP2SP (pdc, &x, &y);
    pixel = _dc_get_pixel_cursor (pdc, x, y);

    UNLOCK_GCRINFO (pdc);

    return pixel;
}

gal_pixel GUIAPI GetPixel (HDC hdc, int x, int y)
{
    return get_pixel (dc_HDC2PDC (hdc), x, y);
}

gal_pixel GUIAPI GetPixelRGB (HDC hdc, int x, int y, Uint8* r, Uint8* g, Uint8* b)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = get_pixel (pdc, x, y);
    GAL_GetRGB (pixel, pdc->surface->format, r, g, b);

    return pixel;
}

gal_pixel GUIAPI GetPixelRGBA (HDC hdc, int x, int y, Uint8* r, Uint8* g, Uint8* b, Uint8* a)
{
    PDC pdc;
    gal_pixel pixel;

    pdc = dc_HDC2PDC (hdc);
    pixel = get_pixel (pdc, x, y);
    GAL_GetRGBA (pixel, pdc->surface->format, r, g, b, a);

    return pixel;
}

gal_pixel GUIAPI RGB2Pixel (HDC hdc, Uint8 r, Uint8 g, Uint8 b)
{
    return GAL_MapRGB (dc_HDC2PDC (hdc)->surface->format, r, g, b);
}

gal_pixel GUIAPI RGBA2Pixel (HDC hdc, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return GAL_MapRGBA (dc_HDC2PDC (hdc)->surface->format, r, g, b, a);
}

void GUIAPI Pixel2RGB (HDC hdc, gal_pixel pixel, Uint8 *r, Uint8 *g, Uint8 *b)
{
    GAL_GetRGB (pixel, dc_HDC2PDC (hdc)->surface->format, r, g, b);
}

void GUIAPI Pixel2RGBA (HDC hdc, gal_pixel pixel, Uint8 *r, Uint8 *g, 
                Uint8 *b, Uint8 *a)
{
    GAL_GetRGBA (pixel, dc_HDC2PDC (hdc)->surface->format, r, g, b, a);
}

