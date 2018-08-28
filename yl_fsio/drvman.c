/******************************************************************************
 * drvman.c -- Drive Manager
 * ============================================================================
 *
 *
 * 2008-0922 ww created
 *
 *
 *(\~~/)(\~~/)
 *('.' )(^.^ )
 * (_(__ (_(__)~*
 *****************************************************************************/
#include "platform/YL_FSIO.h"
#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include <platform/YL_I18N.h>
//#include <fx_api.h>

#define MAX_DRIVE 4

#define DM_STAT_VALID		1
#define DM_STAT_MOUNTED		2

typedef struct {
	int status;
//	FX_MEDIA* mp;
	void* mp;
	DRIVERFUNC driver;
	void* ext;
	char* buf;
	int bufsize;
	IOCTL ioctl;
} DRIVEMAN_COLLECTION_T;


static DRIVEMAN_COLLECTION_T g_DriveInfo[MAX_DRIVE];

#if 0
static ucs2_t *_dir_wname_extract(ucs2_t *source_ptr, ucs2_t *dest_ptr) {
	UINT i;

	dest_ptr[0] = 0x0000;

	/* Is a backslash present?  */
	if (isdelim(*source_ptr))
	{
		source_ptr++;
	}

	/* Loop to remove any leading spaces.  */
	while (*source_ptr == ' ')
	{
		/* Position past leading space.  */
		source_ptr++;
	}

	/* Loop to extract the name.  */
	i = 0;
	while (*source_ptr)
	{
		/* If another backslash is present, break the loop.  */
		if (isdelim(*source_ptr))
				break;

		/* Long name can be at most 255 characters, but are further limited by the
		 FX_MAX_LONG_NAME_LEN define.  */
//		if (i == FX_MAX_LONG_NAME_LEN - 1) break;
		if (i == 256 - 1) break;

		/* Store the character.  */
		dest_ptr[i] = *source_ptr++;
		/* Increment the character counter.  */
		i++;
	}

	/* NULL-terminate the string.  */
	dest_ptr[i] = 0;

	/* Determine if we can backup to the previous character.  */
	if (i)
	{
		/* Yes, we can move backwards.  */
		i--;
	}

	/* Get rid of trailing blanks in the destination string.  */
	while (dest_ptr[i] == ' ')
	{
		/* Set this entry to NULL.  */
		dest_ptr[i] = 0;

		/* If there are still more characters, backup one!  */
		if (i)
		{
			/* Yes, there are move, move backwards one character.  */
			i--;
		}
		else
		{
			/* Get out of the loop.  */
			break;
		}
	}

	/* Determine if the source string is now at the end.  */
	if (*source_ptr == 0)
	{
		/* Yes, return a NULL pointer.  */
		source_ptr = 0;
	}

	/* Return the last pointer position in the source.  */
	return(source_ptr);

}
#endif

// ================================================================================

int yl_dmIsMounted(int drvnum) {
DRIVEMAN_COLLECTION_T *drv;

	if (drvnum>=MAX_DRIVE) return 0;

	drv=&g_DriveInfo[drvnum];
	if (BIT_VAL(drv->status,DM_STAT_MOUNTED)) return 1;

	return 0;
}


int yl_dmMount(int drvnum) {
#if 0
DRIVEMAN_COLLECTION_T *drv;
UINT status;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;

	drv=&g_DriveInfo[drvnum];
	if (!BIT_VAL(drv->status,DM_STAT_VALID)) {
		return YLERR_DRIVE;
	}
	if (BIT_VAL(drv->status,DM_STAT_MOUNTED)) return YLERR_OK;

	status = fx_media_open(
			drv->mp,				// Pointer to media control block.
			"",						// Pointer to media¡¦s name.
			(void (*)(FX_MEDIA_PTR))drv->driver,			// Pointer to I/O driver for this media.
			drv->ext,				// Pointer to optional information
			drv->buf,				// Pointer to the working memory for the media.
			drv->bufsize			// Specifies the size of the working media memory
	);

	switch (status) {
		case FX_SUCCESS:
			break;

		case FX_FAT_READ_ERROR:
			return YLERR_FS; // possible un-formated media

		case FX_MEDIA_INVALID:
		case FX_BOOT_ERROR:
			return YLERR_FAT;	// corrupt FAT

		case FX_IO_ERROR:
			return YLERR_IO;

		case FX_PTR_ERROR:
			return YLERR_INVAL;

		default:
			return YLERR_FAILURE;
	}

	BIT_SET(drv->status,DM_STAT_MOUNTED);

	return YLERR_OK;
#else
	return YLERR_DRIVE;
#endif
}


// ww: perhaps it is unnecessary, why do i want to un-mount a drvie on a DV platform ? card removed ?
int yl_dmUnmount(int drvnum) {
#if 0
DRIVEMAN_COLLECTION_T *drv;
UINT status;
int ret;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;

	drv=&g_DriveInfo[drvnum];
	if (!BIT_VAL(drv->status,DM_STAT_VALID)) {
		return YLERR_DRIVE;
	}

	status=fx_media_close(drv->mp);

	switch (status) {
		case FX_SUCCESS:
		case FX_MEDIA_NOT_OPEN:
			ret=YLERR_OK;
			break;

		case FX_IO_ERROR:
			ret=YLERR_IO;
			break;

		default:
			ret=YLERR_FAILURE;
	}

	BIT_CLR(drv->status,DM_STAT_MOUNTED);
	return ret;
#else
	return YLERR_DRIVE;
#endif

}


int yl_dmFormat(int drvnum, FORMAT_PARAM_T* param) {
#if 0
DRIVEMAN_COLLECTION_T *drv;
UINT status;
int ret;
DRIVE_GEO_T geo;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;

	drv=&g_DriveInfo[drvnum];
	if (!BIT_VAL(drv->status,DM_STAT_VALID)) {
		return YLERR_DRIVE;
	}
	if (!drv->ioctl) return YLERR_NOTSUPP;
	ret = drv->ioctl((DWORD)drv->ext, IOCTRL_DRVIVE_GET_GEO,(DWORD)&geo);
	if (ret<0) return YLERR_NOTSUPP;
	if (!geo.sec_size) return YLERR_NOTSUPP;
	if ((ret=geo.cylinders*geo.heads*geo.sectors)==0) return YLERR_NOTSUPP;

	status = fx_media_format(
		drv->mp,								// Pointer to media control block.
		(void (*)(FX_MEDIA_PTR))drv->driver,	// Driver entry
		drv->ext,								// driver_info_ptr
		(UCHAR*)drv->buf,						// Pointer to the working memory for the media.	//(UCHAR*) Jon modified.
		drv->bufsize,							// Specifies the size of the working media memory
		param?param->vol:"",					// Volume Name
        param?param->nFATs:2,					// Number of FATs
		param?param->nRootDirEnt:224,			// Directory Entries
		param?param->nHiddenSec:0,				// Hidden sectors
        ret,									// Total sectors
        geo.sec_size,							// Sector size
        geo.cluster,							// Sectors per cluster
        geo.heads,								// Heads
        geo.sectors);							// Sectors per track.

	switch (status) {
		case FX_SUCCESS:
			return 0;

		case FX_IO_ERROR:
			return YLERR_IO;
	}
	return YLERR_OK;
#else
	return YLERR_DRIVE;
#endif
}


int yl_dmParsePathW(const ucs2_t *path, char **out_short_path, void **which_media)
{
#if 0
	ucs2_t	 	*start = NULL;
	char		*shortpath = NULL;
	ucs2_t		*tmp_utf_path;
	int			len = 0, drvnum=0;
	int			rc = YLERR_OK;
	ULONG		unilen = 0;
	ucs2_t		*path_left = NULL;
	UCHAR 		sdst[MAX_PATH_LENGTH] = {0};
	FX_PATH		chg_dir;
	void* mp;

	mp = NULL;
//	*out_short_path = NULL;

	if (!path || !*path)
		return YLERR_INVLPATH;

	while (wisspace(*path)) path++;	// ww: eat up spaces

	if (path[1] == ':') {		/* manipulate a: b: c:...*/
		if (wisalpha(path[0])) {
			drvnum=toupper(path[0]) - 'A';
			mp = yl_dmGetFSHandle(drvnum);
			if (!mp) return YLERR_INVLPATH;
		}
		else {
			return YLERR_INVLPATH;
		}

		if (!out_short_path) goto out1;

		if (path[2])
			path += 2;
		else {
			shortpath = malloc(2);
			if (!shortpath)
				return YLERR_NOMEM;

			shortpath[0] = '/';
			shortpath[1] = '\0';
			*out_short_path = shortpath;
			goto out1;
		}
	}

	if (NULL == mp) {
		mp = lpiGetFSHandle();
		if (!mp) {
			mp=yl_dmGetFSHandle(0);
			lpiSetFSHandle(mp);		// set default media to a:
		}
	}

	drvnum=yl_dmGetDrvIndex(mp);

	if (!out_short_path) goto out1;

	if (wisdelim(path[0])) {
		len = wcslen(path) + 1;
		start = (ucs2_t *)malloc(sizeof(ucs2_t) * len);
		if (!start)
			return YLERR_NOMEM;
		memcpy(start, path, len * 2);
	}
	else {
//		tmp_utf_path = malloc (sizeof(ucs2_t) * FX_MAX_LONG_NAME_LEN);
//		if (!tmp_utf_path)
//			return YLERR_NOMEM;
//		wfullpath(tmp_utf_path, path, FX_MAX_LONG_NAME_LEN);
		tmp_utf_path=fullpathW((ucs2_t*)path);
		len = wcslen(tmp_utf_path + 2) + 1;	/* cut off drive name ex. a: */
		start = malloc(sizeof(ucs2_t) * len);
		if (!start) {
			rc = YLERR_NOMEM;
			goto done;
		}
		memcpy(start, tmp_utf_path + 2, len * 2);
		free(tmp_utf_path);

	}

	beautifypathW(start);

// ww modified 2008-1124 -->
// modified to +2 for not just the null character, but the tailing slash too.
// the old +1 will corrupt memory because the tailing slash & null destroy memory control mark
//	len = wcslen(start) + 1;
	len = wcslen(start) + 2;
// ww modified 2008-1124 <--
	shortpath = (CHAR *)malloc(len);
	if (!shortpath) {
		rc = YLERR_NOMEM;
		goto done;
	}

	path_left = start;
	fx_directory_local_path_set(mp, &chg_dir, "/");
	shortpath[0] = '/';
	shortpath[1] = '\0';
	do {
		UINT	entry_len = wcslen(path_left) + 1, i;
		ucs2_t 	dst_dir[entry_len];
		UCHAR 	utf_name[entry_len * 2];

// ww modified 2008-1125 -->
// fxied path_left [1 + i] will access memory out of its space
//		for (i = 0; i < wcslen(path_left + 1); i ++) {
		for (i = 0; i < wcslen(path_left); i ++) {
// ww fxied 2008-1125 <--
			if (wisdelim(path_left [1 + i])) {
				entry_len = (1 + i);
				break;
			}
		}

		path_left = _dir_wname_extract(path_left, dst_dir);
		unilen = wcslen(dst_dir);
		if (!unilen) break;
		if (MBStoASCII((char*)sdst, dst_dir, MAX_PATH_LENGTH) < 0) { // not purely ascii code.  //(char*) Jon modified.
			MBStoLESTR(utf_name, dst_dir, entry_len * 2);
			fx_unicode_short_name_get(mp, utf_name, unilen, (char*)sdst);	//(char*) Jon modified.
			if ('\0' == sdst[0]) {
				if (NULL == path_left)
					rc = YLERR_NOENT;		// this case is for file/directory create
				else
					rc = YLERR_INVLPATH;
				goto done;
			}
		}
		else {
			FX_DIR_ENTRY	entry;
			UINT			status;
			FX_MEDIA		*media_ptr = mp;
			FX_PROTECT
			entry.fx_dir_entry_name = (CHAR*)utf_name;		//(CHAR*)  Jon modified.
			entry.fx_dir_entry_short_name[0]= 0;
			status = fx_directory_search(media_ptr, (CHAR*)sdst, &entry, NULL, NULL); //(CHAR*)  Jon modify.
			FX_UNPROTECT

			if (status != FX_SUCCESS) {
				if (NULL == path_left)
					rc = YLERR_NOENT;		// this case is for file/directory create
				else
					rc = YLERR_INVLPATH;
				goto done;
			}
		}
		// dir
		if (fx_file_attributes_read(mp, (CHAR*)sdst, NULL) != FX_SUCCESS) {	//(CHAR*)  Jon modified.
			fx_directory_local_path_set(mp, &chg_dir, (CHAR*)sdst);			//(CHAR*)  Jon modified.
		}
		else {		// file
			if (path_left != NULL) {
				rc = YLERR_INVLPATH;
				goto done;
			}
		}
		strcat(shortpath, (CHAR*)sdst);	//(CHAR*)  Jon modified.
		strcat(shortpath, "/");
	} while (path_left);

done:
	fx_directory_local_path_clear(mp);
	if (YLERR_NOMEM == rc) {
		if (shortpath)
			free(shortpath);
		*out_short_path = NULL;
	}
	else {
		len = strlen(shortpath);
		if (len > 1) {
			if (isdelim(shortpath[len - 1]))
				shortpath[len - 1] ='\0';
		}
		*out_short_path = shortpath;
	}
	if (start)		free(start);
out1:
	if (which_media) *which_media=mp;
	return rc<0?rc:drvnum;
#else
	return 0;
#endif
}


/*
return value:
	<0  error number
	>=0 drive number of the path
*/
int yl_dmParsePath(const char* path, char **out_path, void **which_media) {
#if 0
char *start = NULL;
int len = 0, drvnum=0;
void *mp;
	mp = NULL;
//	*out_path = NULL;

	if (!path || !*path)
		return YLERR_INVLPATH;

	while (isspace((BYTE)*path)) path++;	// ww: eat up spaces. // (BYTE) Jon modified.

	if (path[1] == ':') {		/* manipulate a: b: c:...*/
		if (isalpha((BYTE)path[0])) {				// (BYTE) Jon modified.
			drvnum=toupper((BYTE)path[0]) - 'A';	// (BYTE) Jon modified.
			mp = yl_dmGetFSHandle(drvnum);
			if (!mp) return YLERR_INVLPATH;
		}
		else {
			return YLERR_INVLPATH;
		}

		if (!out_path) goto out1;

		if (path[2])
			path += 2;
		else {
			start = malloc(2);
			if (!start)
				return YLERR_NOMEM;

			start[0] = '/';
			start[1] = '\0';
			*out_path = start;
			goto out1;
		}
	}

	if (NULL == mp) {
		mp = lpiGetFSHandle();
		if (!mp) {
			mp=yl_dmGetFSHandle(0);
			lpiSetFSHandle(mp);		// set default media to a:
		}
	}

	drvnum=yl_dmGetDrvIndex(mp);

	if (!out_path) goto out1;

	if (isdelim((path[0]))) {
		start = strdup(path);
		if (!start)	{
			return	YLERR_NOMEM;
		}
	}
	else {
		register char *s;
//		s = fullpath(NULL, path, FX_MAX_LONG_NAME_LEN);
		s = fullpath((char*)path);
		if (!s)	{
			return	YLERR_NOMEM;
		}
		len = strlen(s + 2) + 1;
		start = malloc(len);
		if (!start)	{
			free(s);
			return	YLERR_NOMEM;
		}
		memcpy(start, s + 2, len);		/* cut off the first 2 bytes a: */
		free(s);
	}

	beautifypath(start);

	*out_path = start;

out1:
	if (which_media) *which_media=mp;
	return drvnum;
#else
	return 0;
#endif
}


/* --> 20081113 added by chris */
/* The service can only be called immediately after fx_media_open
 * and without any file system activity
 */
int yl_dmMediaCheckRepair(void *mp, DWORD correct_option, DWORD *error_detected) {
#if 0
	ULONG	total_clusters = ((FX_MEDIA*)mp)->fx_media_total_clusters;
	ULONG	scratch_size = (total_clusters >> 3) + 1024;	// recommended by FILEX
	UCHAR	*scratch_buf = NULL;
	UINT	status = FX_SUCCESS;
	int		rc = YLERR_OK;

	scratch_buf = malloc(scratch_size);
	if (!scratch_buf)
		return YLERR_NOMEM;

	status = fx_media_check(mp, scratch_buf, scratch_size, correct_option, (ULONG*)error_detected);

	if (status != FX_SUCCESS) {
		switch (status) {
		case FX_MEDIA_NOT_OPEN:
			rc = YLERR_DRIVE;
			break;
		case FX_NOT_ENOUGH_MEMORY:
			rc = YLERR_NOMEM;
			break;
		case FX_IO_ERROR:
			rc = YLERR_IO;
			break;
		case FX_ERROR_NOT_FIXED:
			rc = YLERR_FAT;
			break;
		default:
			rc = YLERR_FAILURE;
			break;
		}
	}

	if (scratch_buf)	free(scratch_buf);
	return rc;
#else
	return 0;
#endif
}
/* <-- end */


int yl_dmGetMediaFreeSpace(void *mp, DWORD *freespace_in_bytes)
{
#if 0
	int	rc = YLERR_OK;

	rc = fx_media_space_available(mp, (ULONG*)freespace_in_bytes);
	if (rc)
		return YLERR_DRIVE;

	return rc;
#else
	return 0;
#endif
}

int yl_dmGetMediaUsage(void *mp, DWORD *usage_in_bytes)
{
#if 0
	int rc = YLERR_OK;
	ULONG	total_in_bytes, available_in_bytes;

	total_in_bytes = ((FX_MEDIA*)mp)->fx_media_total_clusters * ((FX_MEDIA*)mp)->fx_media_sectors_per_cluster * ((FX_MEDIA*)mp)->fx_media_bytes_per_sector;
	rc |= fx_media_space_available(mp, &available_in_bytes);

	if (rc)
		return YLERR_DRIVE;
	else {
		*usage_in_bytes = total_in_bytes - available_in_bytes;
		return YLERR_OK;
	}
#else
	return 0;
#endif
}

int yl_dmGetMediaTotalSpace(void *mp, DWORD *total)
{
#if 0
	if (NULL == mp) {
		return YLERR_INVAL;
	}

	*total = ((FX_MEDIA*)mp)->fx_media_total_clusters * ((FX_MEDIA*)mp)->fx_media_sectors_per_cluster * ((FX_MEDIA*)mp)->fx_media_bytes_per_sector;

	return YLERR_OK;
#else
	return 0;
#endif
}

int yl_dmIOCtrl(int drvnum, DWORD arg1, DWORD arg2) {
DRIVEMAN_COLLECTION_T *drv;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;

	drv=&g_DriveInfo[drvnum];
	if (!BIT_VAL(drv->status,DM_STAT_VALID)) {
		return YLERR_DRIVE;
	}

	if (!drv->ioctl) return YLERR_NOTSUPP;

	return drv->ioctl((DWORD)drv->ext, arg1,arg2);
}


void* yl_dmGetFSHandle(int drvnum) {
DRIVEMAN_COLLECTION_T *drv;

	if (drvnum>=MAX_DRIVE) return NULL;
	drv=&g_DriveInfo[drvnum];
	if (!BIT_VAL(drv->status,DM_STAT_VALID)) {
		return NULL;
	}

	return drv->mp;
}


int yl_dmGetDrvIndex(void *mp) {
int i;

	if (mp == NULL) return	YLERR_DRIVE;

	for (i = 0; i < MAX_DRIVE; i ++) {
		if ((mp == g_DriveInfo[i].mp) && BIT_VAL(g_DriveInfo[i].status,DM_STAT_VALID) ) {
			return i;
		}
	}

	return (YLERR_FAILURE);
}


int yl_dmUnregister(int drvnum) {
DRIVEMAN_COLLECTION_T *drv;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;
	drv=&g_DriveInfo[drvnum];
	if (drv->mp) free(drv->mp);
	BIT_CLR(drv->status,DM_STAT_VALID);

	return 0;
}


int yl_dmRegister(int drvnum, REGISTER_DRIVE_T *rdrv) {
DRIVEMAN_COLLECTION_T *drv;

	if (drvnum>=MAX_DRIVE) return YLERR_DRIVE;
	drv=&g_DriveInfo[drvnum];

	if (!rdrv->driver) return YLERR_INVAL;
	if (!rdrv->bufsize) return YLERR_INVAL;

	drv->driver=rdrv->driver;
	drv->bufsize=rdrv->bufsize;
	drv->ext=rdrv->ext;

	// Jon added. 2013-0221 --->
	if (rdrv->addr){
		drv->buf = rdrv->addr; // Use driver buffer.
	}
	else{
		drv->buf=malloc(rdrv->bufsize);
	}
	// 2013-0221 <---

	if (!drv->buf) return YLERR_NOMEM;

	drv->ioctl=rdrv->ioctl;

//	drv->mp=calloc(1,sizeof(FX_MEDIA));
	drv->mp=calloc(1,sizeof(void*));
	if (!drv->mp) {
		if (!rdrv->addr){	// Jon added. 2013-0226
			free(drv->buf);
		}
		return YLERR_NOMEM;
	}

	BIT_SET(drv->status,DM_STAT_VALID);

	return 0;

}

