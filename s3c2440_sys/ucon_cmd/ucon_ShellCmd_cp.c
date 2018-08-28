#include <errno.h>
#include "platform/YL_Sys.h"
#include <platform/YL_FSIO.h>
#include "platform/yl_errno.h"
#include <platform/YL_I18N.h>
#include "platform/TQ_BSP.h"

#define CP2_CHUNK_SIZE	4096

int copy2cwd(BYTE* fullsrc, ucs2_t* wdst) {
	FILE* fp_src = NULL;
	FILE* fp_dst = NULL;
	int ret;
	BYTE *buf;

	fp_src = fopen((char*)fullsrc,"r");
	if (!fp_src) {
		printf("failed to open source file\n");
		ret=errno;
		goto cp2_out1;
	}
	fp_dst = fopenW(wdst,"w+");
	if (!fp_dst) {
		printf("failed to create target file\n");
		ret=errno;
		goto cp2_out2;
	}

	buf = malloc((DWORD)CP2_CHUNK_SIZE);
	if (!buf) {
		printf("unable to allocate memory\n");
		ret=YLERR_NOMEM;
		goto cp2_out3;
	}

	while (1) {
	int rb,wb;
		rb = fread(buf,CP2_CHUNK_SIZE,fp_src);
		if (rb==EOF) break;
		else if (rb<0) {
			printf("copy failed\n");
			ret=rb;
			goto cp2_out4;
		}
		wb = fwrite(buf,rb,fp_dst);
		if ((wb<0)||(rb!=wb)) {
			printf("copy failed\n");
			ret=wb;
			goto cp2_out4;
		}
	}

	ret=0;

cp2_out4:
	free(buf);

cp2_out3:
	fclose(fp_dst);

cp2_out2:
	fclose(fp_src);

cp2_out1:
	return ret;
}


#define XCOPY_MAX_DEPTH		12
#define XCOPY_RECURSIVE		0x010000
#define XCOPY_OVERWRITE		0x020000
#define XCOPY_NOASKDIR		0x040000
#define XCOPY_ABORT			0x080000
#define XCOPY_QUIET			0x100000
#define XCOPY_WRITETIME		0x200000

//int max_xcopy_depth=0;



int xcopy1(ffblk *pff, ucs2_t *wdpath, ucs2_t *wdname, DWORD *xmode, int *pcount) {
int srclen,dstlen;
int ret,findend;
struct stat st;
int offset,size;
ucs2_t *wnew_file;

//	if ((BYTE)*pff->xmode>max_xcopy_depth) max_xcopy_depth=(BYTE)*pff->xmode;
	if ((BYTE)*xmode>XCOPY_MAX_DEPTH) return 0;

	ret=chdirW(wdpath);
	if (ret) {
		printf("Invalid destination path %s\n",wdpath);
		goto out0;
	}

	findend=find_first(pff);

	while(!findend) {

		if ( (pff->ff_name[0]=='.')&&( (pff->ff_name[1]==0)||(pff->ff_name[1]=='.') ) ) goto go_next;

		if (pff->ff_attr&FA_DIR) {
			if (!(*xmode&XCOPY_RECURSIVE)) goto go_next;
			 // going in !!
			ret=statW(pff->ff_longname,&st);
			if (ret) { // not exist
				ret=mkdirW(pff->ff_longname);
				if (ret) {
					printf("level %d: failed to make dir %s %s\n",(BYTE)*xmode,wdpath,pff->ff_name);
					goto out0;
				}
			}
			else if (!(*xmode&XCOPY_NOASKDIR)&&!(*xmode&XCOPY_QUIET)) { // overwrite ?
				char* answer;

				askdiragain:
				printf("Directory %s is already exist, Continue [Y]es/[N]o/[F]orce/[A]bort? ",pff->ff_name);
				answer=uconAsk();
				switch (tolower((BYTE)*answer)) {
					case 'f':
						*xmode|=XCOPY_NOASKDIR;

					case 'y':
						break;

					case 'a':
						*xmode|=XCOPY_ABORT;
						ret=-1;
						goto out0;

					case 'n':
						continue;

					default:
						goto askdiragain;
				}
			}

			//remember stuff ....
			srclen=strlen(pff->search_path);	// remember length
			dstlen=wcslen(wdpath);
			pathcatW(wdpath,pff->ff_longname);
			pathcat(pff->search_path,pff->ff_name);
			offset = pff->entry_offset;
			size = pff->cwd_num_entries;

//			(BYTE)(*xmode)++;
			(*((BYTE*)xmode))++;
			ret=xcopy1(pff,wdpath,wdname,xmode,pcount);
//			(BYTE)(*xmode)--;
			(*((BYTE*)xmode))--;

			pff->entry_offset = offset;
			pff->cwd_num_entries = size;
			pff->search_path[srclen]=0;
			wdpath[dstlen]=0;
			chdirW(wdpath);
			if (ret) goto out0;

		}
		else if (!(pff->ff_attr&FA_LABEL)&&wildcmp(pff->rule_set,pff->ff_name)) {
			if (!wdname) wnew_file=pff->ff_longname;
			else wnew_file=wdname;
			srclen=strlen(pff->search_path);	// remember length
			pathcat(pff->search_path,pff->ff_name);
			if (!(*xmode&XCOPY_QUIET)) printf("copying %s ...\n",pff->search_path);
			ret=statW(wnew_file,&st);
			if (!ret&&!(*xmode&XCOPY_OVERWRITE)&&!(*xmode&XCOPY_QUIET)) {
				char* answer;

				askfileagain:
				printf("%s is already exist, Overwrite [Y]es/[N]o/[F]orce/[A]bort? ","FILE");
				answer=uconAsk();
				switch (*answer) {
					case 'f':
						*xmode|=XCOPY_OVERWRITE;

					case 'y':
						break;

					case 'a':
						*xmode|=XCOPY_ABORT;
						ret=-1;
						goto out0;

					case 'n':
						continue;

					default:
						goto askfileagain;
				}
			}
 			copy2cwd((BYTE*)pff->search_path,wnew_file);
 			(*pcount)++;
 			pff->search_path[srclen]=0;
#if 0
			if (!(*xmode&XCOPY_WRITETIME)) {
				struct tm ntime;
				char shortname[13];
				ret=fx_directory_short_name_get(__THREAD_CUR_MEDIA, wnew_file, shortname);
				localtime(&pff->ff_time,&ntime);
				fx_file_date_time_set(__THREAD_CUR_MEDIA, shortname,
	                YEAR0+ntime.tm_year, ntime.tm_mon, ntime.tm_mday,
	                ntime.tm_hour, ntime.tm_min, ntime.tm_sec);
			}
#endif
		}

go_next:

		ret=tq_uartGetCharNB();
		if (ret<0) goto out0;

		findend=find_next(pff);
	}

	ret=0;
out0:
	return ret;
}


#if 0
int xcopy(char *spath, char *sname, char *dpath, char *dname, DWORD *xmode, int *pcount) {
int ret;
struct stat st;
char *src1;
char *dst1;
int nameisfile,wlen;
ffblk ff;
ucs2_t *wdpath,*wdname;

	nameisfile=1;
	src1=malloc(MAX_PATH_LENGTH);
	if (!src1) {
		printf("Unable to allocate memory\n");
		goto out0;
	}
	if (sname) {
		strcpy(src1,spath);
		pathcat(src1,sname);
		ret=stat(src1,&st);
		if (!ret) { // exist
			if (S_ISDIR(st.st_mode)) { // src name is a directory
				sname="*";
				nameisfile=0;
			}
		}
	}
	if (nameisfile) {
		strcpy(src1,spath);
	}
	spath=src1;

	nameisfile=1;
	dst1=malloc(MAX_PATH_LENGTH*sizeof(ucs2_t));
	if (!dst1) {
		printf("Unable to allocate memory\n");
		goto out1;
	}
	wdname=0;
	if (dname) {
		strcpy((char*)dst1,dpath);
		pathcat((char*)dst1,dname);
		ret=stat((char*)dst1,&st);
		if (!ret) {
			if (S_ISDIR(st.st_mode)) {
				nameisfile=0;
			}
		}
		if (nameisfile) {
			wlen=(strlen(dname)+1);
			wdname=malloc(wlen*sizeof(ucs2_t));
			if (!wdname) {
				printf("Unable to allocate memory\n");
				goto out2;
			}
			ret=ASCIItoMBS(wdname,dname,wlen);
			if (ret) {
				printf("File name is not allowed\n");
				goto out3;
			}
		}
	}
	if (nameisfile) {
		strcpy((char*)dst1,dpath);
	}
	chdir(dst1);
	wdpath=(ucs2_t*)dst1;
	wgetcwd(wdpath,MAX_PATH_LENGTH*sizeof(ucs2_t));

	memset(&ff,0,sizeof(ffblk));

	ff.pfx_path=malloc(sizeof(FX_PATH));
	if (!ff.pfx_path) {
		printf("Unable to allocate memory\n");
		goto out3;
	}

	memset(ff.pfx_path,0,sizeof(FX_PATH));

	ff.search_path=spath;
	ff.rule_set=sname;

	ret=xcopy1(&ff,wdpath,wdname,xmode,pcount);

	free(ff.pfx_path);

out3:
	if (wdname) free(wdname);

out2:
	free(dst1);

out1:
	free(src1);

out0:

	return ret;
}
#else
int xcopy(char *spath, char *sname, char *dpath, char *dname, DWORD *xmode, int *pcount) {
int ret=0;
struct stat st;
char *src1;
char *dst1;
int nameisfile,wlen;
ffblk *ff;
ucs2_t *wdpath,*wdname;

	nameisfile=1;
	src1=malloc(MAX_PATH_LENGTH);
	if (!src1) {
		printf("Unable to allocate memory\n");
		goto out0;
	}
	if (sname) {
		strcpy(src1,spath);
		pathcat(src1,sname);
		ret=stat(src1,&st);
		if (!ret) { // exist
			if (S_ISDIR(st.st_mode)) { // src name is a directory
				sname="*";
				nameisfile=0;
			}
		}
	}
	if (nameisfile) {
		strcpy(src1,spath);
	}
	spath=src1;

	nameisfile=1;
	dst1=malloc(MAX_PATH_LENGTH*sizeof(ucs2_t));
	if (!dst1) {
		printf("Unable to allocate memory\n");
		goto out1;
	}
	wdname=0;
	if (dname) {
		strcpy((char*)dst1,dpath);
		pathcat((char*)dst1,dname);
		ret=stat((char*)dst1,&st);
		if (!ret) {
			if (S_ISDIR(st.st_mode)) {
				nameisfile=0;
			}
		}
		if (nameisfile) {
			wlen=(strlen(dname)+1);
			wdname=malloc(wlen*sizeof(ucs2_t));
			if (!wdname) {
				printf("Unable to allocate memory\n");
				goto out2;
			}
			ret=ASCIItoMBS(wdname,dname,wlen);
			if (ret) {
				printf("File name is not allowed\n");
				goto out3;
			}
		}
	}
	if (nameisfile) {
		strcpy((char*)dst1,dpath);
	}
	chdir(dst1);
	wdpath=(ucs2_t*)dst1;
	getcwdW(wdpath,MAX_PATH_LENGTH*sizeof(ucs2_t));

	ff=find_init();
	if (!ff) {
		printf("Unable to allocate memory\n");
		goto out3;
	}

	ff->search_path=spath;
	ff->rule_set=sname;

	ret=xcopy1(ff,wdpath,wdname,xmode,pcount);

	free(ff);


out3:
	if (wdname) free(wdname);

out2:
	free(dst1);

out1:
	free(src1);

out0:

	return ret;
}
#endif

void uconcopy(int argc, char** argv) {
int s;
DWORD xmode=0;
char *src,*dst;
char *fsrc, *fdst;
char fsrcsp[SP_MAX_PATH];
char fdstsp[SP_MAX_PATH];
char *old_cwd;
char** sp;
int count;

	getopt(0,0,0); // reset getopt state
	while ((s=getopt(argc,argv,"rfqh")) != EOF) {
		switch (s) {
			case 'r':
				xmode|=XCOPY_RECURSIVE;
				break;
			case 'f':
				xmode|=XCOPY_OVERWRITE|XCOPY_NOASKDIR;
				break;
			case 'q':
				break;
			case 'h':
				printf(	"Usage: %s [switches] <source> <destination>\n"
						"    -r      copy directories recursively\n"
						"    -f      force overwrite existing files\n"
						"    -q      quiet mode\n"
						"Example:\n"
						"    cp -r -f a:/*.*  b:\n"
						,argv[0]);
				return;
			default:
				break;
		}
	}

	if((argc-optind)>0){
		src=argv[optind++];
	}
	else{
		printf("no source provided.\n");
		return;
	}

	if((argc-optind)>0){
		dst=argv[optind++];
	}
	else {
		dst=".";
	}

	old_cwd=getcwd(0,0);
	if (!old_cwd) {
		printf("unable to retrive current path");
		return;
	}

	fsrc=fullpath(src);
	if (!fsrc) {
		printf("failed to get full path\n");
		goto out1;
	}
	fdst=fullpath(dst);
	if (!fdst) {
		printf("failed to get full path\n");
		goto out2;
	}

	splitpath(fsrc, fsrcsp);
	specialcombine(fsrcsp);

	splitpath(fdst, fdstsp);
	specialcombine(fdstsp);

	count=0;
	sp=(char**)fsrcsp;
	s=xcopy(SP_DRV(sp),SP_NAME(sp),SP_DRV(sp),SP_NAME(sp),&xmode,&count);
	if (s<0) {
		printf("error: %d\n",s);
	}
	else if (s>0) {
		printf("User canceled.\n");
	}

	chdir(old_cwd);

	printf("%d file%c copied.\n",count,count>1?'s':' ');


//out3:
	free(fdst);

out2:
	free(fsrc);

out1:
	free(old_cwd);

}




#if 0
int xcopy(char *spath, char *sname, char *dpath, char *dname, DWORD *xmode) {
int ret;
struct stat st;
char *src1;
ucs2_t *dst1;
int srclen,dstlen;
int findend;
struct ffblk ff;

	if ((BYTE)*xmode>max_xcopy_depth) max_xcopy_depth=(BYTE)*xmode;

	if ((BYTE)*xmode>XCOPY_MAX_DEPTH) return 0;

	if ((BYTE)*xmode==0) {
		srclen=1;
		src1=malloc(MAX_PATH_LENGTH);
		if (!src1) {
			printf("Unable to allocate memory\n");
			goto out0;
		}
		if (sname) {
			strcpy(src1,spath);
			pathcat(src1,sname);
			ret=stat(src1,&st);
			if (!ret) { // exist
				if (S_ISDIR(st.st_mode)) { // src name is a directory
					sname="*";
					srclen=0;
				}
			}
		}
		if (srclen) {
			strcpy(src1,spath);
			srclen=0;
		}
		spath=src1;

		dstlen=1;
		dst1=malloc(MAX_PATH_LENGTH*sizeof(ucs2_t));
		if (!dst1) {
			printf("Unable to allocate memory\n");
			goto out1;
		}
		if (dname) {
			strcpy((char*)dst1,dpath);
			pathcat((char*)dst1,dname);
			ret=stat((char*)dst1,&st);
			if (!ret) {
				if (S_ISDIR(st.st_mode)) {
					dname=0;
					dstlen=0;
				}
			}
		}
		if (dstlen) {
			strcpy((char*)dst1,dpath);
			dstlen=0;
		}
		dpath=(char*)dst1;
		wgetcwd(dst1,MAX_PATH_LENGTH*sizeof(ucs2_t));
	}
	srclen=strlen(spath);
	dstlen=strlen(dpath);

	ret=chdir(dpath);
	if (ret) {
		printf("Invalid destination path %s\n",dpath);
		goto out2;
	}

	memset(&ff,0,sizeof(struct ffblk));
	ff.search_dir=spath;
	findend=find_first1(&ff);

	while(!findend) {

		if ( (ff.ff_name[0]=='.')&&(ff.ff_name[1]==0) ) goto go_next;
		if ( (ff.ff_name[0]=='.')&&(ff.ff_name[1]=='.') ) goto go_next;

		if (ff.ff_attr&FA_DIR) {
			if (!(*xmode&XCOPY_RECURSIVE)) goto go_next;
			 // going in !!
			ret=stat(ff.ff_name,&st);
			if (ret) { // not exist
				ret=mkdir(ff.ff_name);
				if (ret) {
					printf("level %d: failed to make dir %s %s\n",(BYTE)*xmode,dpath,ff.ff_name);
					goto out2;
				}
			}
			else if (!(*xmode&XCOPY_NOASKDIR)&&!(*xmode&XCOPY_QUIET)) { // overwrite ?
				char* answer;

				askdiragain:
				printf("Directory %s is already exist, Continue [Y]es/[N]o/[F]orce/[A]bort? ",ff.ff_name);
				answer=uconAsk();
				switch (tolower(*answer)) {
					case 'f':
						*xmode|=XCOPY_NOASKDIR;

					case 'y':
						break;

					case 'a':
						*xmode|=XCOPY_ABORT;
						ret=-1;
						goto out2;

					case 'n':
						continue;

					default:
						goto askdiragain;
				}
			}

			pathcat(dpath,ff.ff_name);
			pathcat(spath,ff.ff_name);
			(BYTE)(*xmode)++;
			ret=xcopy(spath,sname,dpath,dname,xmode);
			(BYTE)(*xmode)--;

			spath[srclen]=0;
			dpath[dstlen]=0;
			chdir(dpath);
			if (ret) goto out2;

		}
		else if (!(ff.ff_attr&FA_LABEL)&&wildcmp(sname,ff.ff_name)) {
			char *new_file;
			if (!dname) new_file=ff.ff_name;
			else new_file=dname;
			ret=stat(new_file,&st);
			if (!ret&&!(*xmode&XCOPY_OVERWRITE)&&!(*xmode&XCOPY_QUIET)) {
				char* answer;

				askfileagain:
				printf("%s is already exist, Overwrite [Y]es/[N]o/[F]orce/[A]bort? ",new_file);
				answer=uconAsk();
				switch (*answer) {
					case 'f':
						*xmode|=XCOPY_OVERWRITE;

					case 'y':
						break;

					case 'a':
						*xmode|=XCOPY_ABORT;
						ret=-1;
						goto out2;

					case 'n':
						continue;

					default:
						goto askfileagain;
				}
			}
			pathcat(spath,ff.ff_name);
			if (!(*xmode&XCOPY_QUIET)) printf("copying %s ...\n",spath);
			copy2cwd(spath,new_file);
			spath[srclen]=0;
/*
			if (!(*xmode&XCOPY_WRITETIME)) {
				struct tm ntime;
				localtime(&ff.ff_time,&ntime);
				fx_file_date_time_set(__THREAD_CUR_MEDIA, new_file,
	                1900+ntime.tm_year, ntime.tm_mon, ntime.tm_mday,
	                ntime.tm_hour, ntime.tm_min, ntime.tm_sec);
			}*/
		}

go_next:

		ret=yl_uartGetCharNB();
		if (ret==0x03) goto out2;

		findend=find_next1(&ff);
	}

	ret=0;

out2:
	if (!((BYTE)*xmode)) free(dst1);

out1:
	if (!((BYTE)*xmode)) free(src1);

out0:

	return ret;
}
#endif
