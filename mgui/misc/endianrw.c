/*
** $Id: endianrw.c 7342 2007-08-16 03:53:33Z xgwang $
**
** endianrw.c:
**  Functions for dynamically reading and writing endian-specific values
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 2001 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
**
** The idea and some code come from SDL - Simple DirectMedia Layer
**
** Current maintainer: Wei Yongming.
**
** Create date: 2001/09/18
*/

#include <platform/yl_sys.h>
#include <platform/yl_fsio.h>

#include <minigui/common.h>
#include <minigui/endianrw.h>

Uint16 MGUI_ReadLE16 (MG_RWops *src)
{
	Uint16 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapLE16(value));
}

Uint16 MGUI_ReadBE16 (MG_RWops *src)
{
	Uint16 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapBE16(value));
}

Uint32 MGUI_ReadLE32 (MG_RWops *src)
{
	Uint32 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapLE32(value));
}

Uint32 MGUI_ReadBE32 (MG_RWops *src)
{
	Uint32 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapBE32(value));
}

Uint64 MGUI_ReadLE64 (MG_RWops *src)
{
	Uint64 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapLE64(value));
}

Uint64 MGUI_ReadBE64 (MG_RWops *src)
{
	Uint64 value;

	MGUI_RWread(src, &value, (sizeof value), 1);
	return(ArchSwapBE64(value));
}

int MGUI_WriteLE16 (MG_RWops *dst, Uint16 value)
{
	value = ArchSwapLE16(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

int MGUI_WriteBE16 (MG_RWops *dst, Uint16 value)
{
	value = ArchSwapBE16(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

int MGUI_WriteLE32 (MG_RWops *dst, Uint32 value)
{
	value = ArchSwapLE32(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

int MGUI_WriteBE32 (MG_RWops *dst, Uint32 value)
{
	value = ArchSwapBE32(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

int MGUI_WriteLE64 (MG_RWops *dst, Uint64 value)
{
	value = ArchSwapLE64(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

int MGUI_WriteBE64 (MG_RWops *dst, Uint64 value)
{
	value = ArchSwapBE64(value);
	return(MGUI_RWwrite(dst, &value, (sizeof value), 1));
}

Uint16 MGUI_ReadLE16FP (FILE *src)
{
	Uint16 value;

	fread (&value, (sizeof value), src);
	return(ArchSwapLE16(value));
}

Uint32 MGUI_ReadLE32FP (FILE *src)
{
	Uint32 value;

	fread(&value, (sizeof value), src);
	return(ArchSwapLE32(value));
}

int MGUI_WriteLE16FP (FILE *dst, Uint16 value)
{
	value = ArchSwapLE16(value);
	return(fwrite (&value, (sizeof value), dst));
}

int MGUI_WriteLE32FP (FILE *dst, Uint32 value)
{
	value = ArchSwapLE32(value);
	return(fwrite (&value, (sizeof value), dst));
}

