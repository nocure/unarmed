#include "platform/YL_FSIO.h"
#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include <platform/YL_I18N.h>
//#include <fx_api.h>

#if 0
#define FFBLK_MAGIC			0x77884499


/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
static unsigned long __mktime(struct tm *t)
		/*
		const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)*/
{
        unsigned int mon = t->tm_mon, year = t->tm_year;

        /* 1..12 -> 11,12,1..10 */
        if (0 >= (int) (mon -= 2)) {
                mon += 12;      /* Puts Feb last since it has leap day */
                year -= 1;
        }

        return ((((unsigned long)
                  (year/4 - year/100 + year/400 + 367*mon/12 + t->tm_mday) +
                  year*365 - 719499
            )*24 + t->tm_hour /* now have hours */
          )*60 + t->tm_min /* now have minutes */
        )*60 + t->tm_sec; /* finally seconds */
}


static int match_conditions(ffblk *pff, char *fname, char* uname, ULONG* pulen) {
	UINT status;
	int	nomatch,i;
	char *pptn;
	UINT	uname_rc;

	nomatch = FNM_NOMATCH;

	uname_rc = fx_unicode_name_get(pff->pfx_media, fname, (UCHAR*)uname, pulen); // (UCHAR*) Jon modified.

	for (i = 0, pptn = pff->rule_set;; i++, pptn = NULL) {
		char *cmp_name,*rule_indv/*,*saveptr*/;
		UCHAR utf8str[*pulen * 6];

		// ww 2010-0104 fixed reent -->
//		rule_indv = strtok_r(pptn, ";", &saveptr);
		rule_indv = strtok(pptn, ";");
		// ww 2010-0104 fixed reent <--
		if (NULL == rule_indv)
			break;

		/* unicode convert to UTF-8 for wildcmp */
		if (FX_SUCCESS == uname_rc) {
			FXUnicode2UTF8(utf8str, (BYTE*)uname, *pulen * 6);
			cmp_name = (char*)utf8str;
		}
		else {
			cmp_name = fname;
		}

		if (rule_indv[0] == '*' &&  ((rule_indv[1] == 0) || (rule_indv[1] == '.' && rule_indv[2] == '*' && strlen(rule_indv) == 3)) ) {
			nomatch = 0;
			break;
		}
		else
			if (wildcmp(rule_indv,cmp_name)) nomatch =0;
	}

	if (nomatch) return FNM_NOMATCH;
	status = fx_directory_information_get(pff->pfx_media, fname, (UINT*)&pff->ff_attr, (ULONG*)&pff->ff_size, (UINT*)&pff->ff_time.tm_year, (UINT*)&pff->ff_time.tm_mon, (UINT*)&pff->ff_time.tm_mday, (UINT*)&pff->ff_time.tm_hour, (UINT*)&pff->ff_time.tm_min, (UINT*)&pff->ff_time.tm_sec);

	if (!status) {
		DWORD file_time;
		DWORD comp_time;
		file_time=__mktime(&pff->ff_time);
		comp_time=__mktime(&pff->time);

		if (pff->attr & FF_TIME_AFTER) {
			if (file_time < comp_time) nomatch|=FNM_NOMATCH;
		}
		else if (pff->attr & FF_TIME_BEFORE) {
			if (file_time >= comp_time) nomatch|=FNM_NOMATCH;
		}

		if (pff->attr & FF_SIZE_BIGGER) {
			if (pff->ff_size < pff->size) nomatch|=FNM_NOMATCH;
		}
		else if (pff->attr & FF_SIZE_SMALLER) {
			if (pff->ff_size >= pff->size) nomatch|=FNM_NOMATCH;
		}

		if (pff->attr & FF_ATTR_AND) {
			if ((pff->ff_attr&pff->attr)!=pff->attr) nomatch|=FNM_NOMATCH;
		}
		else if (pff->attr & FF_ATTR_OR) {
			if (!(pff->ff_attr&pff->attr)) nomatch|=FNM_NOMATCH;
		}

	}
	return nomatch;
}


static void check_unicode_name(ffblk *pff, char *fname, char* uname, ULONG ulen) {
	strcpy(pff->ff_name,fname);	// ww modified

	if (ulen > 0) {
		pff->ff_hasuname = FX_TRUE;
		LESTRtoMBS(pff->ff_longname, (BYTE*)uname, MAX_PATH_LENGTH); // (BYTE*) Jon modified.
	}
	else {
		pff->ff_hasuname = FX_FALSE;
		ASCIItoMBS(pff->ff_longname, fname, MAX_PATH_LENGTH);
	}
}


static void save_find_state(ffblk *pff) {
	pff->entry_offset = ((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_current_entry;
	if (((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_string[0]) {
		pff->cwd_num_entries = ((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_directory.fx_dir_entry_file_size;
	}
	else {	/* root directory */
		pff->cwd_num_entries = ((FX_MEDIA*)pff->pfx_media)->fx_media_root_directory_entries;
	}
}

static void restore_find_state(ffblk *pff) {
	((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_current_entry = pff->entry_offset;
	if (((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_string[0]) {
		((FX_LOCAL_PATH*)pff->pfx_path)->fx_path_directory.fx_dir_entry_file_size = pff->cwd_num_entries;
	}
	else {	/* root directory */
		((FX_MEDIA*)pff->pfx_media)->fx_media_root_directory_entries = pff->cwd_num_entries;
	}
}
#endif

ffblk* find_init(void) {
#if 0
ffblk* pff;
void* p;

	p=calloc(1,sizeof(ffblk) + sizeof(FX_LOCAL_PATH) + MAX_PATH_LENGTH*sizeof(ucs2_t) + MAX_PATH_LENGTH*sizeof(char));
	if (!p) return 0;

	pff=p;
	p+=sizeof(ffblk);
	pff->pfx_path=p;
	p+=sizeof(FX_LOCAL_PATH);
	pff->ff_longname=p;
	p+=MAX_PATH_LENGTH*sizeof(ucs2_t);
	pff->ff_name=p;

	pff->magic=FFBLK_MAGIC;

	return pff;
#else
	return 0;
#endif
}


/* only for current directory */
int find_first(ffblk *pff) {
#if 0
	UINT 	status;
	int 	nomatch;
	char 	fname[MAX_PATH_LENGTH];
	UCHAR 	uname[MAX_PATH_LENGTH * 2];
	ULONG 	ulen = MAX_PATH_LENGTH * 2;

	if (!pff) return YLERR_INVAL;
	if (!pff->rule_set||!*pff->rule_set) pff->rule_set="*";

	if (pff->search_path) {
		status=yl_dmParsePath(pff->search_path, 0, &pff->pfx_media);
	}
	else {
		status=yl_dmParsePath(".", 0, &pff->pfx_media);
	}
	if (status<0) return FNM_NOMATCH;
	status=fx_directory_local_path_set(pff->pfx_media, pff->pfx_path, pff->search_path+2);	// just skip drive number
	if (status) return FNM_NOMATCH;

	nomatch = FNM_NOMATCH;
	status = fx_directory_first_entry_find(pff->pfx_media, fname);
	while (!status) {

		nomatch = match_conditions(pff,fname,(CHAR*)uname,&ulen);	// (CHAR*) Jon modified.
		if (!nomatch) break;

		status = fx_directory_next_entry_find(pff->pfx_media, fname);
	}

	if (!nomatch) {
		check_unicode_name(pff,fname,(CHAR*)uname,ulen);	// (CHAR*) Jon modified.
	}

	save_find_state(pff);

	return nomatch;
#else
	return 0;
#endif
}


int find_next(ffblk *pff) {
#if 0
	UINT 	status;
	int 	nomatch;
	char 	fname[MAX_PATH_LENGTH];
	UCHAR 	uname[MAX_PATH_LENGTH * 2];
	ULONG 	ulen = MAX_PATH_LENGTH * 2;

	if (!pff || !pff->rule_set) return YLERR_INVAL;

	status=fx_directory_local_path_set(pff->pfx_media, pff->pfx_path, pff->search_path+2);

	restore_find_state(pff);

	nomatch = FNM_NOMATCH;
	status = fx_directory_next_entry_find(pff->pfx_media, fname);
	while (!status) {

		nomatch = match_conditions(pff,fname,(CHAR*)uname,&ulen);	// (CHAR*) Jon modified.
		if (!nomatch) break;

		status = fx_directory_next_entry_find(pff->pfx_media, fname);
	}

	if (!nomatch) {
		check_unicode_name(pff,fname,(CHAR*)uname,ulen);	// (CHAR*) Jon modified.
	}

	save_find_state(pff);

	return nomatch;
#else
	return 0;
#endif
}


