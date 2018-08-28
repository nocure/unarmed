#include "platform/YL_Sys.h"
#include <platform/YL_FSIO.h>
#include <platform/YL_I18N.h>
//#include <fx_api.h>

#if 0
#define FCOPY_PRINTF				yl_uartPrintf
#else
#define FCOPY_PRINTF(...)
#endif


//COPYMODE
#define CP_MAX_DEPTH 5
#define CP_BYTE 0x10//0~15 record folder depth
#define CP_OW (1<<5) //overwrite
#define CP_RC (1<<6) //recursive
#define CP_FOLDER (1<<7)
#define CP_FILE (1<<8)
#define CP_SHOWPROGRESS (1<<9)
#define CP_SHOWSHORTNAME (1<<10)
#define CP_UNICODE (1<<11)
#define CP_ASCII (1<<12)
#define CP_MTIME (1<<13)//copy file modify time

#define CP_MINIMUM_DRIVE 0
#define CP_MAXIMUN_DRIVE 25

//Return Value
enum{
	CP_OK = 0,
	CP_FAIL,
	CP_FILE_NOT_EXIT,
	CP_NULL_DEST_PATH,
	CP_NULL_SRC_PATH,//NULL source path
	CP_DEST_EXIT,
	CP_ALLOCATE_MEMORY_FAIL,
	CP_WRITE_FAIL,
	CP_OPEN_FAIL,
	CP_FILE_READ_ONLY,
};


typedef struct {
	char* SrcPath;
//	fspec_t* search;
//	fdata_t* matched;
	ffblk *ff;
} CP_del;
#if 0
typedef struct {
	char SrcPath[MAX_PATH_LENGTH];
	char DestPath[MAX_PATH_LENGTH];
	fspec_t* search;
	fdata_t* matched;//short name
} CP_info;
#endif
typedef struct {
	DWORD totalsize;
	DWORD foldercount;
	DWORD filecount;
	DWORD mode;
} CP_DIR_INFO;


//COPY Maximum size
#define CP_MAXSIZE 1024
//COPY Maximun string length

//return value, 0::Exist, 1::not Exist
int CP_ISEXIST(char* pathname, int typemode)
{
	struct stat st;
	int st_rtn = -1;
	if(typemode&CP_UNICODE){
		st_rtn = statW((const ucs2_t*)pathname,&st);
	}
	else{
		st_rtn = stat(pathname,&st);
	}
	if(st_rtn < 0){
		FCOPY_PRINTF("CP_ISEXIST %s is not exist\n",pathname);
		return CP_FAIL;
	}
	return CP_OK;
}

//return value, 0::is folder, 1::not a folder, 2::file not exist
int CP_ISFOLDER(char* pathname){
	struct stat st;
	int st_rtn = -1;
	st_rtn = stat(pathname,&st);
	if(st_rtn < 0){
		FCOPY_PRINTF("CP_ISFOLDER %s is not exist\n",pathname);
		return CP_FILE_NOT_EXIT;
	}

	if(S_ISDIR(st.st_mode))
		return CP_OK;
	else
		return CP_FAIL;
}

//return value, 0::read only, 1::not a folder, 2::file not exist
int CP_ISREADONLY(char* pathname){
	struct stat st;
	int st_rtn = -1;
	st_rtn = stat(pathname,&st);
	if(st_rtn < 0){
		FCOPY_PRINTF("CP_ISREADONLY %s is not exist\n",pathname);
		return CP_FILE_NOT_EXIT;
	}
	if(st.st_mode&DT_RDONLY)
		return CP_OK;
	else
		return CP_FAIL;
}

int CP_GetDir(char* src, char* outpath){
	int rtn = CP_FAIL;
	int size = strlen(src);
	if(size<4){
		return rtn;
	}

	while(size>0){
		if(isdelim(src[size]))
			break;
		size--;
	}
	strncpy(outpath,src,size);
	outpath[size]=0;
	rtn = CP_OK;
	return rtn;
}

long CP_GetFileSize(char* pathname){
	struct stat st;
	int st_rtn = -1;
	st_rtn = stat(pathname,&st);
	if(st_rtn < 0){
		FCOPY_PRINTF("CP_GetFileSize %s is not exist\n",pathname);
		return 0;
	}
	return st.st_size;
}

/*
funcion name::CP_SplitPathName
parameter description:
src input fullpathname, output fullpath
name                         output name
*/
int CP_SplitPathName(char* src, char* name){
	int rtn = CP_FAIL;
	int size = strlen(src);
	char temp[MAX_PATH_LENGTH]={0};
	int tcount = 0;
	int i=0;
	if(size<4){
		return rtn;
	}

	while(size>0){
		if(isdelim(src[size]))
			break;
		temp[tcount]=src[size];
		src[size]=0;
		tcount++;
		size--;
	}
	src[size]=0;

	//reverse string
	for(i=0;i<tcount;i++){
		name[i]=temp[(tcount-1-i)];
	}
	name[i+1]=0;

	rtn = CP_OK;
	return rtn;
}

int CP_GetFullPathName(char* npara,char* container){
	int rtn = CP_FAIL;
	if(!container)
		return rtn;

	char* pathname = NULL;
	int driveIndex;
	driveIndex=yl_dmParsePath(npara,&pathname,0);
	if(driveIndex>=0){
		if((driveIndex>=CP_MINIMUM_DRIVE)&&(driveIndex<=CP_MAXIMUN_DRIVE)){
			sprintf(container,"%c:%s",'a'+ driveIndex,pathname);
			rtn = CP_OK;
		}
		else{
			printf(" wrong disc drive!! ");
		}
	}
	if(pathname)
		free(pathname);

	return rtn;
}

int CP_ModifyTime(char* filename,char* src_pathname){
	int rtn = CP_FAIL;
#if 0
	if((!filename)||(!src_pathname))
		return rtn;

	struct stat st;
	int st_rtn = -1;
	st_rtn = stat(src_pathname,&st);
	if(st_rtn < 0){
		FCOPY_PRINTF("CP_ModifyTime %s is not exist\n",src_pathname);
		return rtn;
	}

	char* buffer2 = NULL;
	void* pmedia =NULL;
	yl_dmParsePath(".", &buffer2, &pmedia);

	DWORD tt= fx_file_date_time_set(pmedia, filename,
			st.st_mtime.tm_year, st.st_mtime.tm_mon, st.st_mtime.tm_mday,
			st.st_mtime.tm_hour, st.st_mtime.tm_min, st.st_mtime.tm_sec);
	if(tt==0)
		rtn = CP_OK;
	else
		FCOPY_PRINTF("modify time error");

	if(buffer2)
		free(buffer2);
#endif
	return rtn;
}

int CP_GetShortName(char* out_sname, ucs2_t* longname){
	int rtn = CP_FAIL;
#if 0
	if((!out_sname)||(!longname))
		return rtn;

	char* buffer2 = NULL;
	void* pmedia =NULL;
	BYTE tmp[MAX_PATH_LENGTH * 2]={0};
	yl_dmParsePath(".", &buffer2, &pmedia);
	MBStoLESTR(tmp, longname, MAX_PATH_LENGTH);
	fx_unicode_short_name_get(pmedia, tmp, fx_unicode_length_get(tmp), out_sname);
	if(strlen(out_sname)>0){
		rtn = CP_OK;
	}
	if(buffer2)
		free(buffer2);
#endif
	return rtn;
}

//modify get long name rule
DWORD CP_GetLongName(char* pathname, ucs2_t* longname){
	unsigned long unicodelength = 0;
#if 0
	char path[MAX_PATH_LENGTH]={0};
	char filename[MAX_PATH_LENGTH]={0};
	char fsname[MAX_PATH_LENGTH]={0};
	ucs2_t trans[MAX_PATH_LENGTH]= {0};
	char rec_cwd[MAX_PATH_LENGTH]= {0};
	strcpy(path,pathname);
	CP_SplitPathName(path,filename);
	if(strlen(filename)<=0)
		return 0;

	getcwd(rec_cwd,MAX_PATH_LENGTH);
	chdir(path);

	char* buffer2 = NULL;
	void* pmedia =NULL;
	yl_dmParsePath(".", &buffer2, &pmedia);

	fx_directory_short_name_get(pmedia, filename, fsname);
	fx_unicode_name_get(pmedia, fsname, (UCHAR*)trans, (ULONG*)&unicodelength);

	LESTRtoMBS((ucs2_t *)longname, (const BYTE *)trans, MAX_PATH_LENGTH);

	if(buffer2)
		free(buffer2);

	chdir(rec_cwd);
#endif
	return unicodelength;
}
#if 0
/*
function name::CP_CreateLongNameFolder
data should have destination path and file longname
record new folder short name in fsname
*/
int CP_CreateLongNameFolder(CP_info* data, char* fsname){

	int rtn = CP_FAIL;
	char rec_cwd[MAX_PATH_LENGTH]= {0};
	getcwd(rec_cwd,MAX_PATH_LENGTH);
	int aa = chdir(data->DestPath);
	int rtnExist = CP_FAIL;

	if(data->matched->shortened)//longname folder
		rtnExist = CP_ISEXIST((char*)data->matched->unicode_name,CP_UNICODE);
	else//ASCII name folder
		rtnExist = CP_ISEXIST((char*)data->matched->entry_name,CP_ASCII);

	if(CP_OK==rtnExist){
			rtn = CP_OK;
		}
		else{
		if(data->matched->shortened)
			rtn = wmkdir(data->matched->unicode_name);
		else
			rtn = mkdir(data->matched->entry_name);

			if(rtn)
				printf("mkdir error::%d",rtn);
			else{
				rtn = CP_OK;
			}
		}

	if((fsname)&&(rtn == CP_OK)){
		if(data->matched->shortened){
			CP_GetShortName(fsname,data->matched->unicode_name);
		}
		else{
			strcpy(fsname,data->matched->entry_name);
		}
		char tbuf[MAX_PATH_LENGTH]={0};
		if(data->SrcPath[strlen(data->SrcPath)-1]=='/')
			sprintf(tbuf,"%s%s",data->SrcPath,data->matched->entry_name);
		else
			sprintf(tbuf,"%s/%s",data->SrcPath,data->matched->entry_name);
		CP_ModifyTime(fsname,tbuf);
	}
	aa = chdir(rec_cwd);
	return rtn;
}

/*
function name::CP_DestCheck
parameter
dest input fullpath, output fullpath+src folder name
src   input fullpath
mode file type
*/
//int CP_DestCheck(char* dest,char* src,int mode){
int CP_DestCheck(CP_info* data,int mode){
	int rtn = CP_FAIL;
	if(CP_GetLongName(data->SrcPath, data->matched->unicode_name)>0){
		char strtmp[MAX_PATH_LENGTH]={0};
		strcpy(strtmp,data->SrcPath);
		CP_SplitPathName(strtmp, data->matched->entry_name);
		if(CP_OK==CP_CreateLongNameFolder(data,data->matched->entry_name))
			rtn = CP_OK;
	}
	return rtn;
}

void CP_ProgressInit(char* src, char* dest, int cpmode){
	if(cpmode&CP_SHOWPROGRESS){
		char cwd[MAX_PATH_LENGTH]={0};
		getcwd(cwd,MAX_PATH_LENGTH);
		if(cwd[strlen(cwd)-1]!='/')
			strcat(cwd,"/");
		printf("%s -> %s%s %3d%%",src,cwd,dest,0);
	}
}
void CP_ProgressUpdate(DWORD numerator, DWORD denominator, int cpmode){
	if(cpmode&CP_SHOWPROGRESS){
		if(numerator>denominator){
			FCOPY_PRINTF("CP_ProgressUpdate error\n");
			return;
		}
		DWORD percentage = (numerator*100)/denominator;
		int i = 0;
		for(;i<4;i++){
			yl_uartPutChar(0x08);
		}
		printf("%3d%%",percentage);
	}
}
void CP_ProgressClose(int cpstatus,int cpmode){
	if(cpmode&CP_SHOWPROGRESS){
		if(CP_OK==cpstatus){
			CP_ProgressUpdate(1, 1,cpmode);
			printf("\n");
		}
		else{
			printf("  fail\n");
		}
	}
}


int yl_FileSingleCopy(CP_info* data, int cpmode){
	if(!data)
		return CP_NULL_SRC_PATH;
	int rtn = CP_FAIL;
	FILE* fp_dest = NULL;
	FILE* fp_src = NULL;
	int r_size = CP_MAXSIZE;
	int wrtn = 0;
	void* rbuffer = NULL;
	DWORD cpfilesize=0;
	DWORD copyingsize = 0;

	char rec_cwd[MAX_PATH_LENGTH]={0};
	char src_fpname[MAX_PATH_LENGTH]={0};
	if(cpmode&CP_FILE)
		sprintf(src_fpname,"%s",data->SrcPath);
	else
		sprintf(src_fpname,"%s/%s",data->SrcPath,data->matched->entry_name);
	getcwd(rec_cwd,MAX_PATH_LENGTH);
	int chrtn = chdir(data->DestPath);

	if(CP_FAIL==CP_ISEXIST(src_fpname,CP_ASCII)){
		rtn = CP_FILE_NOT_EXIT;
		return rtn;
	}

	//check if overwrite or not
	if(!(cpmode&CP_OW)){
		int filestaus;
		if(data->matched->shortened)
			filestaus = CP_ISEXIST((char*)data->matched->unicode_name, CP_UNICODE);
		else
			filestaus = CP_ISEXIST((char*)data->matched->entry_name, CP_ASCII);
		if(CP_OK==filestaus){
			if(cpmode&CP_SHOWPROGRESS)
				printf("warning::%s exist already\n",data->matched->entry_name);
			rtn = CP_DEST_EXIT;
			return rtn;
		}
	}
	CP_ProgressInit(src_fpname, data->matched->entry_name, cpmode);

	fp_src = fopen(src_fpname,"r");
	if(data->matched->shortened){//longname file
		fp_dest = wfopen(data->matched->unicode_name,"w+");
	}
	else//ASCII name file
		fp_dest = fopen(data->matched->entry_name,"w+");

	if((!fp_src)||(!fp_dest)){
		printf("open file fail\n");
		rtn = CP_OPEN_FAIL;
		goto yl_FileSingleCopyEXIT;
	}

	rbuffer = malloc((DWORD)CP_MAXSIZE);
	if(!rbuffer){
		printf("memory allocated fail\n");
		rtn = CP_ALLOCATE_MEMORY_FAIL;
		goto yl_FileSingleCopyEXIT;
	}

	cpfilesize = CP_GetFileSize(src_fpname);
	while(r_size==CP_MAXSIZE){
		r_size = fread((void*)rbuffer,CP_MAXSIZE,fp_src);

		//check if end of file
		if(r_size==EOF)
			break;
		wrtn = fwrite(rbuffer,r_size,fp_dest);
		if(r_size != wrtn){
			FCOPY_PRINTF("fwrite fail,er::%d\n",wrtn);
			rtn = CP_WRITE_FAIL;
			goto yl_FileSingleCopyEXIT;
		}
		if(cpmode&CP_SHOWPROGRESS){
			copyingsize = copyingsize + wrtn;
			if(copyingsize>(20*CP_MAXSIZE))
				CP_ProgressUpdate(copyingsize ,cpfilesize, cpmode);
		}
	}
	rtn = CP_OK;
	FCOPY_PRINTF("copy a file complete\n");
	//FCOPY_PRINTF("from %s to %s\n",src_pathname,dest_pathname);
yl_FileSingleCopyEXIT:

	if(fp_src)
		fclose(fp_src);
	if(fp_dest)
		fclose(fp_dest);
	if(rbuffer)
		free(rbuffer);

	CP_ProgressClose(rtn,cpmode);

	//copy modify time
	if(CP_OK == rtn){
		if(data->matched->shortened){
			char shortname[MAX_PATH_LENGTH]={0};
			CP_GetShortName(shortname, data->matched->unicode_name);
			CP_ModifyTime(shortname,src_fpname);
		}
		else{
			CP_ModifyTime(data->matched->entry_name,src_fpname);
		}
	}

	chrtn = chdir(rec_cwd);

	return rtn;
}


int yl_FolderCopy(CP_info* data, int cpmode){
	int rtn = CP_FAIL;
	char icheck = 0;
	ULONG path_entry_Num = data->matched->cwd_num_entries;
	ULONG entryfilesize = data->matched->entry_offset;
	int cdrtn = chdir(data->SrcPath);

	if(find_first(data->matched, data->search)){
		FCOPY_PRINTF("there is no data\n");
		goto yl_FolderCopyEXIT;
	}

	do{
		FCOPY_PRINTF("find_next searched: %s\n", data->matched->entry_name);
		if(strcmp(data->matched->entry_name,".")==0)
			continue;
		if(strcmp(data->matched->entry_name,"..")==0)
			continue;

		icheck = CP_ISFOLDER(data->matched->entry_name);
		if(CP_OK == icheck){
			if(!(cpmode&CP_RC))//not recursive mode will skip folder copy
				continue;
			if((cpmode%CP_BYTE)<CP_MAX_DEPTH){
				char strtemp[MAX_PATH_LENGTH]={0};//store new folder short name
				if(CP_OK!=CP_CreateLongNameFolder(data,strtemp)){
					continue;
				}

				if(data->SrcPath[strlen(data->SrcPath)-1]!='/')
					strcat(data->SrcPath,"/");
				strcat(data->SrcPath,data->matched->entry_name);

				if(data->DestPath[strlen(data->DestPath)-1]!='/')
					strcat(data->DestPath,"/");
				strcat(data->DestPath,strtemp);//add new folder short name

				yl_FolderCopy(data,cpmode+1);

			}
		}
		else if(CP_FAIL == icheck){
			yl_FileSingleCopy(data, cpmode);
		}
		else{
			FCOPY_PRINTF("there is no files\n");
		}
	}while(!find_next(data->matched, data->search));
	rtn = CP_OK;
yl_FolderCopyEXIT:
	//erase current directory to upper dir, matched->entry_name is just a buffer for CP_SplitPathName.
	CP_SplitPathName(data->SrcPath,data->matched->entry_name);
	CP_SplitPathName(data->DestPath,data->matched->entry_name);
	cdrtn = chdir(data->SrcPath);
	data->matched->cwd_num_entries = path_entry_Num;
	data->matched->entry_offset = entryfilesize;
	return rtn;
}
#endif

int CP_DeleteFile(char* pathname, int cpmode){
	int rtn = -1;
	rtn = unlink(pathname);
	if (rtn!=0) {
    	printf("Delete error %d %s\n",rtn,pathname);
		return CP_FAIL;
	}
	else
		return CP_OK;

}

int CP_DeleteFolder(CP_del* pdata, int cpmode){
	int rtn = CP_OK;
	int rtn_dir = 0;
	DWORD path_entry_Num = pdata->ff->cwd_num_entries;
	DWORD entryfilesize = pdata->ff->entry_offset;
	rtn_dir = chdir(pdata->SrcPath);
	if (rtn_dir) return CP_FAIL;

	pdata->ff->search_path=pdata->SrcPath;

	if(!find_first(pdata->ff)){
		do{
			if(strcmp(pdata->ff->ff_name,".")==0)
					continue;
			if(strcmp(pdata->ff->ff_name,"..")==0)
					continue;

			if(pdata->ff->ff_attr&FA_DIR){
				//if((cpmode%CP_BYTE)<CP_MAX_DEPTH){
				if(cpmode&CP_RC){
					if(!isdelim((pdata->SrcPath[strlen(pdata->SrcPath)-1])))
						strcat(pdata->SrcPath,"/");
					strcat(pdata->SrcPath,pdata->ff->ff_name);
					if(CP_FAIL==CP_DeleteFolder(pdata, cpmode+1))
						rtn = CP_FAIL;
				}
			}
			else{
				if(CP_FAIL==CP_DeleteFile(pdata->ff->ff_name, cpmode))
					rtn = CP_FAIL;
				else{
					if(cpmode&CP_SHOWPROGRESS){
						if(!isdelim(pdata->SrcPath[strlen(pdata->SrcPath)-1]))
							printf("unlink %s/%s\n",pdata->SrcPath,pdata->ff->ff_name);
						else
							printf("unlink %s%s\n",pdata->SrcPath,pdata->ff->ff_name);
					}
				}
			}
		}while(!find_next(pdata->ff));
	};

	//clear all folder
	if(cpmode&CP_RC){
		if(!find_first(pdata->ff)){
			do{
				if(strcmp(pdata->ff->ff_name,".")==0)
						continue;
				if(strcmp(pdata->ff->ff_name,"..")==0)
						continue;
				if(pdata->ff->ff_attr&FA_DIR){
					if(rmdir(pdata->ff->ff_name)){
						rtn = CP_FAIL;
					}
					else{
						if(cpmode&CP_SHOWPROGRESS){
							if(!isdelim((pdata->SrcPath[strlen(pdata->SrcPath)-1])))
								printf("remove %s/%s\n",pdata->SrcPath,pdata->ff->ff_name);
							else
								printf("remove %s%s\n",pdata->SrcPath,pdata->ff->ff_name);
						}
					}
				}
			}while(!find_next(pdata->ff));
		}
	}
	//chagne directory to upper dir
	CP_SplitPathName(pdata->SrcPath,pdata->ff->ff_name);
	rtn_dir = chdir(pdata->SrcPath);
	if (rtn_dir) rtn_dir=CP_FAIL;
	pdata->ff->cwd_num_entries = path_entry_Num;
	pdata->ff->entry_offset = entryfilesize;

	return rtn;

}


int CP_Delete(char* pathname, char* patent, int cpmode){
	int filetype;
	ffblk* ff;
	char src_path[MAX_PATH_LENGTH]={0};
	char rec_cwd[MAX_PATH_LENGTH]={0};
	CP_del data;

	ff=find_init();
	if (!ff) return -1;

	if(patent)
		ff->rule_set = patent;
	data.ff= ff;
	data.SrcPath = src_path;
	getcwd(rec_cwd,MAX_PATH_LENGTH);
	filetype = CP_ISFOLDER(pathname);
	switch(filetype){
		case CP_OK://folder
			strcpy(data.SrcPath,pathname);
			CP_DeleteFolder(&data,cpmode);
			if(!patent){
				if(!rmdir(pathname)){
					if(cpmode&CP_SHOWPROGRESS){
						printf("remove %s",pathname);
					}
				}

			}
			break;

		case CP_FAIL://file
			if(CP_OK==CP_DeleteFile(pathname, 0)){
				if(cpmode&CP_SHOWPROGRESS){
					printf("unlink %s",pathname);
				}
			}
			break;

		default:
			goto out1;

	}
	chdir(rec_cwd);

out1:
	free(ff);

	return 0;
}

void CP_ShowDirItem(ffblk* pff, CP_DIR_INFO* data){

	int filetype =0;
//	struct stat st;
//	int st_rtn = -1;
	ucs2_t longname[MAX_PATH_LENGTH]={0};
//	st_rtn = stat(pathname,&st);
//	if(st_rtn < 0){
//		FCOPY_PRINTF("CP_ShowDirItem %s is not exist\n",pathname);
//		return ;
//	}
//	else{
//		_localtime(&st.st_mtime,&ntime);
//	}

	fx_big5_name_get(pff->pfx_media, pff->ff_name, (BYTE*)longname, MAX_PATH_LENGTH);

	if(pff->ff_attr&FA_DIR)
		filetype = CP_FOLDER;
	else if(!(pff->ff_attr&FA_LABEL))
		filetype = CP_FILE;
	else
		filetype = 0;

	switch(filetype){
		case CP_FOLDER:
			printf("%04d/%02d/%02d	",pff->ff_time.tm_year,pff->ff_time.tm_mon,pff->ff_time.tm_mday);
			printf("%02d:%02d    ",pff->ff_time.tm_hour,pff->ff_time.tm_min);
			printf("<DIR>          ");
			if(CP_SHOWSHORTNAME&data->mode)
				printf("%13s ",pff->ff_name);
			if(wcslen(longname)>0)
				printf("%s",longname);
			else
				printf("%s",pff->ff_name);//if no long name, show short name instead of
			data->foldercount++;
			break;
		case CP_FILE:
			printf("%04d/%02d/%02d	",pff->ff_time.tm_year,pff->ff_time.tm_mon,pff->ff_time.tm_mday);
			printf("%02d:%02d    ",pff->ff_time.tm_hour,pff->ff_time.tm_min);
			printf("%-14d ",pff->ff_size);
			if(CP_SHOWSHORTNAME&data->mode)
				printf("%13s ",pff->ff_name);
			if(wcslen(longname)>0)
				printf("%s",longname);
			else
				printf("%s",pff->ff_name);//if no long name, show short name instead of
			data->totalsize = data->totalsize+pff->ff_size;
			data->filecount++;
			break;
		default:
			break;
	}
	printf("\n");
}


void CP_ShowDir(char* pathname,char* patent, DWORD cpmode){
	if(!pathname)
		return;

	ffblk *ff;
//	char cwd[MAX_PATH_LENGTH]={0};
//	ucs2_t longname[MAX_PATH_LENGTH]={0};
	CP_DIR_INFO dirinfo;
	dirinfo.filecount = 0;
	dirinfo.foldercount = 0;
	dirinfo.mode = cpmode;
	dirinfo.totalsize = 0 ;

//	char* temp = NULL;
//	FX_MEDIA *pmdia;

	DWORD freespace = 0;

	ff=find_init();
	if (!ff) return;

	if(NULL!=patent) ff->rule_set = patent;
	ff->search_path=pathname;

//	getcwd(cwd,MAX_PATH_LENGTH);
//	chdir(pathname);

//	yl_dmParsePath(".",&temp,&pmdia);
//	if(temp) free(temp);

	if(!find_first(ff)){
		do{
			if ( (ff->ff_name[0]=='.') && ( (ff->ff_name[1]==0)||(ff->ff_name[1]=='.') ) ) continue;
			CP_ShowDirItem(ff, &dirinfo);
//			memset(longname,0,MAX_PATH_LENGTH*2);
		}while(!find_next(ff));
	}

	yl_dmGetMediaFreeSpace(ff->pfx_media, &freespace);

	printf("%-15d file%c%-15u byte%c\n",dirinfo.filecount,dirinfo.filecount>1?'s':' ',dirinfo.totalsize,dirinfo.totalsize>1?'s':' ');
	printf("%-15d  dir%c%-15u byte%c available\n",dirinfo.foldercount,dirinfo.foldercount>1?'s':' ',freespace,freespace>1?'s':' ');

	free(ff);
//	chdir(cwd);

}




//===================================================================================
//===================================================================================
//===================================================================================
//===================================================================================
//===================================================================================



#if 0
void ucon_copy(int argc, char** argv) {
	int  s;
	int mode=CP_SHOWPROGRESS;
	char patent[MAX_PATH_LENGTH]={0};
	char cwd[MAX_PATH_LENGTH]={0};
	fdata_t matched;
	memset(matched.entry_name,0,MAX_PATH_LENGTH);
	memset(matched.unicode_name,0,MAX_PATH_LENGTH*2);
	matched.cwd_num_entries = 0;
	matched.entry_offset = 0;
	matched.shortened = 0;
	fspec_t	search = {0, 0, TS_NONE, TS_NONE, "*.*"};
	CP_info cp_data;
	memset(cp_data.DestPath,0,MAX_PATH_LENGTH);
	memset(cp_data.SrcPath,0,MAX_PATH_LENGTH);
	cp_data.search = &search;
	cp_data.matched = &matched;
	int icheck = -1;
	int idestcheck = -1;
	int bfolder = -1;

	getopt(0,0,0); // reset getopt state
	while((s=getopt(argc,argv,"rfqh")) != EOF ){
		switch(s){
		case 'r':
			mode = mode | CP_RC;
			break;

		case 'f':
			mode = mode | CP_OW;
			break;
		case 'q':
			mode = mode&(~CP_SHOWPROGRESS);
			break;
		case 'h':
			printf(
				"Usage: %s [switches] <source> <destination>\n"
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
		if(CP_GetFullPathName(argv[optind],cp_data.SrcPath)){
			printf("invalid path %s\n",argv[optind]);
			return;
		}
		optind++;

		icheck = CP_ISFOLDER(cp_data.SrcPath);
		if(CP_FILE_NOT_EXIT==icheck){
			if(CP_OK==CP_SplitPathName(cp_data.SrcPath, patent)){
				int index=0;
				int length = strlen(patent);
				BOOL bcheck = FALSE;
				for(;index<length;index++){
					if(patent[index]=='*'){
						bcheck = TRUE;
						break;
					}
				}
				if(FALSE==bcheck){
					printf("invalid source path\n");
					return;
				}

			}
		}

	}
	else{
		printf("no source provided.\n");
		return;
	}

	char *dst_path;
	if((argc-optind)>0){
		dst_path=argv[optind];
	}
	else {
		dst_path=".";
	}

	if(CP_GetFullPathName(dst_path,cp_data.DestPath)){
		printf("invalid path %s\n",argv[optind]);
		return;
	}
	optind++;

	idestcheck = CP_OK;

	if(CP_OK==CP_ISFOLDER(cp_data.DestPath)){
		bfolder = TRUE;
	}
	else{
		bfolder = FALSE;
	}


	getcwd(cwd,MAX_PATH_LENGTH);

	switch(icheck){
		case CP_OK://folder
			//CP_DestCheck will create folder, and store new folder short name in entry_name
			if(CP_OK==CP_DestCheck(&cp_data, CP_FOLDER)){
				if(cp_data.DestPath[strlen(cp_data.DestPath)-1]!='/')
					strcat(cp_data.DestPath,"/");
				strcat(cp_data.DestPath,cp_data.matched->entry_name);
			}

			if(CP_OK==yl_FolderCopy(&cp_data, mode))
				FCOPY_PRINTF("copy a folder from %s to %s ok\n",cp_data.SrcPath,cp_data.DestPath);
			else
				FCOPY_PRINTF("copy a folder from %s to %s fail\n",cp_data.SrcPath,cp_data.DestPath);


			break;

		case CP_FAIL://file
			if(bfolder==TRUE){
				mode = mode | CP_FOLDER;
				if(CP_GetLongName(cp_data.SrcPath,cp_data.matched->unicode_name)){
					cp_data.matched->shortened = 1;
				}
				else
					CP_SplitPathName(cp_data.SrcPath,cp_data.matched->entry_name);
				if(strlen(cp_data.SrcPath)<=1)
					getcwd(cp_data.SrcPath,sizeof(cp_data.SrcPath));
			}
			else{
				mode = mode | CP_FILE;
				cp_data.matched->shortened = 0;
				CP_SplitPathName(cp_data.DestPath,cp_data.matched->entry_name);
			}

			if(CP_OK==yl_FileSingleCopy(&cp_data, mode))
				FCOPY_PRINTF("copy a file from %s to %s ok\n",cp_data.SrcPath,cp_data.DestPath);
			else
				FCOPY_PRINTF("copy a file from %s to %s fail\n",cp_data.SrcPath,cp_data.DestPath);
			break;
		case CP_FILE_NOT_EXIT://folder+search patent
			if(strlen(patent)>0)
				cp_data.search->dname_ptn = patent;
			if(strlen(cp_data.SrcPath)<=1)
				getcwd(cp_data.SrcPath,sizeof(cp_data.SrcPath));
			if(CP_OK==yl_FolderCopy(&cp_data, mode))
				FCOPY_PRINTF("copy a folder from %s to %s ok\n",cp_data.SrcPath,cp_data.DestPath);
			else
				FCOPY_PRINTF("copy a folder from %s to %s fail\n",cp_data.SrcPath,cp_data.DestPath);
			break;
		default:
			break;
	}

	chdir(cwd);
}
#endif

void ucondir(int argc, char** argv){
	int s;
	DWORD mode = 0;
//	char strpatent[64]={0};
	char pathname [MAX_PATH_LENGTH]={0};
	char** sp;
	getopt(0,0,0); // reset getopt state
	while((s=getopt(argc,argv,"xh")) != EOF ){
		switch(s){
		case 'x':
			mode = mode | CP_SHOWSHORTNAME;
			break;
		case 'h':
			printf(
			"    -x      display short name\n"
			);
			return;
		default:
			break;
		}
	}


	if((argc-optind)>0){
		if(CP_GetFullPathName(argv[optind],pathname)){
			printf("invalid path!!\n");
			return;
		}
	}
	else{
		getcwd(pathname,MAX_PATH_LENGTH);
	}

	int i = 0;
	int bvalid = FALSE;
	for(;i<strlen(pathname);i++){
		if(pathname[i]=='*'){
			bvalid = TRUE;
			break;
		}
	}
	if(bvalid){
//		CP_SplitPathName(pathname,strpatent);
//		CP_ShowDir(pathname,strpatent, mode);
		char pathname_sp[SP_MAX_PATH];
		splitpath(pathname, pathname_sp);
		specialcombine(pathname_sp);
		sp=(char**)pathname_sp;
		CP_ShowDir(SP_DRV(sp),SP_NAME(sp), mode);
	}
	else
		CP_ShowDir(pathname,"*.*",mode);

}



void ucondel(int argc, char** argv){
	int s;
	DWORD mode = CP_SHOWPROGRESS;
//	char strpatent[MAX_PATH_LENGTH]={0};
	char* strtemp = NULL;
	void* pmedia =NULL;
	char pathname[MAX_PATH_LENGTH]={0};
	char **sp;
	getopt(0,0,0); // reset getopt state
	while((s=getopt(argc,argv,"rqh")) != EOF ){
		switch(s){
		case 'r':
			mode = mode | CP_RC;// recursive
			break;
		case 'q':
			mode = mode&(~CP_SHOWPROGRESS);
			break;
		case 'h':
			printf(
			"Usage: %s [switches] <file>\n"
			"    -r      remove directories and their contents recursively\n"
			"    -q      quiet mode\n"
			"\n"
			"Example:\n"
			"    del -r a:/*.*\n"
			,argv[0]);
			return;
		default:
			break;
		}
	}

	if((argc-optind)>0){
		int driveIndex;

		driveIndex=yl_dmParsePath(argv[optind], &strtemp, &pmedia);
		if(driveIndex>=0){
			optind++;
			FCOPY_PRINTF("pathname:%s\n",strtemp);

			if((driveIndex>=CP_MINIMUM_DRIVE)&&(driveIndex<=CP_MAXIMUN_DRIVE)){
				if(strtemp) {
					sprintf(pathname,"%c:%s",'a'+driveIndex,strtemp);
					free(strtemp);
				}
				else
					sprintf(pathname,"%c:",'a'+driveIndex);
			}
			else{
				printf("wrong disc drive!!\n");
				goto CP_ucondelEXIT;
			}
		}
		else{
			printf("invalid path!!\n");
			goto CP_ucondelEXIT;
		}
	}
	else {
		printf("no file provided.\n");
		goto CP_ucondelEXIT;
	}

	int i = 0;
	int bvalid = FALSE;
	for(;i<strlen(pathname);i++){
		if(pathname[i]=='*'){
			bvalid = TRUE;
			break;
		}
	}

	if(bvalid){
//		CP_SplitPathName(pathname,strpatent);
//		CP_Delete(pathname, strpatent, mode);
		char pathname_sp[SP_MAX_PATH];
		splitpath(pathname, pathname_sp);
		specialcombine(pathname_sp);
		sp=(char**)pathname_sp;
		CP_Delete(SP_DRV(sp),SP_NAME(sp), mode);
	}
	else{
		CP_Delete(pathname, NULL, mode);
	}

CP_ucondelEXIT:

	return;

}
