#include "platform/YL_FSIO.h"
#include <platform/YL_Sys.h>
//#include <fx_api.h>


#if 0
#define path_printf				yl_uartPrintf
#else
#define path_printf(...)
#endif

// =============================================================================
void pathcat(char* s1, char* s2) {
	if (!s2) return;
	if (!isdelim(s1[strlen(s1)-1])&&!isdelim(s2[0])) strcat(s1,"/");
	strcat(s1,s2);
}


void pathcatW(ucs2_t* s1, ucs2_t* s2) {
	if (!s2||!*s2) return;
	int s1len=wcslen(s1);
	if (!isdelim(s1[s1len-1])&&!isdelim(s2[0])) {
		s1[s1len++]='/';
		s1[s1len]=0;
	}
	wcscat(s1,s2);
}


// eat-up . .. & /
void beautifypath(char* start) {
char *s , *p ;

	p = s = start;

	do {
		if (isdelim(*p)) {
			if (isdelim(*s)) {	/* skip duplicate (or initial) slash */
				continue;
			}
			if (*s == '.') {
				if (!s[1] || isdelim(s[1])) {	/* remove extra '.' */
					continue;
				}
				if ((s[1] == '.') && (!s[2] || isdelim(s[2]))) {
					++s;
					if (p > start) {
						while (!isdelim(*--p))	/* omit previous dir */
							continue;
					}
					continue;
				}
			}
		}
		*++p = *s;
	} while (*++s);

	if ((p == start) || (!isdelim(*p))) {	/* not a trailing slash */
		++p;					/* so keep last character */
	}
	*p = 0;
}


void beautifypathW(ucs2_t* start) {
ucs2_t *s , *p ;

	p = s = start;

	do {
		if (isdelim(*p)) {
			if (isdelim(*s)) {	/* skip duplicate (or initial) slash */
				continue;
			}
			if (*s == '.') {
				if (!s[1] || isdelim(s[1])) {	/* remove extra '.' */
					continue;
				}
				if ((s[1] == '.') && (!s[2] || isdelim(s[2]))) {
					++s;
					if (p > start) {
						while (!isdelim(*--p))	/* omit previous dir */
							continue;
					}
					continue;
				}
			}
		}
		*++p = *s;
	} while (*++s);

	if ((p == start) || (!isdelim(*p))) {	/* not a trailing slash */
		++p;					/* so keep last character */
	}
	*p = 0;
}


DWORD inc_align(DWORD p) {

	if (p&0x3) {
		while (p&0x3) p++;
	}
	return p+4;
}


void  yl_splitpath(const char* inpath, char *out) {
const char *p, *end, *ext;
char** item=(char**)out;
char* path=out+(4*sizeof(char*));
int len;

	memset(out,0,4*sizeof(char*));

	while (isspace((BYTE)*inpath)) inpath++;	// ww: eat up spaces.	// (BYTE) Jon modified.

	if(inpath[0] && inpath[1] == ':') {
		item[0]=path;
		*path++ = *inpath++;
		*path++ = *inpath++;
		*path++ = 0;
	}

	/* look for end of directory part */
	end = NULL;
	for(p = inpath; *p; p++){
		if (isdelim(*p))
			end = p + 1;
//			end = p;
	}

	/* got a directory */
	if(end){
		len=end - inpath;
		if (len>1) len--;
		path=(char*)inc_align((DWORD)path);
		item[1]=path;

		memcpy( path, inpath, len );
		path+=(len);
		*path++ = 0;

		inpath = end;
	}

	/* look for extension: what's after the last dot */
	if (*inpath) {
//		if (isdelim(*inpath)) inpath++;

		end = NULL;
		for(p = inpath; *p; p++){
			if (*p == '.')
				end = p;
		}
		if (end) ext =p;
		else {
			ext=0;
			end=p;
		}
		len=end-inpath;

		if (len) {
			path=(char*)inc_align((DWORD)path);

			item[2]=path;

			memcpy( path, inpath, len );
			path+=len;
			*path++ =0;
		}

		if (ext) {
			path=(char*)inc_align((DWORD)path);
			if (!item[2]) {
				item[2]=path;
			}
			else {
				item[3]=path;
			}
			strcpy( path, end );
		}
	}
}


void yl_makepath(char *path, const char *drive, const char *dir,
		const char *filename, const char *ext) {

	char *ptemp = path;

	if ( !ptemp) return;

	if (drive && (drive[1]==':') ) {
		*ptemp++ = *drive;
		*ptemp++ = ':';
	}

	if (dir && *dir) {
		unsigned int len = strlen(dir);

		if (!isdelim(*dir))
			*ptemp++ = '/';
		strcpy(ptemp, dir);
		ptemp += len;
	}

	if (!isdelim(ptemp[-1]))
		*ptemp++ = '/';

	if (filename && *filename) {
		unsigned int len = strlen(filename);
		strcpy(ptemp, filename);
		ptemp += len;
	}
	if (ext && *ext) {
		if (*ext != '.')
			*ptemp++ = '.';
		strcpy(ptemp, ext);
		ptemp += strlen(ext);
	}

	*ptemp = '\0';

	beautifypath(path+2);

}


char* yl_fullpath(char* ppart) {
char *pbuf;
char *cwd;
char *spcwd;
char *fpath;
char *fullstr=NULL;

	pbuf=malloc(SP_MAX_PATH);
	if (!pbuf) return 0;

	fpath=malloc(MAX_PATH_LENGTH);
	if (!fpath) goto errfp1;

	splitpath(ppart,pbuf);
	spcwd=malloc(SP_MAX_PATH);
	if (!spcwd) goto errfp2;

	cwd=getcwd(0,0);
	if (!cwd) goto errfp2;
	splitpath(cwd,spcwd);

	free(cwd);

	if (SP_NAME(spcwd)) {
		pathcat(SP_DIR(spcwd),SP_NAME(spcwd));
	}
	if (SP_EXT(spcwd)) {
		strcat(SP_DIR(spcwd),SP_EXT(spcwd));
	}

	if (!SP_DRV(pbuf)) {
		SP_DRV(pbuf)=SP_DRV(spcwd);
		if (SP_DIR(pbuf)) {
			if (!isdelim(*SP_DIR(pbuf))) { // path need to be concatnated !
				pathcat(SP_DIR(spcwd),SP_DIR(pbuf));
				SP_DIR(pbuf)=SP_DIR(spcwd);
			}
		}
		else {
			SP_DIR(pbuf)=SP_DIR(spcwd);
		}
	}
	else if (SP_DRV(spcwd)) {
		if (tolower((BYTE)*SP_DRV(pbuf))==tolower((BYTE)*SP_DRV(spcwd))) { // same drive number
			if (!SP_DIR(pbuf)) SP_DIR(pbuf)=SP_DIR(spcwd);
			else if (!isdelim(*SP_DIR(pbuf))) { // path need to be concatnated !
				pathcat(SP_DIR(spcwd),SP_DIR(pbuf));
				SP_DIR(pbuf)=SP_DIR(spcwd);
			}
		}
	}

	makepath(fpath,SP_DRV(pbuf),SP_DIR(pbuf),SP_NAME(pbuf),SP_EXT(pbuf));

	fullstr=fpath;

	free(spcwd);

errfp2:
	if (!fullstr) free(fpath);

errfp1:
	free(pbuf);

	if (fullstr) {
		fullstr=strdup(fpath);
		free(fpath);
	}

	return fullstr;

}


// combines splited to drv:/dir & name.ext
// to access by SP_DRV & SP_NAME macros
void specialcombine(char* pathbuf) {
	if (SP_DRV(pathbuf)) {
		if (SP_DIR(pathbuf)) {
			pathcat(SP_DRV(pathbuf),SP_DIR(pathbuf));
		}
	}
	else SP_DRV(pathbuf)=SP_DIR(pathbuf);
	if (SP_NAME(pathbuf)) {
		if (SP_EXT(pathbuf)) {
			strcat(SP_NAME(pathbuf),SP_EXT(pathbuf));
		}
	}
	else SP_NAME(pathbuf)=SP_EXT(pathbuf);
}

// ================

void  yl_splitpathW(const ucs2_t* inpath, ucs2_t *out) {
const ucs2_t *p, *end, *ext;
ucs2_t** item=(ucs2_t**)out;
ucs2_t* path=out+((4*sizeof(char*))/2);
int len;

	memset(out,0,4*sizeof(char*));

	while (wisspace(*inpath)) inpath++;	// ww: eat up spaces

	if(inpath[0] && inpath[1] == ':') {
		item[0]=path;
		*path++ = *inpath++;
		*path++ = *inpath++;
		*path++ = 0;
	}

	/* look for end of directory part */
	end = NULL;
	for(p = inpath; *p; p++){
		if (wisdelim(*p))
			end = p + 1;
	//			end = p;
	}

	/* got a directory */
	if(end){
		len=end - inpath;
		if (len>1) len--;
		path=(ucs2_t*)inc_align((DWORD)path);
		item[1]=path;

		memcpy( path, inpath, len*2 );
		path+=(len);
		*path++ = 0;

		inpath = end;
	}

	/* look for extension: what's after the last dot */
	if (*inpath) {
//		if (wisdelim(*inpath)) inpath++;

		end = NULL;
		for(p = inpath; *p; p++){
			if (*p == '.')
				end = p;
		}
		if (end) ext =p;
		else {
			ext=0;
			end=p;
		}
		len=end-inpath;

		if (len) {
			path=(ucs2_t*)inc_align((DWORD)path);

			item[2]=path;

			memcpy( path, inpath, len*2 );
			path+=len;
			*path++ =0;
		}

		if (ext) {
			path=(ucs2_t*)inc_align((DWORD)path);
			if (!item[2]) {
				item[2]=path;
			}
			else {
				item[3]=path;
			}
			wcscpy( path, end );
		}
	}
}


void yl_makepathW(ucs2_t *path, const ucs2_t *drive, const ucs2_t *dir,
		const ucs2_t *filename, const ucs2_t *ext) {

	ucs2_t *ptemp = path;

	if ( !ptemp) return;

	if (drive && (drive[1]==':') ) {
		*ptemp++ = *drive;
		*ptemp++ = ':';
	}

	if (dir && *dir) {
		unsigned int len = wcslen(dir);

		if (!wisdelim(*dir))
			*ptemp++ = '/';
		wcscpy(ptemp, dir);
		ptemp += len;
	}

	if (!wisdelim(ptemp[-1]))
		*ptemp++ = '/';

	if (filename && *filename) {
		unsigned int len = wcslen(filename);
		wcscpy(ptemp, filename);
		ptemp += len;
	}
	if (ext && *ext) {
		if (*ext != '.')
			*ptemp++ = '.';
		wcscpy(ptemp, ext);
		ptemp += wcslen(ext);
	}

	*ptemp = '\0';

	beautifypathW(path+2);

}


ucs2_t* yl_fullpathW(ucs2_t* ppart) {
	ucs2_t *pbuf;
	ucs2_t *cwd;
	ucs2_t *spcwd;
	ucs2_t *fpath;
	ucs2_t *fullstr=NULL;

	pbuf=malloc(SP_MAX_PATH*2);
	if (!pbuf) return 0;

	fpath=malloc(MAX_PATH_LENGTH*2);
	if (!fpath) goto errfp1;

	splitpathW(ppart,pbuf);
	spcwd=malloc(SP_MAX_PATH*2);
	if (!spcwd) goto errfp2;

	cwd=getcwdW(0,0);
	if (!cwd) goto errfp2;
	splitpathW(cwd,spcwd);

	free(cwd);

	if (WSP_NAME(spcwd)) {
		pathcatW(WSP_DIR(spcwd),WSP_NAME(spcwd));
	}
	if (WSP_EXT(spcwd)) {
		wcscat(WSP_DIR(spcwd),WSP_EXT(spcwd));
	}

	if (!WSP_DRV(pbuf)) {
		WSP_DRV(pbuf)=WSP_DRV(spcwd);
		if (WSP_DIR(pbuf)) {
			if (!wisdelim(WSP_DIR(pbuf)[0])) { // path need to be concatnated !
				pathcatW(WSP_DIR(spcwd),WSP_DIR(pbuf));
				WSP_DIR(pbuf)=WSP_DIR(spcwd);
			}
		}
		else {
			WSP_DIR(pbuf)=WSP_DIR(spcwd);
		}
	}
	else if (WSP_DRV(spcwd)) {
		int d1,d2;
		d1=WSP_DRV(pbuf)[0];
		d2=WSP_DRV(spcwd)[0];

		if (d1==d2) {
//		if (tolower(WSP_DRV(pbuf)[0])==tolower(WSP_DRV(spcwd)[0])) { // same drive number
			if (!WSP_DIR(pbuf)) WSP_DIR(pbuf)=WSP_DIR(spcwd);
			else if (!wisdelim(WSP_DIR(pbuf)[0])) { // path need to be concatnated !
				pathcatW(WSP_DIR(spcwd),WSP_DIR(pbuf));
				WSP_DIR(pbuf)=WSP_DIR(spcwd);
			}
		}
	}

	makepathW(fpath,WSP_DRV(pbuf),WSP_DIR(pbuf),WSP_NAME(pbuf),WSP_EXT(pbuf));

	fullstr=fpath;

	free(spcwd);

errfp2:
	if (!fullstr) free(fpath);

errfp1:
	free(pbuf);

	if (fullstr) {
		fullstr=wcsdup(fpath);
		free(fpath);
	}

	return fullstr;

}


// combines splited to drv:/dir & name.ext
// to access by SP_DRV & SP_NAME macros
void specialcombineW(ucs2_t* pathbuf) {
	if (WSP_DRV(pathbuf)) {
		if (WSP_DIR(pathbuf)) {
			pathcatW(WSP_DRV(pathbuf),WSP_DIR(pathbuf));
		}
	}
	else WSP_DRV(pathbuf)=WSP_DIR(pathbuf);
	if (WSP_NAME(pathbuf)) {
		if (WSP_EXT(pathbuf)) {
			wcscat(WSP_NAME(pathbuf),WSP_EXT(pathbuf));
		}
	}
	else WSP_NAME(pathbuf)=WSP_EXT(pathbuf);
}


// ============================================================================
// ============================================================================
// ============================================================================
// ============================================================================

#if 0
char* yl_concatpath(char* absPath, const char* relPath, unsigned int size)
{

	char* buffer = NULL;
	BOOL alloced = FALSE;
	WORD bufferlength = 0;
	WORD relpathlength = 0;

	if (!relPath || !*relPath)
		return getcwd(absPath, size);

	if (absPath == NULL)
	{
		buffer = malloc(MAX_PATH_LENGTH);
		if (!buffer) return NULL;
		size = MAX_PATH_LENGTH;
		alloced = TRUE;
	}
	else
		buffer = absPath;

	if (size < 4)
		goto yl_concatepathEXIT;

	if(alloced)
		getcwd(buffer, size);
	bufferlength = strlen(buffer);
	while(bufferlength>0)
	{
		if((buffer[(bufferlength-1)]=='/')||(buffer[(bufferlength-1)]=='\\')){
			buffer[(bufferlength-1)] = 0;
			bufferlength--;
		}
		else
			break;
	}
	relpathlength = strlen(relPath);
	while(relpathlength>0)
	{
		if((relPath[0]=='/')||(relPath[0]=='\\'))
		{
			relPath = relPath +1;
			relpathlength--;
		}
		else
			break;
	}
	if((bufferlength+relpathlength)<=size)
	{
		strcat(buffer,"/");
		strcat(buffer,relPath);
		return buffer;
	}

yl_concatepathEXIT:
	if (alloced && buffer)
		free(buffer);
	return NULL;

}
#endif
#if 0
ucs2_t* yl_unicodeConcatPath(ucs2_t* absPath,const ucs2_t* relPath)
{
	if(!relPath||!*relPath)
		return NULL;
	ucs2_t *pstart = absPath;
	while (*absPath)
	    absPath++;

	if (absPath != pstart) {
		if (*(absPath - 1) != '/') {
			*(absPath ++) = '/';
		}
	}
  	while (*relPath){
		*absPath = *relPath;
		absPath++;
		relPath++;
  	}
	if ('/' == *(absPath - 1))
		absPath --;
	*absPath = 0;

  	return pstart;
}
#endif

// =============================================================================
#if 0
void yl_makepath(char * path, const char * drive,
                     const char *dir, const char * filename,
                      const char * ext)
{
	char *ptemp = path;

	path_printf("(%s %s %s %s)\n", drive, dir,filename, ext);

	if( !ptemp){
		return;
	}

	if(drive && drive[0] && drive[1])
	{
		*ptemp++ = drive[0];
		*ptemp++ = ':';
		*ptemp++ = '/';
	}
	if(dir && dir[0])
	{
		unsigned int len = strlen(dir);
		memcpy(ptemp, dir, len);
		ptemp += len;
		if(ptemp[-1] != '/' && ptemp[-1] != '\\')
			*ptemp++ = '/';
	}
	if(filename && filename[0])
	{
		unsigned int len = strlen(filename);
		memcpy(ptemp, filename, len);
		ptemp += len;
	}
	if(ext && ext[0])
	{
		if(ext[0] != '.')
			*ptemp++ = '.';
		strcpy(ptemp, ext);
		ptemp += strlen(ext);
		*ptemp = '\0';
	}
	else
		*ptemp = '\0';
	path_printf("returning %s\n",path);
}
#endif

#if 0
void yl_unicodeMakepath(ucs2_t* path, const ucs2_t* drive,
	                          const ucs2_t* dir, const ucs2_t* filename, const ucs2_t* ext)
{
	ucs2_t *ptemp = path;

	if( !path )
		return;

	if(drive && drive[0])
	{
		*ptemp++ = drive[0];
		*ptemp++ = ':';
		*ptemp++ = '/';
	}
	if(dir && dir[0])
	{
		WORD len = wcslen((ucs2_t*)dir);
		memcpy(ptemp, dir, (len*sizeof(ucs2_t)));
		ptemp += len;
		if(ptemp[-1] != ('/'))
			*ptemp++ = '/';
	}
	if(filename && filename[0])
	{
		WORD len = wcslen((ucs2_t*)filename);
		memcpy(ptemp, filename, (len*sizeof(ucs2_t)));
		ptemp += len;
	}
	if(ext && ext[0])
	{
		if(ext[0] != ('.'))
			*ptemp++ = '.';

		wcscat((ucs2_t*)ptemp, (ucs2_t*)ext);
		ptemp += wcslen((ucs2_t*)ext);
		*ptemp = 0;
	}
	else
		*ptemp = 0;
}
#endif
#if 0
char* yl_fullpath(char* absPath, const char* relPath, unsigned int size)
{
	if((NULL == absPath)&&(NULL != relPath)){
		return yl_concatpath(NULL, relPath,0);
	}

	getcwd(absPath, size);
	return yl_concatpath(absPath, relPath,size);
}
#endif
#if 0
void  yl_splitpath(const char* inpath, char * drv, char * dir,
					char* fname, char * ext )
{
	const char *p, *end;

	if(inpath[0] && inpath[1] == ':'){
		if (drv){
			drv[0] = inpath[0];
			drv[1] = inpath[1];
			drv[2] = inpath[2];
			drv[3] = 0;
		}
		inpath += 3;
	}
	else if (drv)
		drv[0] = 0;

	/* look for end of directory part */
	end = NULL;
	for(p = inpath; *p; p++){
		if (*p == '/' || *p == '\\')
			//end = p + 1;
			end = p;
	}

	/* got a directory */
	if(end){
		if(dir){
			memcpy( dir, inpath, end - inpath );
			dir[end - inpath] = 0;
		}
		inpath = end+1;
	}
	else if(dir)
		dir[0] = 0;

	/* look for extension: what's after the last dot */
	end = NULL;
	for(p = inpath; *p; p++){
		if (*p == '.')
			end = p;
	}

	if(!end)
		end = p; /* there's no extension */

	if(fname){
		memcpy( fname, inpath, end - inpath );
		fname[end - inpath] = 0;
	}
	if(ext)
		strcpy( ext, end );
}
#endif
#if 0
void yl_unicodeSplitpath(ucs2_t* inpath, ucs2_t* drv,
	                           ucs2_t* dir, ucs2_t* filename, ucs2_t* ext)
{
	ucs2_t *p, *end;

	if(inpath[0] && inpath[1] == ':'){
		if (drv){
			drv[0] = inpath[0];
			drv[1] = inpath[1];
			drv[2] = inpath[2];
			drv[3] = 0;
		}
		inpath += 3;
	}
	else if (drv)
		drv[0] = 0;

	/* look for end of directory part */
	end = NULL;
	for(p = inpath; *p; p++){
		if (*p == '/')
			end = p;
	}

	/* got a directory */
	if(end){
		if(dir){
			memcpy( dir, inpath, (end - inpath)*sizeof(ucs2_t) );
			dir[end - inpath] = 0;
		}
		inpath = end+1;
	}
	else if(dir)
		dir[0] = 0;

	/* look for extension: what's after the last dot */
	end = NULL;
	for(p = inpath; *p; p++){
		if (*p == '.')
			end = p+1;
	}

	if(!end)
		end = p; /* there's no extension */

	if(filename){
		memcpy( filename, inpath, (end - inpath)*sizeof(ucs2_t));
		filename[end - inpath] = 0;
	}

	if(ext)
		wcscat( (ucs2_t*)ext, (ucs2_t*)end );
}
#endif
#if 0
ucs2_t* yl_unicodeFullpath(ucs2_t* absPath,const ucs2_t* relPath,unsigned int size)
{
	if(!relPath||!*relPath)
		return wgetcwd(absPath,size);

	wgetcwd(absPath,size);
	WORD rel_length = wcslen(relPath);
	WORD abs_length = wcslen(absPath);
	if((abs_length+rel_length)<size)
		yl_unicodeConcatPath(absPath,(ucs2_t *)relPath);
	return absPath;
}
#endif

#if 0
WORD* yl_wfullpath(WORD* absPath, const WORD* relPath, unsigned int size)
 {
   DWORD rc;
   WCHAR* buffer;
   WCHAR* lastpart;
   BOOL alloced = FALSE;

   if (!relPath || !*relPath)
     return _wgetcwd(absPath, size);

   if (absPath == NULLs)
   {
       buffer = MSVCRT_malloc(MAX_PATH * sizeof(WCHAR));
       size = MAX_PATH;
       alloced = TRUE;
   }
   else
       buffer = absPath;

   if (size < 4)
   {
     *MSVCRT__errno() = MSVCRT_ERANGE;
     return NULL;
   }

   TRACE(":resolving relative path %s\n",debugstr_w(relPath));

   rc = GetFullPathNameW(relPath,size,buffer,&lastpart);

   if (rc > 0 && rc <= size )
     return buffer;
   else
   {
       if (alloced)
           MSVCRT_free(buffer);
         return NULL;
   }
 }
#endif

#if 0
void uconPathtest(int argc, char** argv){
	char totalfullpath[MAX_PATH_LENGTH];
	char tdrive[20];
	char tdir[128];
	char tfilename[128];
	char t_ext[20];
	int tsize = 0;
	char* rtnpath = NULL;
	if (argc>=2) {
		if(strcmp(argv[1],"makepath")==0){
			if(argv[2]!=NULL){
				strncpy(tdrive,argv[2],20);
			}
			if(argv[3]!=NULL){
				strncpy(tdir,argv[3],128);
			}
			if(argv[4]!=NULL){
				strncpy(tfilename,argv[4],128);
			}
			if(argv[5]!=NULL){
				strncpy(t_ext,argv[5],20);
			}
			yl_makepath(totalfullpath, tdrive, tdir, tfilename, t_ext);
			printf("totalfullpath b::%s\n",totalfullpath);
		}
		if(strcmp(argv[1],"fullpath")==0){
			if(argv[2]!=NULL){
				strncpy(totalfullpath,argv[2],MAX_PATH_LENGTH);
			}
			if(argv[3]!=NULL){
				strncpy(tdir,argv[3],128);
			}
			if(argv[4]!=NULL){
				tsize = atoi(argv[4]);
			}
			mkdir("qazwsx");
			chdir("qazwsx");
			mkdir("qwerasdf");
			chdir("qwerasdf");
			rtnpath = yl_fullpath(totalfullpath, tdir, tsize);
			printf("rtnpath b::%s\n",rtnpath);
		}

		if(strcmp(argv[1],"f1")==0){
			mkdir("qazwsx");
			chdir("qazwsx");
			mkdir("qwerasdf");
			chdir("qwerasdf");
			rtnpath = yl_fullpath(NULL, "////dsfk\\\\", 30);
			printf("rtnpath b::%s\n",rtnpath);
		}

		if(strcmp(argv[1],"f2")==0){
			//char atest[20]={0};
			//rtnpath = yl_fullpath(atest, "////dsfk\\\\", 20);
			//path_printf("rtnpath b::%s\n",rtnpath);
//			mkdir("qazwsx");
//			chdir("qazwsx");
//			mkdir("qwerasdf");
//			chdir("qwerasdf");
			//yl_wgetcwd((WORD*)totalfullpath, 128);
			WORD testpath[128] = {0};
			//WORD trel[10] = {0x2f00,0x3f00,0x4f00,0x5f00,0};
			WORD trel[10] = {0x002f,0x003f,0x004f,0x005f,0};
			//yl_unicodeFullpath(testpath,trel,128);
			yl_fullpath((char*)testpath,(char*)trel,128);



			//WORD testfname[128]={0x2d00,0x3d00,0x4d00,0x5d00,0};
			WORD testfname[128]={0x002d,0x003d,0x004d,0x005d,0};
			//WORD testext[10]={0x2e00,0x3300,0x4f00};
			WORD testext[10]={0x002e,0x0033,0x004f};
			WORD testpath_2[128] = {0};

			//yl_unicodeMakepath(testpath_2, testpath, trel, testfname, testext);
			yl_makepath((char*)testpath_2,(char*)testpath,(char*)trel,(char*)testfname,(char*)testext);

			WORD testdrv[10]={0};
			WORD testdir[128]={0};
			WORD testfname_2[128]={0};
			WORD testext_2[10]={0};
			//yl_unicodeSplitpath(testpath_2, testdrv, testdir, testfname_2, testext_2);
			yl_splitpath((char*)testpath_2,(char*)testdrv,(char*)testdir,(char*)testfname_2,(char*)testext_2);
			WORD testext_3[10]={0};



		}

		if(strcmp(argv[1],"splitpath")==0){
			yl_splitpath("a:fasf/teask/ffff\\\\\\\\ee/aaa/bbb\\\\bccc/cccc.txt", tdrive, tdir, tfilename, t_ext);
			path_printf("%s %s %s %s\n",tdrive,tdir,tfilename,t_ext);
		}
		//yl_wgetcwd((WORD*)totalfullpath, 128);
	}

}
#endif

#if 0
DWORD GetFullPathNameA( char* name, DWORD len, char* buffer,
                            char* lastpart )
{
	WCHAR *nameW;
	WCHAR bufferW[MAX_PATH];
	DWORD ret;

	if (!(nameW = FILE_name_AtoW( name, FALSE ))) return 0;

	ret = GetFullPathNameW( nameW, MAX_PATH, bufferW, NULL);

	if (!ret) return 0;
	if (ret > MAX_PATH){
		SetLastError(ERROR_FILENAME_EXCED_RANGE);
		return 0;
	}
	ret = copy_filename_WtoA( bufferW, buffer, len );
	if (ret < len && lastpart){
		LPSTR p = buffer + strlen(buffer) - 1;

		if (*p != '\\'){
			while ((p > buffer + 2) && (*p != '\\')) p--;
			*lastpart = p + 1;
		}
		else
			*lastpart = NULL;
	}
	return ret;
}
#endif

#if 0
 VOID CDECL _wmakepath(MSVCRT_wchar_t *path, const MSVCRT_wchar_t *drive, const MSVCRT_wchar_t *directory,
                       const MSVCRT_wchar_t *filename, const MSVCRT_wchar_t *extension)
 {
     MSVCRT_wchar_t *p = path;

     TRACE("%s %s %s %s\n", debugstr_w(drive), debugstr_w(directory),
           debugstr_w(filename), debugstr_w(extension));

     if ( !path )
         return;

     if (drive && drive[0])
     {
         *p++ = drive[0];
         *p++ = ':';
     }
     if (directory && directory[0])
     {
         unsigned int len = wcslen(directory);
         memmove(p, directory, len * sizeof(MSVCRT_wchar_t));
         p += len;
         if (p[-1] != '/' && p[-1] != '\\')
             *p++ = '\\';
     }
     if (filename && filename[0])
     {
         unsigned int len = wcslen(filename);
         memmove(p, filename, len * sizeof(MSVCRT_wchar_t));
         p += len;
     }
     if (extension && extension[0])
     {
         if (extension[0] != '.')
             *p++ = '.';
         wcscpy(p, extension);
     }
     else
         *p = '\0';

     TRACE("returning %s\n", debugstr_w(path));
 }

 /*********************************************************************
  *              _wsplitpath (MSVCRT.@)
  *
  * Unicode version of _splitpath.
  */
 void CDECL _wsplitpath(const MSVCRT_wchar_t *inpath, MSVCRT_wchar_t *drv, MSVCRT_wchar_t *dir,
                        MSVCRT_wchar_t *fname, MSVCRT_wchar_t *ext )
 {
     const MSVCRT_wchar_t *p, *end;

     if (inpath[0] && inpath[1] == ':')
     {
         if (drv)
         {
             drv[0] = inpath[0];
             drv[1] = inpath[1];
             drv[2] = 0;
         }
         inpath += 2;
     }
     else if (drv) drv[0] = 0;

     /* look for end of directory part */
     end = NULL;
     for (p = inpath; *p; p++) if (*p == '/' || *p == '\\') end = p + 1;

     if (end)  /* got a directory */
     {
         if (dir)
         {
             memcpy( dir, inpath, (end - inpath) * sizeof(MSVCRT_wchar_t) );
             dir[end - inpath] = 0;
         }
         inpath = end;
     }
     else if (dir) dir[0] = 0;

     /* look for extension: what's after the last dot */
     end = NULL;
     for (p = inpath; *p; p++) if (*p == '.') end = p;

     if (!end) end = p; /* there's no extension */

     if (fname)
     {
         memcpy( fname, inpath, (end - inpath) * sizeof(MSVCRT_wchar_t) );
         fname[end - inpath] = 0;
     }
     if (ext) wcscpy( ext, end );
 }

 /*********************************************************************
  *              _wfullpath (MSVCRT.@)
  *
  * Unicode version of _fullpath.
  */




#endif

