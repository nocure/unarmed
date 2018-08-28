#include "platform/YL_FSIO.h"
#include <platform/YL_Sys.h>
#include <platform/YL_Errno.h>
#include <platform/YL_I18N.h>
//#include <fx_api.h>


int fx_big5_name_get(void* media_ptr, char *src_short_name, UCHAR *dest_big5_name, int size)
{
#if 0
	int				i, rc = YLERR_OK;
//	FX_MEDIA 		*media_ptr = NULL;
//	char			*abspath = NULL;
	UINT			status;
    UCHAR			unicode_name[FX_MAX_LONG_NAME_LEN * 2];
    UCHAR			big5_name[FX_MAX_LONG_NAME_LEN * 2];
    UCHAR			*ucs2lestr = NULL, *big5str = NULL;
    ULONG			unilen = 0;
    size_t			inlen, outlen;

//    yl_dmParsePath(".", &abspath, &media_ptr);
	status = fx_unicode_name_get((FX_MEDIA*)media_ptr, src_short_name, unicode_name, &unilen);
// ww 2013-0321 { added error handling
	switch (status) {
		case FX_SUCCESS:
			break;

		case FX_NOT_FOUND:
			rc = YLERR_NOENT;
			goto done;

		case FX_MEDIA_NOT_OPEN:
		case FX_PTR_ERROR:
		case FX_CALLER_ERROR:
		case FX_IO_ERROR:
		default:
			rc = YLERR_INVAL;
			goto done;
	}
// ww 2013-0321 } added error handling

	if (unilen > 0) {
		ucs2lestr = unicode_name;
		big5str = big5_name;
		inlen = unilen * 2;
		outlen = sizeof(big5_name);
		memset(big5str, 0, outlen);
		rc = fx_ucs2_to_big5str(&ucs2lestr, &inlen, &big5str, &outlen);
		if (rc)	{
			dbgout("%s: error translation between unicode and big5\n", __FUNCTION__);
			goto done;
		}

		for (i = 0; big5_name[i]; i ++) {
			if (i >= size-1) {
				rc = YLERR_INVAL;
				break;
			}
			dest_big5_name[i]= big5_name[i];
		}
		dest_big5_name[i] = '\0';
	}
	else {
		rc = YLERR_NOENT;
	}

done:
//	if (abspath)	free(abspath);
	return rc;
#else
	return 0;
#endif
}


