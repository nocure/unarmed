#include "platform/YL_FSIO.h"
#include <sct.h>
#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include <platform/YL_I18N.h>
#include <errno.h>
//#include <fx_api.h>


int translate_errno(UINT status) {
#if 0
	switch (status) {
		case FX_SUCCESS:
			return 0;

		case FX_MEDIA_INVALID:
		case FX_BOOT_ERROR:
			return YLERR_FS;

		case FX_FAT_READ_ERROR:
		case FX_FILE_CORRUPT:
		case FX_SECTOR_INVALID:
			return YLERR_FAT;

		case FX_IO_ERROR:
			return YLERR_IO;

		case FX_NOT_FOUND:
		case FX_INVALID_NAME:
		case FX_INVALID_PATH:
			return YLERR_NOENT;

		case FX_MEDIA_NOT_OPEN:
			return YLERR_DRIVE;

		case FX_NOT_A_FILE:
			return YLERR_ISDIR;

		case FX_NOT_OPEN:
			return YLERR_BADF;

		case FX_ACCESS_ERROR:
		case FX_WRITE_PROTECT:
			return YLERR_WP;

		case FX_NO_MORE_SPACE:
			return YLERR_NOSPC;

		case FX_ALREADY_CREATED:
			return YLERR_EXIST;

		case FX_NOT_DIRECTORY:
			return YLERR_NOTDIR;

		case FX_DIR_NOT_EMPTY:
			return YLERR_ENOTEMPTY;

		case FX_NO_MORE_ENTRIES:
			return YLERR_NMFILE;

		case FX_INVALID_YEAR:
		case FX_INVALID_MONTH:
		case FX_INVALID_DAY:
		case FX_INVALID_HOUR:
		case FX_INVALID_MINUTE:
		case FX_INVALID_SECOND:
		case FX_INVALID_ATTR:
		case FX_PTR_ERROR:
		case FX_INVALID_OPTION:
			return YLERR_INVAL;

		case FX_NOT_IMPLEMENTED:
			return YLERR_NOTSUPP;

		case FX_END_OF_FILE:
		case FX_CALLER_ERROR:
		case FX_BUFFER_ERROR:
		case FX_ERROR_NOT_FIXED:
		case FX_ERROR_FIXED:
		default:
			break;
	}
#endif
	return -1;
}


FILE *_fopen(const char *file, const char *mode) {
#if 0
	int 	flags = O_RDONLY;
	int		rc;
	UINT	status = FX_SUCCESS;
	FX_FILE *	fp = NULL;
	char *	abspath = NULL;
	FX_MEDIA *	mediap = NULL;

	rc = 0;
	while (*mode) {
		switch (*mode++) {
		case 'r':
			flags = O_RDONLY;
			break;
		case 'w':
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case 'a':
			flags = O_WRONLY | O_CREAT | O_APPEND;
			break;
		case '+':
			rc = 1;
			break;
		case 'b':
			break;

		default:
			errno=YLERR_INVAL;
			return 0;
		}
	}

	if (rc) {
		flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
	}

	fp=malloc(sizeof(FX_FILE));
	if (!fp) {
		rc = YLERR_NOMEM;
		goto done;
	}

	rc = yl_dmParsePath(file, &abspath, (void*)&mediap);
	if (rc<0) goto out0;

	if (flags & (O_RDWR | O_WRONLY)) {
		status = fx_file_open(mediap, fp, (CHAR *)abspath, FX_OPEN_FOR_WRITE);
		if (status != FX_SUCCESS) {		/* file not exist */
			status = fx_file_create(mediap, (CHAR *)abspath);
			if (status == FX_SUCCESS) {
				status = fx_file_open(mediap, fp, (CHAR *)abspath, FX_OPEN_FOR_WRITE);
			}
		}
		else {			/* file already exist */
			if (flags & O_TRUNC) {
				status = fx_file_truncate_release(fp, 0);
				if (status == FX_SUCCESS) {
					status = fx_file_seek(fp, 0);
				}
			}
		}
	}
	else {		/* read only */
		status = fx_file_open(mediap, fp, (CHAR *)abspath, FX_OPEN_FOR_READ_FAST);
	}

	if (status) goto done;
	if (flags & O_APPEND) {
		status = fx_file_seek(fp, fp->fx_file_current_file_size);
	}

done:
	rc=translate_errno(status);

out0:
	if (abspath)	free(abspath);
	errno = rc;
	if (rc<0){
		if (fp) 		free(fp);
		return NULL;
	}
	return (FILE*)fp;
#else
	return 0;
#endif

}


FILE *fopenW(const ucs2_t *file, const char *mode) {
#if 0
	int 	flags = O_RDONLY;
	int		rc;
	UINT	status = FX_SUCCESS;
	FX_FILE *	fp = NULL;
	char *	abspath = NULL;
	FX_MEDIA *	mediap = NULL;
	UINT 	len = wcslen(file) + 1;
	ucs2_t	spbuf[len+SP_EXTRA_SPACE];
	UCHAR 	fat_entry[len * 2];
	char 	short_name[FX_MAX_SHORT_NAME_LEN] = {0};
	FX_PATH dstp;
	UINT	unilen = 0;

	rc=0;
	while (*mode) {
		switch (*mode++) {
		case 'r':
			flags = O_RDONLY;
			break;
		case 'w':
			flags = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case 'a':
			flags = O_WRONLY | O_CREAT | O_APPEND;
			break;
		case '+':
			rc = 1;
			break;
		case 'b':
			break;

		default:
			errno=YLERR_INVAL;
			return 0;
		}
	}

	if (rc) {
		flags = (flags & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
	}

	fp=malloc(sizeof(FX_FILE));
	if (!fp) {
		errno=YLERR_NOMEM;
		return 0;
	}

	rc = yl_dmParsePathW(file, &abspath, (void*)&mediap);

	if ((rc != YLERR_OK) && (rc != YLERR_NOENT)) {
		goto out0;
	}

	if (flags & (O_RDWR | O_WRONLY)) {
		if (YLERR_NOENT == rc) {
			memset(fat_entry, 0, len * 2);
			splitpathW((ucs2_t *)file, spbuf);
			specialcombineW(spbuf);
			unilen = wcslen(WSP_NAME(spbuf));
			fx_directory_local_path_set(mediap, &dstp, abspath);
			MBStoLESTR(fat_entry, WSP_NAME(spbuf), 256);
			status = fx_unicode_file_create(mediap, fat_entry, unilen, short_name);
			if (status)
				goto done;
			fx_file_open(mediap, fp, (CHAR *)short_name, FX_OPEN_FOR_WRITE);
			rc = YLERR_OK;
			fx_directory_local_path_restore(lpiGetFSHandle(), lpiGetCWD());
		}
		else {
			status = fx_file_open(mediap, fp, (CHAR *)abspath, FX_OPEN_FOR_WRITE);
		}
	}
	else {		/* read only */
		status = fx_file_open(mediap, fp, (CHAR *)abspath, FX_OPEN_FOR_READ_FAST);
	}

	if (status) goto done;
	if (flags & O_APPEND) {
		status = fx_file_seek(fp, fp->fx_file_current_file_size);
	}

done:
	rc=translate_errno(status);

out0:
	if (abspath) free(abspath);
	errno = rc;
	if (rc<0){
		if (fp) free(fp);
		return NULL;
	}
	return (FILE*)fp;
#else
	return 0;
#endif
}


int _fclose(FILE * __f) {
#if 0
	int	rc;

	rc = fx_file_close((FX_FILE*)__f);
	if (FX_NOT_OPEN == rc) {
		rc = YLERR_BADF;
	}
	free(__f);
	return rc;
#else
	return 0;
#endif
}


int _fseek(FILE * __f, off_t __o, int __w) {
#if 0
int rc;
	if (__w != SEEK_SET && __w != SEEK_CUR && __w != SEEK_END)
		return YLERR_INVAL;

	if (SEEK_SET == __w)
		rc = fx_file_relative_seek((FX_FILE*)__f, __o, FX_SEEK_BEGIN);
	else if (SEEK_CUR == __w)
		rc = fx_file_relative_seek((FX_FILE*)__f, __o, FX_SEEK_FORWARD);
	else if (SEEK_END == __w)
		rc = fx_file_relative_seek((FX_FILE*)__f, __o, FX_SEEK_END);
	else{
		/* should never happen */
		rc=YLERR_INVAL;
	}

	if (rc) return translate_errno(rc);

	return 0;
#else
	return 0;
#endif
}


off_t _ftell(FILE * __f) {
#if 0
	return ((FX_FILE*)__f)->fx_file_current_file_offset;
#else
	return 0;
#endif
}


int _feof(FILE *__f) {
#if 0
	if (((FX_FILE*)__f)->fx_file_current_file_offset >= ((FX_FILE*)__f)->fx_file_current_file_size)
		return TRUE;
	else
		return FALSE;
#else
	return 0;
#endif
}


size_t _fread(void *buf, size_t size, FILE *__f) {
#if 0
	int	rc;
	ULONG	ret_size;

	if ((NULL == buf) || (size < 0) || (NULL == __f)) {
		return YLERR_INVAL;
	}

	rc = fx_file_read((FX_FILE*)__f, buf, (ULONG)size, &ret_size);

	if (rc) {
		return translate_errno(rc);
	}

	return ret_size;
#else
	return 0;
#endif
}


size_t _fwrite(const void *buf, size_t size, FILE *__f) {
#if 0
	int 	rc;

	if ((NULL == buf) || (size < 0) || (NULL == __f)) {
		return YLERR_INVAL;
	}

	rc = fx_file_write((FX_FILE*)__f, (VOID *)buf, size);

	if (rc) {
		return translate_errno(rc);
	}
	return size;
#else
	return 0;
#endif
}


int unlinkW(const ucs2_t * file_name)
{
#if 0
	int	rc = YLERR_OK;
	char *	abspath = NULL;
	FX_MEDIA *	mediap = NULL;

	rc = yl_dmParsePathW((ucs2_t *)file_name, &abspath, (void*)&mediap);

	if (rc) {
		goto done;
	}
	rc = fx_file_delete(mediap, (CHAR *)abspath);

done:
	if (abspath)	free(abspath);
	return translate_errno(rc);
#else
	return 0;
#endif
}


int _unlink(const char * file_name)
{
#if 0
	int	rc = YLERR_OK;
	char *	abspath = NULL;
	FX_MEDIA *	mediap = NULL;

	rc = yl_dmParsePath(file_name, &abspath, (void*)&mediap);

	if (rc<0)
		goto done;

	rc = fx_file_delete(mediap, (CHAR *)abspath);

done:
	if (abspath)	free(abspath);

	return translate_errno(rc);
#else
	return 0;
#endif
}


int _fputc(int ch, FILE *fp)
{
#if 0
	if (!fp) return YLERR_INVAL;

	return (fwrite(&ch, 1, fp)==1)? (int)ch : EOF;
#else
	return 0;
#endif
}


int _fputs(const char *str, FILE *fp)
{
#if 0
	if ((NULL == str) || (NULL == fp)) return YLERR_INVAL;

	return fwrite(str, strlen(str), fp);
#else
	return 0;
#endif
}


int _fgetc(FILE *fp)
{
#if 0
	unsigned char	ch;

	if (NULL == fp) return YLERR_INVAL;

	return (fread(&ch, 1, fp) == 1) ? (int)ch : EOF;
#else
	return 0;
#endif
}


char *_fgets(char *str, int size, FILE *fp)
{
#if 0
	int		ch;
	char	*p = str;

	if ((NULL == str) || (size < 0) || (NULL == fp)) {
		return NULL;
	}

	while (size > 1) {
		ch = getc(fp);

		if (EOF == ch) {
			*p = '\0';
			return NULL;
		}
		*p ++ = ch;
		size --;
		if ('\n' == ch)
			break;
	}
	if (size)
		*p = '\0';

	return str;
#else
	return 0;
#endif
}


#if 0
static int fx_time_to_tm(UINT fx_date, UINT fx_time, struct tm *out_tm)
{
	int	rc = YLERR_OK;

	out_tm->tm_sec = (fx_time & FX_SECOND_MASK) * 2;
	out_tm->tm_min = (fx_time >> FX_MINUTE_SHIFT) & FX_MINUTE_MASK;
	out_tm->tm_hour = (fx_time >> FX_HOUR_SHIFT) & FX_HOUR_MASK;
	out_tm->tm_mday = fx_date & FX_DAY_MASK;
	out_tm->tm_mon = (fx_date >> FX_MONTH_SHIFT) & FX_MONTH_MASK;
	out_tm->tm_year = ((fx_date >> FX_YEAR_SHIFT) & FX_YEAR_MASK) + (FX_BASE_YEAR);
//	out_tm->tm_wday = 0;
//	out_tm->tm_yday = 0;
//	out_tm->tm_isdst = 0;

	return rc;
}
#endif


/*
 * ww modified: these modification applied to wstat too.
 * remarked code free(entry.fx_dir_entry_name) below, entry.fx_dir_entry_name IS NOT an
 * allocated memory, try to free it will cause memory corruption.
 *
 * when the target path is root, set DT_DIR | FX_VOLUME to st_mode only, do not thing else
 */
int _stat(const char *path, struct stat *buf)
{
#if 0
	int		rc = YLERR_OK;
	UINT	attr = 0;
	FX_FILE	fp;
	FX_DIR_ENTRY	entry;
	char	*abspath = NULL;
	FX_MEDIA *media_ptr = NULL;
	CHAR	fx_search_buffer[FX_MAX_SHORT_NAME_LEN];

	rc = yl_dmParsePath(path, &abspath, (void*)&media_ptr);

	if (rc<0)
		goto done;

// ww 2008-1008 added -->
	memset(buf,0,sizeof(struct stat));
// ww added <--

	/* chris 20081103 added -->
	 * special treatment for root directory
	 */
	// a: or b: root directory
	if (NULL == abspath || (('/' == abspath[0] || '\\' == abspath[0]) && 0 == abspath[1])) {
#if 0
		ULONG i;
		buf->st_mode = DT_DIR | FX_VOLUME;
		FX_PROTECT
		entry.fx_dir_entry_name = fx_search_buffer;
		entry.fx_dir_entry_short_name[0]= 0;

		i = 0;
	    do
	    {
	    	fx_directory_entry_read(media_ptr, FX_NULL, &i, &entry);

	        /* Check for a volume name.  */
	        if (entry.fx_dir_entry_attributes & FX_VOLUME)
	        	break;

	            /* Move to next directory entry.  */
	        i++;

	    } while (i < media_ptr -> fx_media_root_directory_entries);
	    FX_UNPROTECT
		buf->st_size = entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);

//		free(entry.fx_dir_entry_name);
#endif
		buf->st_mode = DT_DIR | FX_VOLUME;
	}
	/* chris added end <-- */
	else if (FX_SUCCESS == fx_directory_attributes_read(media_ptr, abspath, &attr)) {
		/* it's a directory */
		buf->st_mode = (attr | DT_DIR) & (UINT)(~FX_DIRECTORY);
		/* chris 20081014 added -->
		 * Use fx_directory_search instead of opendir to avoid
		 * opendir in another opendir, which cause local path confused.
		 */
		FX_PROTECT
		entry.fx_dir_entry_name = fx_search_buffer;
		entry.fx_dir_entry_short_name[0]= 0;
		fx_directory_search(media_ptr, abspath, &entry, NULL, NULL);

		FX_UNPROTECT
		/* chris added end <-- */

		buf->st_size = entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);

//		free(entry.fx_dir_entry_name);
	}
	else if (FX_SUCCESS == fx_file_attributes_read(media_ptr, abspath, &attr)) {
		/* it's a directory */
		buf->st_mode = (attr | DT_REG);
		fx_file_open(media_ptr, &fp, abspath, FX_OPEN_FOR_READ_FAST);

		buf->st_size = fp.fx_file_dir_entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);

		fx_file_close(&fp);
	}
	else {
		rc = YLERR_NOENT;
	}

done:
	if (abspath)	free(abspath);

	return rc<0?rc:0;
#else
	return 0;
#endif
}


int statW(const ucs2_t *path, struct stat *buf)
{
#if 0
	int		rc = YLERR_OK;
	UINT	attr = 0;
	FX_FILE	fp;
	FX_DIR_ENTRY	entry;
	char	*abspath = NULL;
	FX_MEDIA *media_ptr = NULL;
	CHAR	fx_search_buffer[FX_MAX_SHORT_NAME_LEN];

	rc = yl_dmParsePathW((ucs2_t *)path, &abspath, (void*)&media_ptr);

	if (rc)
		goto done;

// ww 2008-1008 added -->
	memset(buf,0,sizeof(struct stat));
// ww added <--

	/* chris 20081103 added -->
	 * special treatment for root directory
	 */
	// a: or b: root directory
	if (NULL == abspath || (('/' == abspath[0] || '\\' == abspath[0]) && 0 == abspath[1])) {
#if 0
		ULONG i;
		buf->st_mode = DT_DIR | FX_VOLUME;
		FX_PROTECT
		entry.fx_dir_entry_name = fx_search_buffer;
		entry.fx_dir_entry_short_name[0]= 0;

		i = 0;
	    do
	    {
	    	fx_directory_entry_read(media_ptr, FX_NULL, &i, &entry);

	        /* Check for a volume name.  */
	        if (entry.fx_dir_entry_attributes & FX_VOLUME)
	        	break;

	        /* Move to next directory entry.  */
	        i++;

	    } while (i < media_ptr -> fx_media_root_directory_entries);
	    FX_UNPROTECT

		buf->st_size = entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);
//		free(entry.fx_dir_entry_name);
#endif
		buf->st_mode = DT_DIR | FX_VOLUME;
	}
	/* chris added end <-- */
	else if (FX_SUCCESS == fx_directory_attributes_read(media_ptr, abspath, &attr)) {
		/* it's a directory */
		buf->st_mode = (attr | DT_DIR) & (UINT)(~FX_DIRECTORY);
		/* chris 20081014 added -->
		 * Use fx_directory_search instead of opendir to avoid
		 * opendir in another opendir, which cause local path confused.
		 */
		FX_PROTECT
		entry.fx_dir_entry_name = fx_search_buffer;
		entry.fx_dir_entry_short_name[0]= 0;
		fx_directory_search(media_ptr, abspath, &entry, NULL, NULL);

		FX_UNPROTECT
		/* chris added end <-- */

		buf->st_size = entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);

//		free(entry.fx_dir_entry_name);
	}
	else if (FX_SUCCESS == fx_file_attributes_read(media_ptr, abspath, &attr)) {
		/* it's a directory */
		buf->st_mode = (attr | DT_REG);
		fx_file_open(media_ptr, &fp, abspath, FX_OPEN_FOR_READ_FAST);

		buf->st_size = fp.fx_file_dir_entry.fx_dir_entry_file_size;

		/* no last access time kept in DIR_ENTRY */
		fx_time_to_tm(entry.fx_dir_entry_last_accessed_date, 0, &buf->st_atime);
		fx_time_to_tm(entry.fx_dir_entry_created_date, entry.fx_dir_entry_created_time, &buf->st_ctime);
		fx_time_to_tm(entry.fx_dir_entry_date, entry.fx_dir_entry_time, &buf->st_mtime);

		fx_file_close(&fp);
	}
	else {
		rc = YLERR_NOENT;
	}

done:
	if (abspath)	free(abspath);

	return rc;
#else
	return 0;
#endif

}


int _fflush(void)
{
#if 0
#define MAX_DRIVE 4
	int		rc = YLERR_OK;
	int 	i;
	FX_MEDIA	*mediap = NULL;

	for (i = 0; i < MAX_DRIVE; i ++) {
		mediap = yl_dmGetFSHandle(i);

		if (mediap) {
			if ( fx_media_flush(mediap) )
				rc = YLERR_IO;
		}
	}

	return rc;
#else
	return 0;
#endif
}


/* no need to support unicode, because of no input device */
int _rename(const char *oldpath, const char *newpath)
{
#if 0
	int rc = YLERR_OK;
	struct stat s;
	char *abspath_old = NULL, *abspath_new = NULL;
	FX_MEDIA *mediap = NULL;

	memset(&s, 0 , sizeof(struct stat));

	if (YLERR_OK == stat(oldpath, &s)) {
		yl_dmParsePath(oldpath, &abspath_old, (void*)&mediap);
		yl_dmParsePath(newpath, &abspath_new, (void*)&mediap);
		if (S_ISDIR(s.st_mode)) {
			rc = fx_directory_rename(mediap, abspath_old, abspath_new);
			if (rc) {
				rc = (FX_ALREADY_CREATED == rc) ? YLERR_EXIST : YLERR_INVLPATH;
			}
		}
		else if (S_ISREG(s.st_mode)) {
			rc = fx_file_rename(mediap, abspath_old, abspath_new);
			if (rc) {
				rc = (FX_ALREADY_CREATED == rc) ? YLERR_EXIST : YLERR_INVLPATH;
			}
		}
		else
			rc = YLERR_INVAL;		// not file and not directory
	}
	else
		rc = YLERR_NOENT;			// oldpath not even exist

	if (abspath_old)	free(abspath_old);
	if (abspath_new)	free(abspath_new);
	return rc;
#else
	return 0;
#endif
}

FILE* _tmpfile(void) {
	return 0;
}

int _ferror(FILE* stream) {
	return 0;
}

