/*
** $Id: coor.c 7359 2007-08-16 05:08:40Z xgwang $
**
** Coordinates operations of GDI.
**
** Copyright (C) 2003 ~ 2007 Feynman Software
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/06/12, derived from original gdi.c
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

/************************ Coordinate system transfer *************************/
void GUIAPI SPtoLP(HDC hdc, POINT* pPt)
{
    if (hdc == HDC_SCREEN) return;

    coor_SP2LP ((PDC)hdc, &pPt->x, &pPt->y);
}

void GUIAPI LPtoSP(HDC hdc, POINT* pPt)
{
    if (hdc == HDC_SCREEN) return;

    coor_LP2SP ((PDC)hdc, &pPt->x, &pPt->y);
}

void GUIAPI LPtoDP(HDC hdc, POINT* pPt)
{
    PDC pdc;

    pdc = dc_HDC2PDC(hdc);

    if (pdc->mapmode != MM_TEXT) {
        pPt->x = (pPt->x - pdc->WindowOrig.x)
             * pdc->ViewExtent.x / pdc->WindowExtent.x
             + pdc->ViewOrig.x;

        pPt->y = (pPt->y - pdc->WindowOrig.y)
             * pdc->ViewExtent.y / pdc->WindowExtent.y
             + pdc->ViewOrig.y;
    }
}

void GUIAPI DPtoLP (HDC hdc, POINT* pPt)
{
    PDC pdc;

    pdc = dc_HDC2PDC (hdc);

    if (pdc->mapmode != MM_TEXT) {
        pPt->x = (pPt->x - pdc->ViewOrig.x)
             * pdc->WindowExtent.x / pdc->ViewExtent.x
             + pdc->WindowOrig.x;

        pPt->y = (pPt->y - pdc->ViewOrig.y)
             * pdc->WindowExtent.y / pdc->ViewExtent.y
             + pdc->WindowOrig.y;
    }
}

