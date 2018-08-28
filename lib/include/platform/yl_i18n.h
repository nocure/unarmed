#ifndef YL_I18N_H_
#define YL_I18N_H_

/* chris: from libiconv, used for big5<-->unicode(UCS-2) conversion. */
/* Return code if invalid.*/
#define RET_ILSEQ      -1
/* Return code if only a shift sequence of n bytes was read. */
#define RET_TOOFEW(n)  (-2-(n))
/* Return code if invalid.*/
#define RET_ILUNI      -1
/* Return code if output buffer is too small. */
#define RET_TOOSMALL   -2
#define DEFAULT_CHARACTER 0x3f /* ASCII '?' */

typedef struct _CPInfo {
	unsigned int	codePage;
//	char			canonicalName[16];
	int 			(*mbtowc) (wchar_t *pwc, const unsigned char *s, int n);
	int 			(*wctomb) (unsigned char *r, wchar_t wc, int n);
} CPInfo;

typedef struct {
	unsigned short indx; /* index into big table */
	unsigned short used; /* bitmask of used entries */
} Summary16;

// ==== debug =================================================
#define	DEBUG
#if defined(DEBUG)
void printAllCodePageInfo (void);
#endif

#define	SKIP_INVALID_CHARS	0x01		/* replace invalid char with '_' and return YLERR_OK */
// ==== UCS2 <--> MultiBytes conversion =======================
int UCS2ToMultiByte(uint32 CodePage, DWORD dwFlags, const ucs2_t *ucs2str, int ucslen, char *mbstr, int mblen);
int MultiByteToUCS2(uint32 CodePage, const char *mbstr, int mblen, ucs2_t *ucs2buf, int ucslen);

// ==== ascii =================================================
int ASCIItoMBS(ucs2_t *dest_mbs, const char *src_str, size_t dest_size);
int MBStoASCII(char *dest_str, const ucs2_t *src_mbs, size_t dest_size);

// ==== UCS Little Endian =========================================
int LESTRtoMBS(ucs2_t *dest_mbs, const BYTE *src_str, size_t dest_size);
int MBStoLESTR(BYTE *dest_str, const ucs2_t *src_mbs, size_t dest_size);

// ==== UTF8 =========================================
int FXUnicode2UTF8(BYTE *dest_str, const BYTE *src_str, size_t dest_size);

// ==== BIG5 2003 ====================================
int big5str_to_fx_ucs2(BYTE **inbuf, size_t *inlen, BYTE **outbuf, size_t *outlen);
int fx_ucs2_to_big5str(BYTE **inbuf, size_t *inlen, BYTE **outbuf, size_t *outlen);

#endif /*YL_I18N_H_*/
