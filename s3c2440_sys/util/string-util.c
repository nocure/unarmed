#include "platform/YL_SYS.h"
#include <platform/yl_FSIO.h>
#include <string.h>

#if 0
/* static function delaration */
static char *_strtok_r(char *s, const char *delim, char **lasts, int skip_leading_delim) {
	char *spanp;
	int c, sc;
	char *tok;

	if (s == NULL && (s = *lasts) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
		if (c == sc) {
			if (skip_leading_delim) {
				goto cont;
			}
			else {
				*lasts = s;
				s[-1] = 0;
				return (s - 1);
			}
		}
	}

	if (c == 0) {		/* no non-delimiter characters */
		*lasts = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (char *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*lasts = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}


char *yl_strtok(char *s, const char *delim)
{
	__REENT_STRTOK_LAST = NULL;
    return _strtok_r (s, delim, &(__REENT_STRTOK_LAST), 1);
}

char *yl_strtok_r(char *s, const char *delim, char **lasts)
{
	return _strtok_r (s, delim, lasts, 1);
}
#endif


extern char *__strtok_r (char *, const char *, char **, int);
char *strtok (char *s, const char *delim) {
	return __strtok_r (s, delim, tsk_var(TSK_STRTOK), 1);
}


char *yl_strdup(const char *str)
{
	size_t len = 0;
	char *copy= NULL;

	len = strlen(str) + 1;
	copy = (char *)malloc(len);

	if (copy) {
		memcpy(copy, str, len);
	}

	return copy;
}

ucs2_t* yl_wconcat(ucs2_t* absPath,const ucs2_t* relPath)
{
	if(!relPath||!*relPath)
		return NULL;
	ucs2_t *pstart = absPath;
	while (*absPath)
	    absPath++;

  	while (*relPath){
		*absPath = *relPath;
		absPath++;
		relPath++;
  	}
	*absPath = 0;

  	return pstart;
}


ucs2_t* yl_wstrcpy(ucs2_t* dst,const ucs2_t* src)
{
	if(!src||!dst)
		return NULL;

	while (*src)
		*dst++ = *src++;

	*dst=0;

  	return dst;
}


int yl_wstrlen(const ucs2_t* uni)
{
	int count = 0;
	while (*uni){
	    uni++;
		count++;
	}
	return count;
}


ucs2_t *yl_wstrdup(const ucs2_t *str)
{
	size_t len = 0;
	ucs2_t *copy= NULL;

	len = wcslen(str) + 1;
	copy = (ucs2_t *)malloc(len*2);

	if (copy) {
		memcpy(copy, str, len*2);
	}

	return copy;
}

// =========================================================================
int yl_strtoaddr(char* str,DWORD *value) {
char *ep;
DWORD addr;

	addr=strtoul(str,&ep,0);
	if (*ep) {
		printf("Input error: %s\n",str);
		return -1;
	}
	if (
			((addr>=0x00000000)&&(addr<0x00001000))	// iram area 0 - 0x1000
		||	((addr>=0x30000000)&&(addr<0x34000000))	// sdram area
		||	((addr>=0x48000000)&&(addr<0x5c000000))	// register area
)	{
		*value=addr;
		return 0;
	}

	printf("Input address out of range: %s\n",str);
	return -1;
}

int yl_strtol(char* str,int *value) {
char *ep;

	*value=strtol(str,&ep,0);
	if (*ep) {
		printf("Input error: %s\n",str);
		return -1;
	}
	return 0;
}


int yl_strtoul(char* str,DWORD *value) {
char *ep;

	*value=strtoul(str,&ep,0);
	if (*ep) {
		printf("Input error: %s\n",str);
		return -1;
	}
	return 0;
}


