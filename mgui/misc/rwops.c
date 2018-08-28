/*
** $Id: rwops.c 7342 2007-08-16 03:53:33Z xgwang $
**
** rwops.c:
**  Functions for reading and writing data from general sources,
**      such as file, memory, etc.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
**
** The idea and most code come from SDL - Simple DirectMedia Layer
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/12/14
**
*/

#include <string.h>
#include <platform/yl_sys.h>
#include <platform/yl_fsio.h>

#include <minigui/common.h>
#include <minigui/endianrw.h>

#define MGUI_Error(x)
#define MGUI_SetError       printf
#define MGUI_OutOfMemory()   printf ("Out of memory")

/* This file provides a general interface for SDL to read and write
   data sources.  It can easily be extended to files, memory, etc.
*/

/* Functions to read/write stdio file pointers */

static int stdio_seek(MG_RWops *context, int offset, int whence)
{
    if ( fseek(context->hidden.stdio.fp, offset, whence) == 0 ) {
        return(ftell(context->hidden.stdio.fp));
    } else {
        MGUI_Error(MGUI_EFSEEK);
        return(-1);
    }
}

static int stdio_read(MG_RWops *context, void *ptr, int size, int maxnum)
{
    size_t nread;

    nread = fread(ptr, size * maxnum, context->hidden.stdio.fp);
    //if ( nread == 0 && ferror(context->hidden.stdio.fp) ) {
    if ( nread < 0 ) {
        MGUI_Error(MGUI_EFREAD);
    }
    return(nread);
}
static int stdio_write(MG_RWops *context, const void *ptr, int size, int num)
{
    size_t nwrote;

    nwrote = fwrite(ptr, size * num, context->hidden.stdio.fp);
    //if ( nwrote == 0 && ferror(context->hidden.stdio.fp) ) {
    if ( nwrote < 0 ) {
        MGUI_Error(MGUI_EFWRITE);
    }
    return(nwrote);
}

static int stdio_close(MG_RWops *context)
{
    if ( context ) {
        if ( context->hidden.stdio.autoclose ) {
            /* WARNING:  Check the return value here! */
            fclose(context->hidden.stdio.fp);
        }
        free(context);
    }
    return(0);
}
static int stdio_eof(MG_RWops *context)
{
    return feof (context->hidden.stdio.fp);
}

/* Functions to read/write memory pointers */

static int mem_seek(MG_RWops *context, int offset, int whence)
{
    Uint8 *newpos;

    switch (whence) {
        case SEEK_SET:
            newpos = context->hidden.mem.base+offset;
            break;
        case SEEK_CUR:
            newpos = context->hidden.mem.here+offset;
            break;
        case SEEK_END:
            newpos = context->hidden.mem.stop+offset;
            break;
        default:
            MGUI_SetError("Unknown value for 'whence'\n");
            return(-1);
    }
    if ( newpos < context->hidden.mem.base ) {
        newpos = context->hidden.mem.base;
    }
    if ( newpos > context->hidden.mem.stop ) {
        newpos = context->hidden.mem.stop;
    }
    context->hidden.mem.here = newpos;
    return(context->hidden.mem.here-context->hidden.mem.base);
}
static int mem_read(MG_RWops *context, void *ptr, int size, int maxnum)
{
    int num;

    num = maxnum;
    if ( (context->hidden.mem.here + (num*size)) > context->hidden.mem.stop ) {
        num = (context->hidden.mem.stop-context->hidden.mem.here)/size;
    }
    memcpy(ptr, context->hidden.mem.here, num*size);
    context->hidden.mem.here += num*size;
    return(num);
}
static int mem_write(MG_RWops *context, const void *ptr, int size, int num)
{
    if ( (context->hidden.mem.here + (num*size)) > context->hidden.mem.stop ) {
        num = (context->hidden.mem.stop-context->hidden.mem.here)/size;
    }
    memcpy(context->hidden.mem.here, ptr, num*size);
    context->hidden.mem.here += num*size;
    return(num);
}

static int mem_close(MG_RWops *context)
{
    if ( context ) {
        free(context);
    }
    return(0);
}
static int mem_close_none (MG_RWops *context)
{
    return(0);
}
static int mem_eof(MG_RWops *context)
{
    if (context->hidden.mem.here >= context->hidden.mem.stop)
        return 1;
    return 0;
}

/* Functions to create MG_RWops structures from various data sources */
MG_RWops *MGUI_RWFromFile(const char *file, const char *mode)
{
    FILE *fp;
    MG_RWops *rwops;

    rwops = NULL;

    fp = fopen(file, (char*)mode);

    if ( fp == NULL ) {
        MGUI_SetError("Couldn't open %s\n", file);
    } else {
        rwops = MGUI_RWFromFP(fp, 1);
    }

    return(rwops);
}

MG_RWops *MGUI_RWFromFP(FILE *fp, int autoclose)
{
    MG_RWops *rwops;

    rwops = MGUI_AllocRW();
    if ( rwops != NULL ) {
        rwops->seek = stdio_seek;
        rwops->read = stdio_read;
        rwops->write = stdio_write;
        rwops->close = stdio_close;
        rwops->eof   = stdio_eof;
        rwops->hidden.stdio.fp = fp;
        rwops->hidden.stdio.autoclose = autoclose;

        rwops->type = RWAREA_TYPE_STDIO;
    }
    return(rwops);
}

MG_RWops *MGUI_RWFromMem(void *mem, int size)
{
    MG_RWops *rwops;

    rwops = MGUI_AllocRW();
    if ( rwops != NULL ) {
        rwops->seek = mem_seek;
        rwops->read = mem_read;
        rwops->write = mem_write;
        rwops->close = mem_close;
        rwops->eof   = mem_eof;
        rwops->hidden.mem.base = (Uint8 *)mem;
        rwops->hidden.mem.here = rwops->hidden.mem.base;
        if ((0xFFFFFFFF - (Uint32)mem) < size)
            rwops->hidden.mem.stop = (void*)0xFFFFFFFF;
        else
            rwops->hidden.mem.stop = rwops->hidden.mem.base+size;

        rwops->type = RWAREA_TYPE_MEM;
    }
    return(rwops);
}

void MGUI_InitMemRW (MG_RWops* area, void *mem, int size)
{
    area->seek = mem_seek;
    area->read = mem_read;
    area->write = mem_write;
    area->close = mem_close_none;
    area->eof   = mem_eof;
    area->hidden.mem.base = (Uint8 *)mem;
    area->hidden.mem.here = area->hidden.mem.base;
    if ((0xFFFFFFFF - (Uint32)mem) < size)
        area->hidden.mem.stop = (void*)0xFFFFFFFF;
    else
        area->hidden.mem.stop = area->hidden.mem.base+size;

    area->type = RWAREA_TYPE_MEM;
}

MG_RWops *MGUI_AllocRW(void)
{
    MG_RWops *area;

    area = (MG_RWops *)malloc(sizeof *area);
    if ( area == NULL ) {
        MGUI_OutOfMemory();
    }
    else
        area->type = RWAREA_TYPE_UNKNOWN;

    return(area);
}

void MGUI_FreeRW(MG_RWops *area)
{
    free(area);
}

int MGUI_RWgetc (MG_RWops* area)
{
    unsigned char c;

    if (MGUI_RWread (area, &c, 1, 1) == 0)
        return EOF;

    return c;
}


