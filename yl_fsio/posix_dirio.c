#include "platform/YL_FSIO.h"
#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include <platform/YL_I18N.h>
#include <errno.h>
//#include <fx_api.h>

typedef struct _DIR_S {
	unsigned int media_num;
//	FX_LOCAL_PATH path;
	void *path;
	struct dirent dent;
} DIR_S;

int translate_errno(UINT status);


// returns 1 if we have a match.
int wildcmp(const char *wild, const char *string) {
const char *cp= NULL, *mp= NULL;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;
		} else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	return !*wild;
}

// ================================================================


DIR *_opendir(const char *dir_name) {
#if 0
	UINT	status;
	int		err = YLERR_OK;
	DIR		*dirp = NULL;
	char	*abspath = NULL;
	FX_MEDIA		*media_ptr = NULL;
	int idx;

	dirp = (DIR *)malloc(sizeof(DIR_S));
	if (!dirp) {
		err = YLERR_NOMEM;
		goto done;
	}

	idx = yl_dmParsePath(dir_name, &abspath, (void*)&media_ptr);

	if (idx<0) {
		err =idx;
		goto done;
	}

	status = fx_directory_local_path_set(media_ptr, &((DIR_S*)dirp)->path, (CHAR *)abspath);
	if (status != FX_SUCCESS) {
		err = YLERR_INVLPATH;
		goto done;
	}

	((DIR_S*)dirp)->path.fx_path_current_entry = 0;
	((DIR_S*)dirp)->media_num = idx;

done:
	if (abspath) free(abspath);
	errno = err;
	if (err) {
		if (dirp) free(dirp);
		return NULL;
	}
	return (DIR *)dirp;
#else
	return 0;
#endif
}


struct dirent *_readdir(DIR *dir) {
#if 0
	int				rc = YLERR_OK;
	int				i = 0;
    char*			dir_name;		// support unicode
    struct dirent *	tmpdent = NULL;
    UINT			attr = 0;
    FX_MEDIA *		mediap = NULL;

    if (!dir) {
       	errno=YLERR_INVAL;
       	goto err0;
    }

    mediap = yl_dmGetFSHandle(((DIR_S*)dir)->media_num);
    fx_directory_local_path_restore(mediap, &((DIR_S*)dir)->path);
    dir_name=malloc(FX_MAX_LONG_NAME_LEN);
    if (!dir_name) {
    	errno=YLERR_NOMEM;
       	goto err0;
    }

    rc = fx_directory_next_entry_find(mediap, dir_name);
    if (rc != FX_SUCCESS) {
    	rc = 0;
    	goto err1;
    }

	tmpdent = &((DIR_S*)dir)->dent;
	memset(tmpdent, 0, sizeof(struct dirent));

	for(i = 0; dir_name[i]; i ++) {
		if (i>=sizeof(tmpdent->d_name)-1) break; // ww check for length
		tmpdent->d_name[i]= dir_name[i];
	}
	tmpdent->d_name[i] = (CHAR)0;
	tmpdent->d_off = ((DIR_S*)dir)->path.fx_path_current_entry;
	tmpdent->d_reclen = FX_MAX_LONG_NAME_LEN;

	rc = fx_directory_attributes_read(mediap, dir_name, &attr);
	if (FX_SUCCESS == rc) {
		tmpdent->d_type = DT_DIR;
	}
	else {
		rc = fx_file_attributes_read(mediap, dir_name, &attr);
		if (FX_SUCCESS == rc) {
			tmpdent->d_type = DT_REG;
		}
		else {
			/*TODO: add file type volume, hidden, archive...etc */
			rc = YLERR_OK;
// 			dbgout("readdir: unknown entry\n");
		}
	}

err1:
	free(dir_name);

err0:

	errno=rc;
	return tmpdent;
#else
	return 0;
#endif
}


int _closedir(DIR *dp)
{
#if 0
	/* chris 2008-1010 added -->
	 * free() would not clear the dp->path(refer by thread
	 * local path) to zero, so thread_filex_ptr still
	 * point to address with legal information.
	 * Fix it by clean dp->path to all zero.
	 */
	fx_directory_local_path_clear(yl_dmGetFSHandle(((DIR_S*)dp)->media_num));
	free(dp);

	return YLERR_OK;
#else
	return 0;
#endif
}


int _chdir(const char *path_name) {
#if 0
	int		rc = YLERR_OK;
	char	*cwd_str = NULL;
	FX_MEDIA	*mediap = NULL;

	rc = yl_dmParsePath(path_name, &cwd_str, (void*)&mediap);

	if (rc<0)
		goto done;

	rc = fx_directory_local_path_set(mediap, lpiGetCWD(), cwd_str);
	if (rc) {
		rc = (FX_MEDIA_NOT_OPEN == rc) ? YLERR_DRIVE : YLERR_NOENT;
		goto done;
	}
	lpiSetFSHandle(mediap);

done:
	if (cwd_str)
		free(cwd_str);
	return rc<0?rc:0;
#else
	return 0;
#endif
}


int chdirW(const ucs2_t *path_name) {
#if 0
	int		rc = YLERR_OK;
	char	*cwd_str = NULL;
	FX_MEDIA	*mediap = NULL;

	rc = yl_dmParsePathW(path_name, &cwd_str, (void*)&mediap);

	if (rc<0)
		goto done;

	rc = fx_directory_local_path_set(mediap, lpiGetCWD(), cwd_str);
	if (rc) {
		rc = (FX_MEDIA_NOT_OPEN == rc) ? YLERR_DRIVE : YLERR_NOENT;
		goto done;
	}
	lpiSetFSHandle(mediap);

done:
	if (cwd_str)
		free(cwd_str);
	return rc<0?rc:0;
#else
	return 0;
#endif
}


int mkdirW(const ucs2_t *path_name) {
#if 0
	int		err = YLERR_OK;
	char	*abspath = NULL;
	FX_MEDIA *mediap = NULL;
	UINT 	len = wcslen(path_name) + 1;
	ucs2_t	spbuf[len+SP_EXTRA_SPACE];
	UCHAR 	fat_entry[len * 2];
	char 	short_name[FX_MAX_SHORT_NAME_LEN] = {0};
	FX_PATH dstp;
	UINT	unilen = 0;

	err = yl_dmParsePathW(path_name, &abspath, (void*)&mediap);
	if (err>=0) {
		err = YLERR_EXIST;
		goto done;
	}
	else if (YLERR_NOENT == err) {		/* Destination directory not exist */
		memset(fat_entry, 0, len * 2);
		splitpathW((ucs2_t *)path_name, spbuf);
		specialcombineW(spbuf);
		unilen = wcslen(WSP_NAME(spbuf));
		fx_directory_local_path_set(mediap, &dstp, abspath);
		MBStoLESTR(fat_entry, WSP_NAME(spbuf), 256);
		err = fx_unicode_directory_create(mediap, fat_entry, unilen, short_name);
		fx_directory_local_path_restore(lpiGetFSHandle(), lpiGetCWD());
	}

	err=translate_errno(err);

done:
	if (abspath)
		free(abspath);
	return err;
#else
	return 0;
#endif
}


int _mkdir(const char *path_name) {
#if 0
	int		err = YLERR_OK;
	char	*abspath = NULL;
	FX_MEDIA *mediap = NULL;

	err = yl_dmParsePath(path_name, &abspath, (void*)&mediap);

	if (err<0) goto done;

	err = fx_directory_create(mediap, (CHAR *)abspath);
	err=translate_errno(err);

done:
	if (abspath)
		free(abspath);
	return err;
#else
	return 0;
#endif
}


int rmdirW(const ucs2_t *path_name)
{
#if 0
	int		err = YLERR_OK;
	char	*abspath = NULL;
	FX_MEDIA *mediap = NULL;

	err = yl_dmParsePathW((ucs2_t *)path_name, &abspath, (void*)&mediap);

	if (err<0)
		goto done;

	err = fx_directory_delete(mediap, (CHAR *)abspath);
	err=translate_errno(err);

done:
	if (abspath)
		free(abspath);
	return err;
#else
	return 0;
#endif
}


int _rmdir(const char *path_name) {
#if 0
	int		err = YLERR_OK;
	char	*abspath = NULL;
	FX_MEDIA *mediap = NULL;

	err = yl_dmParsePath(path_name, &abspath, (void*)&mediap);

	if (err<0)
		goto done;

	err = fx_directory_delete(mediap, (CHAR *)abspath);
	err=translate_errno(err);

done:
	if (abspath)
		free(abspath);
	return err;
#else
	return 0;
#endif
}


void _rewinddir(DIR *dp) {
#if 0

	((DIR_S*)dp)->path.fx_path_current_entry = 0;

	return;
#else
	return;
#endif
}


char *_getcwd(char *cwd, unsigned long size) {
#if 0
	int		rc = YLERR_OK;
	char *	cur_path = NULL, path[FX_MAX_LONG_NAME_LEN];
	FX_MEDIA *mp = NULL;
	int cwd_alloc=0;

	if (NULL == cwd) {
		size = FX_MAX_LONG_NAME_LEN;
		cwd = (char *)malloc(size);
		if(!cwd) {
			errno = YLERR_NOMEM;
			return NULL;
		}
		cwd_alloc=1;
	}

	memset(cwd, 0, size);
	memset(path, 0, FX_MAX_LONG_NAME_LEN);

	mp = lpiGetFSHandle();
	if (!mp) {
		// set default media to a:
		mp=yl_dmGetFSHandle(0);
		if (!mp) {
			rc = YLERR_FS;
			goto done;
		}
		lpiSetFSHandle(mp);
	}

	cur_path = ((FX_PATH*)lpiGetCWD())->fx_path_string;
	if ((NULL == cur_path) || (0 == cur_path[0])) {	// root directory
		path[0] = '/';
		path[1] = '\0';
	}
	else {
		if (size < strlen(cur_path) + 2) {		/* append a:\ or b:\ */
			rc = YLERR_NAMETOOLONG;
			goto done;
		}
		memcpy(path, cur_path, strlen(cur_path) + 1);
	}
	sprintf(cwd, "%c:%s", yl_dmGetDrvIndex(mp) + 'A', path);

done:

	errno=rc;
	if (rc) {
		if (cwd_alloc) {
			free(cwd);
		}
		return NULL;
	}
	else
		return cwd;
#else
	return 0;
#endif
}


// ww optimized a bit
ucs2_t* getcwdW(ucs2_t* wcwd_in, unsigned int size)
{
#if 0
	ucs2_t wbuffer[MAX_PATH_LENGTH];
	ucs2_t trans[MAX_PATH_LENGTH];
	char *cwd,*cwd1,*dstart,*dir2goin;
	char* old_cwd;
	int depth;
	int ret,wcwdlen;
	char buffer_shortname[13];
	ULONG unicodelength = 0;
	ucs2_t *new_wcwd,*wcwd;

	size--;	// reserve 1 space for null

	new_wcwd=0;

	if (!wcwd_in) {
		size = FX_MAX_LONG_NAME_LEN*2;
		wcwd = (ucs2_t*)malloc(size);
		if(!wcwd) goto out0;
	}
	else wcwd=wcwd_in;

	cwd1=getcwd(0,MAX_PATH_LENGTH);
	if (!cwd1) goto out1;
	cwd=cwd1;

	old_cwd=strdup(cwd);
	if (!old_cwd) goto out2;

	// drive number
	wcwd[0]=cwd[0];	// 'A'
	wcwd[1]=cwd[1];	// ':'
	wcwd[2]=0;

	wcwdlen=2;
	depth=0;
	dir2goin=0;
	while (1) {
		dstart=cwd;
		while(1) {
		int c;
			if (!(c=*cwd)) break;
			if (isdelim(c)) {
				*cwd++=0;
				break;
			}
			cwd++;
		}

		if (cwd<=dstart) break;
		if (depth) { // going in on second round
			chdir(dir2goin);
			ret=fx_directory_short_name_get(lpiGetFSHandle(), dstart, buffer_shortname);
			if (ret) goto out3;

			fx_unicode_name_get(lpiGetFSHandle(), buffer_shortname, (UCHAR*)trans, &unicodelength);
			if(!unicodelength){
				unicodelength = strlen(buffer_shortname);
				ASCIItoMBS(wbuffer, buffer_shortname, unicodelength);
			}
			else{
				MBStoLESTR((UCHAR *)wbuffer, trans, MAX_PATH_LENGTH);
			}
			wcwdlen+=wcslen(wbuffer);
			if (wcwdlen>=size) goto out3;
			pathcatW(wcwd,wbuffer);
		}
		dir2goin=dstart;

		depth++;
	}
	new_wcwd=wcwd;


out3:
	chdir(old_cwd);
	free(old_cwd);

out2:
	free(cwd1);

out1:
	if (!new_wcwd) {
		if (!wcwd_in) {
			free(wcwd);
		}
	}
	else {
		if (!wcwd_in) {
			new_wcwd=wcsdup(wcwd);
			free(wcwd);
		}
	}

out0:

	return new_wcwd;
#else
	return 0;
#endif
}


#if 0
static time_t
calendar_to_timet(UINT yr, UINT mnth, UINT day, UINT hr, UINT min, UINT secs)
{
	struct tm	t;

	t.tm_sec = secs;
	t.tm_min = min;
	t.tm_hour = hr;
	t.tm_mday = day;
	t.tm_mon = mnth;
	t.tm_year = yr - YEAR_BASE;
	t.tm_wday = 0;
	t.tm_yday = 0;
	t.tm_isdst = 0;

	return mktime(&t);
}


/* only for current directory */
int find_first(fdata_t *match_data, const fspec_t *ptn) {
	UINT 	status= FX_SUCCESS;
	FX_MEDIA *cur_media= NULL;
	char *	cur_path= NULL;
	char 	fname[MAX_NAME_LEN];
	UCHAR 	uname[MAX_NAME_LEN * 2];
	int 	i, j, matched = FNM_NOMATCH;
	UINT 	attr = 0, year = 0, month = 0, day = 0;
	UINT 	hour = 0, min = 0, secs = 0;
	ULONG 	size = 0, ulen = 0;
	time_t 	modtime = 0;
	char *	rule_indv= NULL, *saveptr= NULL, *pptn= NULL;
	char 	rule_set[strlen(ptn->dname_ptn) + 1];
	UINT	uname_rc = FX_SUCCESS;

	if (NULL == match_data || NULL == ptn)
		return YLERR_INVAL;
	/* validate find_t */
	if ((ptn->dname_ptn[0] == '\0') && (TS_NONE == ptn->flag) && (TP_NONE
			== ptn->dtype))
		return YLERR_INVAL;

	memset(match_data, 0, sizeof(match_data));
	yl_dmParsePath(".", &cur_path, &cur_media);

	status = fx_directory_first_entry_find(cur_media, fname);
	if (status) {
		/* FX_NO_MORE_ENTRIES */
		goto done;
	}
	uname_rc = fx_unicode_name_get(cur_media, fname, uname, &ulen);

	do {
		matched = FALSE;
		j = 0;
		if (ptn->dname_ptn[0] != '\0') {
			for (i = 0; i < strlen(ptn->dname_ptn); i ++) {
				if (isspace(ptn->dname_ptn[i])) {
					i ++;
				}
				rule_set[j ++] = ptn->dname_ptn[i];
			}
			rule_set[j] = '\0';


			char *	cmp_name = NULL;
			UCHAR 	utf8str[ulen * 6];

			matched = FNM_NOMATCH;
			for (i = 0, pptn = rule_set;; i++, pptn = NULL) {
				rule_indv = strtok_r(pptn, ";", &saveptr);
				if (NULL == rule_indv)
					break;

				/* unicode convert to UTF-8 for wildcmp */
				if (FX_SUCCESS == uname_rc) {
					FXUnicode2UTF8(utf8str, uname, ulen * 6);
					cmp_name = utf8str;
				}
				else {
					cmp_name = fname;
				}

				if (rule_indv[0] == '*' && rule_indv[1] == '.' && rule_indv[2] == '*' && strlen(rule_indv) == 3)
					matched &= wildcmp("*",cmp_name)?FNM_MATCH:FNM_NOMATCH;
				else
					matched &= wildcmp(rule_indv,cmp_name)?FNM_MATCH:FNM_NOMATCH;
			}
		}

		status
				=fx_directory_information_get(cur_media, fname, &attr, &size, &year, &month, &day, &hour, &min, &secs);

		if (ptn->flag & TIME_CMP) {
			modtime = calendar_to_timet(year, month, day, hour, min, secs);
			if (ptn->flag & TIME_AFT)
				matched |= (modtime >= ptn->time) ? FNM_MATCH : FNM_NOMATCH;
			else
				matched |= (modtime < ptn->time) ? FNM_MATCH : FNM_NOMATCH;
		}

		if (ptn->size > 0) {
			if (ptn->flag & SIZE_GRT)
				matched |= (size >= ptn->size) ? FNM_MATCH : FNM_NOMATCH;
			else
				matched |= (size < ptn->size) ? FNM_MATCH : FNM_NOMATCH;
		}

		if (ptn->dtype != TP_NONE) {
			if (attr & (FX_DIRECTORY | FX_VOLUME)) {
				if (attr & FX_DIRECTORY)
					matched |= !S_ISDIR(ptn->dtype);
			} else { /* file */
				matched |= !S_ISREG(ptn->dtype);
			}
		}

		if (FNM_NOMATCH == matched) {
			status = fx_directory_next_entry_find(cur_media, fname);
			uname_rc = fx_unicode_name_get(cur_media, fname, uname, &ulen);
		}

	} while (FNM_NOMATCH == matched && FX_SUCCESS == status);

done:
	if (FNM_MATCH == matched) {
		strcpy(match_data->entry_name,fname);	// ww modified
//		for (i = 0; i < strlen(fname); i ++)
//			match_data->entry_name[i] = fname[i];
//		match_data->entry_name[i] = '\0';

		if (ulen > 0) {
			match_data->shortened = FX_TRUE;
			LESTRtoMBS(match_data->unicode_name, uname, MAX_NAME_LEN);
		}
		else {
			match_data->shortened = FX_FALSE;
			ASCIItoMBS(match_data->unicode_name, fname, MAX_NAME_LEN);
		}

		match_data->entry_offset = __THREAD_CWD.fx_path_current_entry;
		if (__THREAD_CWD.fx_path_string[0]) {
			match_data->cwd_num_entries = __THREAD_CWD.fx_path_directory.fx_dir_entry_file_size;
		}
		else {	/* root directory */
			match_data->cwd_num_entries = cur_media->fx_media_root_directory_entries;
		}
	}
	if (cur_path)
		free(cur_path);
	return matched;
}


/* only for current directory */
int find_next(fdata_t *match_data, const fspec_t *ptn) {
	UINT 	status= FX_SUCCESS;
	FX_MEDIA *cur_media= NULL;
	char *	cur_path= NULL;
	char 	fname[MAX_NAME_LEN];
	UCHAR 	uname[MAX_NAME_LEN * 2];
	int 	i, j, matched = FNM_NOMATCH;
	UINT 	attr = 0, year = 0, month = 0, day = 0;
	UINT 	hour = 0, min = 0, secs = 0;
	ULONG 	size = 0, ulen = 0;
	time_t 	modtime = 0;
	char *	rule_indv= NULL, *saveptr= NULL, *pptn= NULL;
	char 	rule_set[strlen(ptn->dname_ptn) + 1];
	UINT	uname_rc = FX_SUCCESS;

	if (NULL == match_data || NULL == ptn)
		return YLERR_INVAL;
	/* validate find_t */
	if ((ptn->dname_ptn[0] == '\0') && (TS_NONE == ptn->flag) && (TP_NONE == ptn->dtype))
		return YLERR_INVAL;

	if (match_data->shortened > 1 || match_data->entry_offset > match_data->cwd_num_entries)
			return YLERR_INVAL;

	yl_dmParsePath(".", &cur_path, &cur_media);
	__THREAD_CWD.fx_path_current_entry = match_data->entry_offset;
	__THREAD_CWD.fx_path_directory.fx_dir_entry_file_size = match_data->cwd_num_entries;

	while ((FNM_NOMATCH == matched) && (FX_SUCCESS == status)) {
		status = fx_directory_next_entry_find(cur_media, fname);

		if (status)
			goto done;

		uname_rc = fx_unicode_name_get(cur_media, fname, uname, &ulen);

		matched = FALSE;
		if (ptn->dname_ptn[0] != '\0') {
			j = 0;
			for (i = 0; i < strlen(ptn->dname_ptn); i ++) {
				if (isspace(ptn->dname_ptn[i])) {
					i ++;
				}
				rule_set[j ++] = ptn->dname_ptn[i];
			}
			rule_set[j] = '\0';


			char *	cmp_name = NULL;
			UCHAR 	utf8str[ulen * 6];

			matched = FNM_NOMATCH;
			for (i = 0, pptn = rule_set;; i++, pptn = NULL) {
				rule_indv = strtok_r(pptn, ";", &saveptr);
				if (NULL == rule_indv)
					break;

				/* unicode convert to UTF-8 for wildcmp */
				if (FX_SUCCESS == uname_rc) {
					FXUnicode2UTF8(utf8str, uname, ulen * 6);
					cmp_name = utf8str;
				}
				else {
					cmp_name = fname;
				}

				if (rule_indv[0] == '*' && rule_indv[1] == '.' && rule_indv[2] == '*' && strlen(rule_indv) == 3)
					matched &= wildcmp("*",cmp_name)?FNM_MATCH:FNM_NOMATCH;
				else
					matched &= wildcmp(rule_indv,cmp_name)?FNM_MATCH:FNM_NOMATCH;
			}
		}

		status
				= fx_directory_information_get(cur_media, fname, &attr, &size, &year, &month, &day, &hour, &min, &secs);

		if (ptn->flag & TIME_CMP) {
			modtime = calendar_to_timet(year, month, day, hour, min, secs);
			if (ptn->flag & TIME_AFT)
				matched |= (modtime >= ptn->time) ? FNM_MATCH : FNM_NOMATCH;
			else
				matched |= (modtime < ptn->time) ? FNM_MATCH : FNM_NOMATCH;
		}

		if (ptn->size > 0) {
			if (ptn->flag & SIZE_GRT)
				matched |= (size >= ptn->size) ? FNM_MATCH : FNM_NOMATCH;
			else
				matched |= (size < ptn->size) ? FNM_MATCH : FNM_NOMATCH;
		}

		if (ptn->dtype != TP_NONE) {
			matched |= !((attr & DT_HIDDEN) & ptn->dtype);
			matched |= !((attr & DT_RDONLY) & ptn->dtype);
			if (attr & (FX_DIRECTORY | FX_VOLUME)) {
				if (attr & FX_DIRECTORY)
					matched |= !S_ISDIR(ptn->dtype);
			} else { /* file */
				matched |= !S_ISREG(ptn->dtype);
			}
		}
	}
done:
	if (FNM_MATCH == matched) {
		strcpy(match_data->entry_name,fname);	// ww modified
//		for (i = 0; i < strlen(fname); i ++)
//			match_data->entry_name[i] = fname[i];
//		match_data->entry_name[i] = '\0';

		if (ulen > 0) {
			match_data->shortened = FX_TRUE;
			LESTRtoMBS(match_data->unicode_name, uname, MAX_NAME_LEN);
		}
		else {
			match_data->shortened = FX_FALSE;
			ASCIItoMBS(match_data->unicode_name, fname, MAX_NAME_LEN);
		}

		match_data->entry_offset = __THREAD_CWD.fx_path_current_entry;
		if (__THREAD_CWD.fx_path_string[0]) {
			match_data->cwd_num_entries = __THREAD_CWD.fx_path_directory.fx_dir_entry_file_size;
		}
		else {	/* root directory */
			match_data->cwd_num_entries = cur_media->fx_media_root_directory_entries;
		}
	}
	if (cur_path)
		free(cur_path);
	return matched;
}


ucs2_t* _wgetcwd(ucs2_t* absPath,unsigned int size)
{
	ucs2_t wbuffer[MAX_PATH_LENGTH]= {0};
	ucs2_t trans[MAX_PATH_LENGTH]= {0};
	ucs2_t unicodebuffer[MAX_PATH_LENGTH]= {0};
	char sbuf[MAX_PATH_LENGTH]= {0};

	getcwd(sbuf,MAX_PATH_LENGTH);

	char* buffer2 = NULL;
	FX_MEDIA* pmedia =NULL;
	yl_dmParsePath(".", &buffer2, &pmedia);

	if(strlen(sbuf) < 3)
		return NULL;
	if(sbuf[1]== ':'){
		absPath[0] = (ucs2_t)sbuf[0];
		absPath[1] = (ucs2_t)sbuf[1];
		absPath[2] = '/';
		absPath[3] = 0;
	}


	ULONG uWNum = 0;
	while(strlen(sbuf)>3)
	{
		char drv[5]={0};
		char dir[MAX_PATH_LENGTH]={0};
		char pathname[MAX_PATH_LENGTH]={0};
		char ext[4]={0};
		char buffer_shortname[13]={0};
		ULONG unicodelength = 0;

		yl_splitpath(sbuf, drv, dir,pathname, ext);
		memset(sbuf,0,128);
		strcpy( sbuf, drv );
		if(dir[0]!=NULL)
			yl_concatpath(sbuf, dir, 128);

		chdir(sbuf);
		fx_directory_short_name_get(pmedia, pathname, buffer_shortname);
		fx_unicode_name_get(pmedia, buffer_shortname, (UCHAR*)trans, &unicodelength);
		if(unicodelength==0){
			unicodelength = strlen(buffer_shortname);
			ASCIItoMBS(wbuffer, buffer_shortname, unicodelength);
		}
		else{
			MBStoLESTR((UCHAR *)wbuffer, trans, MAX_PATH_LENGTH);
		}

		uWNum = uWNum + unicodelength + 1;
		if(uWNum>MAX_PATH_LENGTH)
			break;

		yl_unicodeConcatPath(wbuffer,unicodebuffer);

		memset((char*)unicodebuffer,0,MAX_PATH_LENGTH*2);
		yl_unicodeConcatPath(unicodebuffer,wbuffer);
	}

	WORD index = 0;
	WORD end = 0;
	if(uWNum==0)
		end = 0;
	else if((uWNum+3)>size)
		end = size - 3;
	else
		end = uWNum-1;

	for(index=0;index<end;index++){
		absPath[3+index]= unicodebuffer[index];
	}
	absPath[3+end] = 0;


	if(buffer2){
		chdir(buffer2);
		free(buffer2);
	}
	return absPath;

}
#endif
