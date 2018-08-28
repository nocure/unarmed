#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include "platform/YL_I18N.h"

#include "converters.h"

static CPInfo sysdep_cpinfo[] = {
#define MAP(cpnum, name, prefix) {cpnum, (void *)prefix##_mbtowc, (void *)prefix##_wctomb},
#include "CodePageMap.h"
#undef MAP
};

// ==== debug =================================================
#if defined(DEBUG)
void printAllCodePageInfo (void)
{
	CPInfo *ptr = NULL;
	DWORD	count = sizeof(sysdep_cpinfo)/sizeof(sysdep_cpinfo[0]);

	for (ptr = sysdep_cpinfo; count > 0; ptr ++, count --) {
//		dbgout("%d: code name: %s\n", count, ptr->canonicalName);
		dbgout("%d: code page: %d\n", count, ptr->codePage);
		dbgout("%d: mbtowc: 0x%08x\n", count, ptr->mbtowc);
		dbgout("%d: wctomb: 0x%08x\n", count, ptr->wctomb);
	}
}
#endif

static int IsValidCodePage(int CodePage)
{
	CPInfo *ptr = NULL;
	DWORD	count = sizeof(sysdep_cpinfo)/sizeof(sysdep_cpinfo[0]);

	for (ptr = sysdep_cpinfo; count > 0; ptr ++, count --) {
		if (ptr->codePage == CodePage)
			return TRUE;
	}

	return FALSE;
}

static CPInfo * GetCPInfo(int CodePage)
{
	CPInfo *ptr = NULL;
	DWORD	count = sizeof(sysdep_cpinfo)/sizeof(sysdep_cpinfo[0]);

	for (ptr = sysdep_cpinfo; count > 0; ptr ++, count --) {
		if (ptr->codePage == CodePage)
			return ptr;
	}

	return NULL;
}

// ==== UCS2 <--> MultiBytes conversion =======================
/* input:
 * 		[in]CodePage		equivalent to Windows Code Page, ref the CodePage.map
 * 		[in]dwFlags			if SKIP_INVALID_CHARS set, replace the invalid char to '_'
 * 							if 0, return error code.
 * 		[in]ucs2str			pointer to the UCS-2 string to be converted.
 * 		[in]ucslen			size in ucs2_t values, of the UCS-2 string indicated by ucs2str. If -1 is set,
 * 							the length would be calculated automatically. If 0, the function fails.
 * 		[out]mbstr			poiter to a buffer that receives the converted string.
 * 		[in]mblen			size in bytes, of the buffer indicated by mbstr. If set to 0, the function return
 * 							the required buffer size and make no use of the pointer of mbstr.
 * output:
 * 		return the number of bytes written to the buffer. Also include the byte of terminatin NULL.
 * 		If fail, the error code YLERR_E2BIG indicate the buffer is too small.
 * 		YLERR_INVAL indicate the invalid parameter
 */
int
UCS2ToMultiByte(DWORD CodePage, DWORD dwFlags, const ucs2_t *ucs2str, int ucslen, char *mbstr, int mblen)
{
	DWORD	ulen = 0;
	CPInfo	*cptr = NULL;
	int		bytes = 0, szEst = 0, byteConsumed = 0, byteLeft = 0;
	char	*buffer = NULL, *pbuf = NULL;
	int		retval = YLERR_OK;

	if (NULL == ucs2str || 0 == ucslen)
		return YLERR_INVAL;

	if (!IsValidCodePage(CodePage))
		return YLERR_INVAL;

	if (ucslen < 0)		// calculate the ucs2str size automatically
		ulen = wcslen(ucs2str);
	else
		ulen = ucslen;

	szEst = ulen * 8;			// for worst case conversion
	byteLeft = szEst;
	pbuf = buffer = malloc(szEst);
	cptr = GetCPInfo(CodePage);

	while (*ucs2str) {
		wchar_t ch = *ucs2str;
		bytes = cptr->wctomb((BYTE*)pbuf, ch, byteLeft);	//(BYTE*) Jon modified.

		if (bytes < 0) {
			if (dwFlags & SKIP_INVALID_CHARS) {
				*pbuf = '_';
				bytes = 1;
			}
			else {
				retval = YLERR_INVAL;
				goto done;
			}
		}

		byteLeft -= bytes;
		pbuf += bytes;
		ucs2str ++;
	}
	*pbuf = '\0';

	byteConsumed = (szEst - byteLeft) + 1;		// plus the terminating NULL

	if (0 == mblen || NULL == mbstr) {
		retval = byteConsumed;
	}
	else {
		if (byteConsumed > mblen)
			retval = YLERR_E2BIG;

		memcpy(mbstr, buffer, byteConsumed);
		retval = byteConsumed;
	}
done:
	if (buffer)	free(buffer);
	return	retval;
}

/* input:
 * 		[in]CodePage		equivalent to Windows Code Page, ref the CodePage.map
 * 		[in]mbstr			pointer to the multi byte string to be converted.
 * 		[in]mblen			size in bytes, of the multi byte string indicated by mbstr. If -1 is set,
 * 							the length would be calculated automatically. If 0, the function fails.
 * 		[out]ucs2str		poiter to a buffer that receives the converted string.
 * 		[in]ucslen			size in ucs2_t values, of the buffer indicated by ucs2buf. If set to 0, the
 * 							function return the required buffer size in ucs2_t ,and make no use of the
 * 							pointer of mbstr.
 * output:
 * 		return the number of bytes written to the buffer. Also include the byte of terminatin NULL.
 * 		If fail, the error code YLERR_E2BIG indicate the buffer is too small.
 * 		YLERR_INVAL indicate the invalid parameter
 */
int
MultiByteToUCS2(DWORD CodePage, const char *mbstr, int mblen, ucs2_t *ucs2buf, int ucslen)
{
	DWORD	len = 0;
	CPInfo	*cptr = NULL;
	int		bytes = 0, byteLeft = 0;
	int		consumed = 0;
	ucs2_t	*buffer = NULL, *pbuf = NULL;
	int		retval = YLERR_OK;

	if (NULL == mbstr || 0 == mblen)
		return YLERR_INVAL;

	if (!IsValidCodePage(CodePage))
		return YLERR_INVAL;

	if (mblen < 0)			// calculate the mbstr size automatically
		len = strlen(mbstr);
	else
		len = mblen;

	byteLeft = len;
	pbuf = buffer = malloc(len * sizeof(ucs2_t));
	cptr = GetCPInfo(CodePage);

	while (*mbstr) {
		wchar_t ch = 0;
		bytes = cptr->mbtowc(&ch, (BYTE*)mbstr, byteLeft);	//(BYTE*) Jon modified.

		*pbuf = (ucs2_t)ch;

		if (bytes < 0) {
			retval = YLERR_INVAL;
			goto done;
		}

		byteLeft -= bytes;
		pbuf ++;
		mbstr += bytes;
	}
	*pbuf = 0;

	consumed = wcslen(buffer) + 1;		// plus the terminating NULL

	if (0 == ucslen || NULL == ucs2buf) {
		retval = consumed;
	}
	else {
		if (consumed > ucslen)
			retval = YLERR_E2BIG;

		memcpy(ucs2buf, buffer, consumed * sizeof(ucs2_t));
		retval = consumed;
	}
done:
	if (buffer)	free(buffer);
	return	retval;
}

// ==== ascii =================================================

int ASCIItoMBS(ucs2_t *dest_mbs, const char *src_str, size_t n)
{
	if (NULL == dest_mbs || NULL == src_str || 0 == n)
		return YLERR_INVAL;

	while (n > 0 && *src_str) {
		ucs2_t	ch = (ucs2_t)*src_str;
		*dest_mbs = ch;

		n --;
		dest_mbs ++;
		src_str ++;
	}
	*dest_mbs = 0;

	if (n <= 0 && *src_str)
		return YLERR_E2BIG;

	return YLERR_OK;
}


int MBStoASCII(char *dest_str, const ucs2_t *src_mbs, size_t n)
{
	int	bytes = 0;

	if (NULL == dest_str || NULL == src_mbs || 0 == n)
		return YLERR_INVAL;

	while (n > 0 && *src_mbs) {
		wchar_t ch = *src_mbs;
		bytes = ascii_wctomb((BYTE*)dest_str, ch, n);	//(BYTE*) Jon modified.

		if (bytes < 0) {
			return YLERR_INVAL;
		}

		n -= bytes;
		dest_str += bytes;
		src_mbs ++;
	}
	*dest_str = '\0';

	if (n <= 0 && *src_mbs)
		return YLERR_E2BIG;

	return YLERR_OK;
}


// ==== UCS Little Endian =========================================

/* FILEX APIs use Little Endian string as arguments */
int LESTRtoMBS(ucs2_t *dest_mbs, const BYTE *src_str, size_t n)
{
	int	bytes = 0;

	if (NULL == dest_mbs || NULL == src_str || 0 == n)
		return YLERR_INVAL;

	while (n > 0 && *src_str) {
		wchar_t ch = 0;
		bytes = ucs2le_mbtowc(&ch, src_str, n * 2);
		*dest_mbs = (ucs2_t)ch;

		if (bytes < 0) {
			return YLERR_INVAL;
		}

		n --;
		dest_mbs ++;
		src_str += bytes;
	}
	*dest_mbs = 0;

	if (n <= 0 && *src_str)
		return YLERR_E2BIG;

	return YLERR_OK;
}


int MBStoLESTR(BYTE *dest_str, const ucs2_t *src_mbs, size_t n)
{
	int	bytes = 0;

	if (NULL == dest_str || NULL == src_mbs || 0 == n)
		return YLERR_INVAL;

	while (n > 0 && *src_mbs) {
		wchar_t ch = *src_mbs;
		bytes = ucs2le_wctomb(dest_str, ch, n);

		if (bytes < 0) {
			return YLERR_INVAL;
		}

		n -= bytes;
		dest_str += bytes;
		src_mbs ++;
	}
	dest_str[0] = '\0';
	dest_str[1] = '\0';

	if (n <= 0 && *src_mbs)
		return YLERR_E2BIG;

	return YLERR_OK;
}


// ==== UTF8 =========================================

int FXUnicode2UTF8(BYTE *dest_str, const BYTE *src_str, size_t n)
{
	int	bytes = 0;

	if (NULL == dest_str || NULL == src_str || 0 == n)
		return YLERR_INVAL;

	while (n > 0 && (src_str[0] || src_str[1])) {
		wchar_t ch = *src_str;
		bytes = ucs2le_mbtowc(&ch, src_str, n);
		if (bytes < 0) {
			return YLERR_INVAL;
		}

		src_str += bytes;

		bytes = utf8_wctomb(dest_str, ch, n);
		if (bytes < 0) {
			return YLERR_INVAL;
		}

		n -= bytes;
		dest_str += bytes;
	}
	dest_str[0] = '\0';

	if (n <= 0 && *src_str)
		return YLERR_E2BIG;

	return YLERR_OK;
}


// ==== BIG5 2003 ====================================

int big5str_to_fx_ucs2(BYTE **inbuf, size_t *inbytesleft,
		BYTE **outbuf, size_t *outbytesleft) {

	if (inbuf == NULL || *inbuf == NULL || *inbytesleft == 0) {
		return YLERR_INVAL;
	}

	if (*outbytesleft == 0 || *outbuf == NULL) {
		return YLERR_E2BIG;
	}

	while (*inbytesleft > 0) {
		size_t bytes = 0;
		wchar_t ch = 0;

		if (*outbytesleft == 0) {
			return YLERR_E2BIG;
		}

		bytes = cp950_mbtowc(&ch, *inbuf, *inbytesleft);

		if (bytes < 0) {
			return YLERR_INVAL;
		}
		*inbuf += bytes;
		*inbytesleft -= bytes;

		bytes = ucs2le_wctomb(*outbuf, ch, *outbytesleft);
		if (RET_ILUNI == bytes) {		/* invalid character */
			bytes = ucs2le_wctomb(*outbuf, (wchar_t)DEFAULT_CHARACTER, *outbytesleft);
			if (bytes < 0) {
				return YLERR_E2BIG;
			}
		}
		*outbuf += bytes;
		*outbytesleft -= bytes;
	}
	return YLERR_OK;
}


int  fx_ucs2_to_big5str(BYTE **inbuf, size_t *inbytesleft,
		BYTE **outbuf, size_t *outbytesleft) {


	if (inbuf == NULL || *inbuf == NULL || *inbytesleft == 0) {
		return YLERR_INVAL;
	}

	if (*outbytesleft == 0 || *outbuf == NULL) {
		return YLERR_E2BIG;
	}

	while (*inbytesleft > 0) {
		size_t bytes = 0;
		wchar_t ch = 0;

		if (*outbytesleft == 0) {
			return YLERR_E2BIG;
		}

		bytes = ucs2le_mbtowc(&ch, *inbuf, *inbytesleft);

		if (bytes < 0) {
			return YLERR_INVAL;
		}
		*inbuf += bytes;
		*inbytesleft -= bytes;

		bytes = cp950_wctomb(*outbuf, ch, *outbytesleft);
		if (RET_ILUNI == bytes) {		/* invalid character */
			bytes = cp950_wctomb(*outbuf, (wchar_t)DEFAULT_CHARACTER, *outbytesleft);
			if (bytes < 0) {
				return YLERR_E2BIG;
			}
		}
		*outbuf += bytes;
		*outbytesleft -= bytes;
	}
	return YLERR_OK;
}




