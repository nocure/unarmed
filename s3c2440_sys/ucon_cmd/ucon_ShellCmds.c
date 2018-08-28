#include "platform/YL_Sys.h"
#include <platform/YL_FSIO.h>
#include "platform/yl_errno.h"


void uconMD(int argc, char** argv) {
int status = 0;
	if (argc>=2) {
    	status = mkdir(argv[1]);
    	if(status != 0) printf("mkdir Failed\n");
    }
	return;
}


void uconCD(int argc, char** argv) {
int status = 0;
    if (argc>=2) {
    	status = chdir(argv[1]);
    	if(status != 0) printf("No such directory\n");
    }
	return;
}


void uconRD(int argc, char** argv) {
	int status = 0;
    if (argc>=2) {
    	status = rmdir(argv[1]);
    	if(status != 0) printf("No such directory\n");
    }
	return;
}


void uconPwd(int argc, char** argv) {
char test_temp[64];
	getcwd(test_temp,sizeof(test_temp));
	printf(test_temp);
	return;
}


void ucontype(int argc, char** argv) {
char buf[65];
FILE *f;
int sr;

	if (argc<2) {
		printf("%s <file>",argv[0]);
		return;
	}

	f=fopen(argv[1],"r");
	if (!f) {
		printf("file not found\n");
		return;
	}
	while(1) {

		sr=fread(buf,64,f);
		if (sr==-1) break;	// EOF
		else if (sr<0) { // Error
			printf("read error:%d",sr);
			break;
		}
		buf[sr]=0;
		printf(buf);
	}
	fclose(f);
	printf("\n");

	return;
}


void uconfwrite(int argc, char** argv) {
DWORD addr;
DWORD len;
char *ep;
FILE *fp1=0;
    if (argc<4) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <length>\n",argv[0]);
		printf("\t *** This test will modify the content of specified memory\n");
		printf("\t *** Use with caution !!!\n\n");
		return;
	}
    addr=strtoul(argv[2],&ep,0);
    if(*ep){
    	printf("Input address error: %s\n\n",argv[1]);
    	return;
    }
    len=strtoul(argv[3],&ep,0);
    if(*ep) {
    	printf("Input length error: %s\n\n",argv[2]);
    	return;
    }
   	fp1=fopen(argv[1],"w+");
	if(!fp1) {
		printf("open test file error\n");
	}
	fwrite((void*)addr,len,fp1);
	fclose(fp1);
	printf("fwrite ok \n");

	return;

}


void uconfread(int argc, char** argv)
{
DWORD addr;
char *ep;
FILE *fp1=0;
struct	stat s;
int status = 0;
    if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <mem addr> <length>\n",argv[0]);
		printf("\t *** This test will modify the content of specified memory\n");
		printf("\t *** Use with caution !!!\n\n");
		return;
	}
    addr=strtoul(argv[2],&ep,0);
    if(*ep){
    	printf("Input address error: %s\n\n",argv[1]);
    	return;
    }
    status = stat(argv[1], &s);
    if(status != 0){
    	printf("not ok status is %d\n",status);
    	return;
    }
    else{
        printf("ok status is %d\n",status);
    }
    fp1=fopen(argv[1], "r");
	if(!fp1) {
		printf("open test file error\n");
	}
	fseek(fp1, 0, SEEK_SET);
	fread((void*)addr,s.st_size,fp1);
	fclose(fp1);
	printf("read ok\n");

	return;
}


void uconren(int argc, char** argv)
{
    if (argc<3) {
		printf("Usage:\n");
		printf("\t %s <original name> <new name>\n",argv[0]);
		return;
	}

    if(rename(argv[1],argv[2]) != 0){
    	printf("Error occurs.\n");
	}
	return;
}



void uconCheck(int argc, char** argv) {
int ret;
DWORD correct_option,error_detected;
char *drv;
void *fxm;
int idx;

	correct_option=0;
	getopt(0,0,0); // reset getopt state
	while ((ret=getopt(argc,argv,"f")) != EOF) {
		switch (ret) {
			case 'f':
				correct_option|=(FAT_CHAIN_ERROR|DIRECTORY_ERROR|LOST_CLUSTER_ERROR);
				break;
			default:
				printf(	"Usage: %s [switches] <drive letter:>\n"
						"    -f      fix errors\n"
						,argv[0]);
				return;
		}
	}

	if ((argc-optind)>0) {
		drv=argv[optind];
	}
	else {
		printf("Drive letter required.\n");
		return;
	}

	idx=yl_dmParsePath(drv,NULL,&fxm);
	if (idx<0) {
		printf("Invalid drive %s\n",drv);
		return;
	}

	ret=yl_dmUnmount(idx);
	if (ret) {
		printf("Unable to unmount. %d\n",ret);
		return;
	}

	ret=yl_dmMount(idx);
	if (ret) {
		printf("Unable to re-mount. %d\n",ret);
		return;
	}

	error_detected=0;
	ret=yl_dmMediaCheckRepair(fxm, correct_option, &error_detected);
	if (error_detected&FAT_CHAIN_ERROR) {
		printf("FAT Chain error detected.\n");
	}
	if (error_detected&DIRECTORY_ERROR) {
		printf("Directory structure error detected.\n");
	}
	if (error_detected&LOST_CLUSTER_ERROR) {
		printf("Lost clusters found.\n");
	}
	if (error_detected&FILE_SIZE_ERROR) {
		printf("File size error.\n");
	}

	if (!ret) {
		if (!error_detected) {
			printf("No error found.\n");
		}
		else if (error_detected&correct_option) {
			printf("Successfully repaired.\n");
		}
	}
	else if (ret==YLERR_FAT) {
		printf("Unable to fix errors.\n");
	}
	else {
		printf("file system check failed. %d\n",ret);
	}

}


// mount & unmount
void uconMount(int argc, char** argv) {
int ret;
char *drv;
int idx;

	if (argc<2) drv=".";
	else drv=argv[1];

	idx=yl_dmParsePath(drv,NULL,0);
	if (idx<0) {
		printf("Invalid drive %s\n",drv);
		return;
	}

	if (*argv[0]=='m') {
		ret=yl_dmMount(idx);
		if (ret) {
			printf("Unable to mount. %d\n",ret);
			return;
		}
	}
	else {
		ret=yl_dmUnmount(idx);
		if (ret) {
			printf("Unable to Unmount. %d\n",ret);
			return;
		}
	}
}


// mount & unmount
void uconNFTLInfo(int argc, char** argv) {
int ret;
char *drv;
int idx;

	if (argc<2) drv=".";
	else drv=argv[1];

	idx=yl_dmParsePath(drv,NULL,0);
	if (idx<0) {
		printf("Invalid drive %s\n",drv);
		return;
	}

	ret=yl_dmIOCtrl(idx,IOCTRL_NFTL_DISPINFO,0);
	if (ret) printf("Not an NFTL drive\n");
}


void uconFormat(int argc, char** argv) {
int ret;
char *drv;
int idx;
int lv,cmem;

	cmem=lv=0;
	getopt(0,0,0); // reset getopt state
	while ((ret=getopt(argc,argv,"lrc")) != EOF) {
		switch (ret) {
			case 'l':
				if (!lv) lv=1;
				break;

			case 'r':
				if (!lv) lv=2;
				break;

			case'c':
				cmem=1;
				break;

			default:
				goto usage;
		}
	}

	if ((argc-optind)>0) {
		drv=argv[optind];
	}
	else {
		goto usage;
		printf("Drive letter required.\n");
		return;
	}

	idx=yl_dmParsePath(drv,NULL,0);
	if (idx<0) {
		printf("Invalid drive %s\n",drv);
		return;
	}

	if (yl_dmIsMounted(idx)) {
		printf("Drive is mounted, Unmount first.\n");
		return;
	}

	if (cmem) {
		ret=yl_dmIOCtrl(idx,IOCTRL_RAMDRV_RESET_MEMORY,0);
		if (ret) goto err;
	}

	if (lv==2) { // RAW format
		ret=yl_dmIOCtrl(idx,IOCTRL_NFTL_RAW_FORMAT,0);
		if (ret) goto err;
		ret=yl_dmIOCtrl(idx,IOCTRL_NFTL_INITIALIZE,0);
	}
	else if (lv==1) { // low-level format
		ret=yl_dmIOCtrl(idx,IOCTRL_NFTL_LOW_LEVEL_FORMAT,0);
		if (ret) goto err;
		ret=yl_dmIOCtrl(idx,IOCTRL_NFTL_INITIALIZE,0);
	}
	else ret=0;
	if (ret) goto err;
	ret=yl_dmFormat(idx,0);

err:
	if (ret) {
		printf("fs format error code: %d\n",ret);
	}
	return;


usage:
	printf(	"Usage: %s [switches] <drive letter:>\n"
			"    -l      low-level format\n"
			"    -r      raw format, regardless\n"
			"    -c      reset memory, for memory based drive only\n"
			,argv[0]);
	return;


}



#if 0
void uconPD(int argc, char** argv) {
	printf("powering down...\n");
	__asm__ __volatile__ ("wr %g0, %asr19");	// power down test
	__asm__ __volatile__ ("wr %g0, %asr19");	// power down test
	__asm__ __volatile__ ("wr %g0, %asr19");	// power down test
	__asm__ __volatile__ ("wr %g0, %asr19");	// power down test
	printf("back to live\n");
}



void ucondir(int argc, char** argv){
char *path,*tempp,pathbuf[80];
DIR	 *dp;
struct dirent *dent;
int status;
struct stat s;
struct tm tt;
int dircnt = 0, filecnt = 0, filesize = 0;
ULONG freespace;
FX_MEDIA *mp;
UCHAR	big5name[128];

	if (argc>=2) path=argv[1];
	else path=".";

  	yl_dmParsePath(path,&tempp,&mp);

	dp = opendir(path);
	if(NULL == dp){
		printf("Invalid path %s\n",path);
		return;
	}

  	while ((dent = readdir(dp)) != NULL){
  		// excliudes "." & ".."
  		if ( (dent->d_name[0]=='.') && (dent->d_name[1]==0) ) continue; // .
  		if ( (dent->d_name[0]=='.') && (dent->d_name[1]=='.') ) continue; // ..

		strcpy(pathbuf,path);
  		strcat(pathbuf,"/");
  		strcat(pathbuf,dent->d_name);
		status = stat(pathbuf, &s);
		if (status!=0) continue;
		localtime(&s.st_mtime,&tt);

		if(S_ISDIR(s.st_mode)){    //Directory
	   		printf("%04d/%02d/%02d  ",tt.tm_year+1900,tt.tm_mon,tt.tm_mday);
	   		printf("%02d:%02d         ",tt.tm_hour,tt.tm_min);

	   		printf("<DIR>    ");

	   		status = fx_big5_name_get(mp, dent->d_name, big5name, 128);
	   		if (status != FX_SUCCESS)
	   		printf("%s",dent->d_name);
	   		else
	   			printf("%s",big5name);
			dircnt++;
	   	}
		else if(S_ISREG(s.st_mode)){    // file
	   		printf("%04d/%02d/%02d  ",tt.tm_year+1900,tt.tm_mon,tt.tm_mday);
	   		printf("%02d:%02d  ",tt.tm_hour,tt.tm_min);

	   		printf("%-14d  ",s.st_size);

	   		status = fx_big5_name_get(mp, dent->d_name, big5name, 128);
	   		if (status != FX_SUCCESS)
	   		printf("%s",dent->d_name);
	   		else
	   			printf("%s",big5name);
			filesize = s.st_size + filesize;
			filecnt++;
		}
   		printf("\n");

	}
	closedir(dp);

  	free(tempp);
	yl_dmGetMediaFreeSpace(mp, &freespace);
	printf("%-15d file%c%-15d byte%c\n",filecnt,filecnt>1?'s':' ',filesize,filesize>1?'s':' ');
	printf("%-15d  dir%c%-15d byte%c available\n",dircnt,dircnt>1?'s':' ',freespace,freespace>1?'s':' ');

	return;
}


#endif


