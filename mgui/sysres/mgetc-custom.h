/*
 * mgetc-custom.c
 * system configuration file for customer's board.
 */

#ifndef _SYS_CFG_INCLUDED
    #define _SYS_CFG_INCLUDED

static char *SYSTEMFONT_KEYS[] =
{"font_number", "font0", "default", "wchar_def", "fixed", "caption", "menu", "control"};

static char *SYSTEMFONT_VALUES[] =
{
    "1",
    "rbf-fixed-rrncnn-6-12-ISO8859-1",	// font0
//    "rbf-fixed-rrncnn-8-16-ISO8859-1",	// font1
    "0", "0", "0", "0", "0", "0"
};

#endif /* !_SYS_CFG_INCLUDED */
