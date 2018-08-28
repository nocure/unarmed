/*
** $Id: misc.c 7480 2007-08-29 02:47:27Z xwyan $
**
** misc.c: This file include some miscelleous functions.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** All rights resered by Feynman Software.
**
** Create date: 1998/12/31
**
** Current maintainer: Wei Yongming.
*/

#include <string.h>
#include <stdlib.h>
#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "misc.h"


#ifdef _CLIPBOARD_SUPPORT
#include "clipboard.h"
#endif

/* Handle of MiniGUI etc file object */
GHANDLE hMgEtc = 0;


extern GHANDLE __mg_get_mgetc (void);

BOOL InitMgEtc (void)
{
    hMgEtc = __mg_get_mgetc ();

    return TRUE;
}

void TerminateMgEtc (void) { }

BOOL InitMisc (void)
{
#ifdef _CLIPBOARD_SUPPORT
    InitClipBoard ();
#endif

    return InitMgEtc ();
}

void TerminateMisc (void)
{
#ifdef _CLIPBOARD_SUPPORT
    TerminateClipBoard ();
#endif

    TerminateMgEtc ();
}

/****************************** ETC file support ******************************/

static PETCSECTION etc_NewSection (ETC_S* petc)
{
    PETCSECTION psect;

    if (petc->section_nr == petc->sect_nr_alloc) {
        /* add 5 sections each time we realloc */
        petc->sect_nr_alloc += NR_SECTS_INC_ALLOC;
        petc->sections = realloc (petc->sections,
                    sizeof (ETCSECTION)*petc->sect_nr_alloc);
    }
    psect = petc->sections + petc->section_nr;

    psect->name = NULL;

    petc->section_nr ++;

    return psect;
}

static void etc_NewKeyValue (PETCSECTION psect,
                const char* key, const char* value)
{
    if (psect->key_nr == psect->key_nr_alloc) {
        psect->key_nr_alloc += NR_KEYS_INC_ALLOC;
        psect->keys = realloc (psect->keys,
                            sizeof (char*) * psect->key_nr_alloc);
        psect->values = realloc (psect->values,
                            sizeof (char*) * psect->key_nr_alloc);
    }

    psect->keys [psect->key_nr] = FixStrDup (key);
    psect->values [psect->key_nr] = FixStrDup (value);
    psect->key_nr ++;
}

static int etc_GetSectionValue (PETCSECTION psect, const char* pKey,
                          char* pValue, int iLen)
{
    int i;

    for (i=0; i<psect->key_nr; i++) {
        if (strcmp (psect->keys [i], pKey) == 0) {
            break;
        }
    }

    if (iLen > 0) { /* get value */
        if (i >= psect->key_nr)
            return ETC_KEYNOTFOUND;

        strncpy (pValue, psect->values [i], iLen);
    }
    else { /* set value */
        if (psect->key_nr_alloc <= 0)
            return ETC_READONLYOBJ;

        if (i >= psect->key_nr) {
            etc_NewKeyValue (psect, pKey, pValue);
        }
        else {
            FreeFixStr (psect->values [i]);
            psect->values [i] = FixStrDup (pValue);
        }
    }

    return ETC_OK;
}

int GUIAPI GetValueFromEtc (GHANDLE hEtc, const char* pSection,
                            const char* pKey, char* pValue, int iLen)
{
    int i, empty_section = -1;
    ETC_S *petc = (ETC_S*) hEtc;
    PETCSECTION psect = NULL;

    if (!petc || !pValue)
        return -1;

    for (i=0; i<petc->section_nr; i++) {
        psect = petc->sections + i;
        if (!psect->name) {
           empty_section = i;
           continue;
        }

        if (strcmp (psect->name, pSection) == 0) {
            break;
        }
    }

    if (i >= petc->section_nr) {
        if (iLen > 0)
            return ETC_SECTIONNOTFOUND;
        else {
            if (petc->sect_nr_alloc <= 0)
                return ETC_READONLYOBJ;

            if (empty_section >= 0)
                psect = petc->sections + empty_section;
            else {
                psect = etc_NewSection (petc);
            }

            if (psect->name == NULL) {
                psect->key_nr = 0;
                psect->name = FixStrDup (pSection);
                psect->key_nr_alloc = NR_KEYS_INIT_ALLOC;
                psect->keys = malloc (sizeof (char* ) * NR_KEYS_INIT_ALLOC);
                psect->values = malloc (sizeof (char* ) * NR_KEYS_INIT_ALLOC);
            }
        }
    }

    return etc_GetSectionValue (psect, pKey, pValue, iLen);
}

int GUIAPI GetIntValueFromEtc (GHANDLE hEtc, const char* pSection,
                               const char* pKey, int* value)
{
    int ret;
    char szBuff [51];

    ret = GetValueFromEtc (hEtc, pSection, pKey, szBuff, 50);
    if (ret < 0) {
        return ret;
    }

    *value = strtol (szBuff, NULL, 0);
    if (*value == LONG_MIN || *value == LONG_MAX)
        return ETC_INTCONV;

    return ETC_OK;
}

GHANDLE GUIAPI FindSectionInEtc (GHANDLE hEtc,
                const char* pSection, BOOL bCreateNew)
{
    int i, empty_section = -1;
    ETC_S *petc = (ETC_S*) hEtc;
    ETCSECTION* psect = NULL;

    if (petc == NULL || pSection == NULL)
        return 0;

    for (i = 0; i < petc->section_nr; i++) {
        if (petc->sections [i].name == NULL) {
            empty_section = i;
        }
        else if (strcmp (petc->sections [i].name, pSection) == 0)
            return (GHANDLE) (petc->sections + i);
    }

    /* not found */
    if (bCreateNew) {
        if (petc->sect_nr_alloc <= 0)
            return ETC_READONLYOBJ;

        if (empty_section >= 0) {
            psect = petc->sections + empty_section;
        }
        else {
            psect = etc_NewSection (petc);
        }

        if (psect->name == NULL) {
            psect->key_nr = 0;
            psect->name = FixStrDup (pSection);
            psect->key_nr_alloc = NR_KEYS_INIT_ALLOC;
            psect->keys = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
            psect->values = malloc (sizeof (char*) * NR_KEYS_INIT_ALLOC);
        }
    }

    return (GHANDLE)psect;
}

int GUIAPI GetValueFromEtcSec (GHANDLE hSect,
                const char* pKey, char* pValue, int iLen)
{
    PETCSECTION psect = (PETCSECTION)hSect;

    if (psect == NULL)
        return ETC_INVALIDOBJ;

    return etc_GetSectionValue (psect, pKey, pValue, iLen);
}

int GUIAPI GetIntValueFromEtcSec (GHANDLE hSect, const char* pKey, int* pValue)
{
    PETCSECTION psect = (PETCSECTION)hSect;
    int ret;
    char szBuff [51];

    if (psect == NULL)
        return ETC_INVALIDOBJ;

    ret = etc_GetSectionValue (psect, pKey, szBuff, 50);
    if (ret < 0) {
        return ret;
    }

    *pValue = strtol (szBuff, NULL, 0);
    if (*pValue == LONG_MIN || *pValue == LONG_MAX)
        return ETC_INTCONV;

    return ETC_OK;
}

int GUIAPI SetValueToEtcSec (GHANDLE hSect, const char* pKey, char* pValue)
{
    PETCSECTION psect = (PETCSECTION)hSect;

    if (psect == NULL)
        return ETC_INVALIDOBJ;

    return etc_GetSectionValue (psect, pKey, pValue, -1);
}

int GUIAPI RemoveSectionInEtc (GHANDLE hEtc, const char* pSection)
{
    int i;
    PETCSECTION psect;

    if (hEtc == 0)
        return ETC_INVALIDOBJ;

    if (((ETC_S*)hEtc)->sect_nr_alloc == 0)
        return ETC_READONLYOBJ;

    psect = (PETCSECTION) FindSectionInEtc (hEtc, pSection, FALSE);
    if (psect == NULL)
        return ETC_SECTIONNOTFOUND;

    for (i = 0; i < psect->key_nr; i ++) {
        FreeFixStr (psect->keys [i]);
        FreeFixStr (psect->values [i]);
    }
    free (psect->keys);
    free (psect->values);
    FreeFixStr (psect->name);

    psect->key_nr = 0;
    psect->name = NULL;
    psect->keys = NULL;
    psect->values = NULL;

    return ETC_OK;
}

/****************************** Ping and Beep *********************************/
void GUIAPI Ping(void)
{
    //putchar ('\a');
    //fflush (stdout);
	// TODO: restore
}

char* strnchr (const char* s, size_t n, int c)
{
    size_t i;

    for (i=0; i<n; i++) {
        if ( *s == c)
            return (char *)s;

        s ++;
    }

    return NULL;
}

int substrlen (const char* text, int len, int delimiter, int* nr_delim)
{
    char* substr;

    *nr_delim = 0;

    if ( (substr = strnchr (text, len, delimiter)) == NULL)
        return len;

    len = substr - text;

    while (*substr == delimiter) {
        (*nr_delim) ++;
        substr ++;
    }

    return len;
}

char * strtrimall( char *src)
{
    int  nIndex1;
    int  nLen;

    if (src == NULL)
        return NULL;

    if (src [0] == '\0')
        return src;

    nLen = strlen (src);

    nIndex1 = 0;
    while (isspace ((int)src[nIndex1]))
        nIndex1 ++;

    if (nIndex1 == nLen) {
        *src = '\0';
        return src;
    }

    strcpy (src, src + nIndex1);

    nLen = strlen (src);
    nIndex1 = nLen - 1;
    while (isspace ((int)src[nIndex1]))
        nIndex1 --;

    src [nIndex1 + 1] = '\0';

    return src;
}



