/*
 * sefsio.c
 *
 *  Created on: 2015/10/29
 *      Author: user
 *
 * Real World file I/O, used with skyeye simulator ww special version 0.5
 * Read/Write data in simulator's memory from/to real file.
 *
 */
#include "platform/yl_type.h"
#include "platform/sefsio_api.h"
#include "sefsio_priv.h"


SE_FILE* se_fopen(const char *file, const char *mode) {
	rSEFSIOP1=(DWORD)file;
	rSEFSIOP2=(DWORD)mode;
	rSEFSIOFUNC=FUNC_fopen;
	return (SE_FILE*)rSEFSIOFUNC;
}


int se_fclose(SE_FILE * __f) {
	rSEFSIOP1=(DWORD)__f;
	rSEFSIOFUNC=FUNC_fclose;
	return (int)rSEFSIOFUNC;
}

int se_fseek(SE_FILE * __f, off_t __o, int __w) {
	rSEFSIOP1=(DWORD)__f;
	rSEFSIOP2=(DWORD)__o;
	rSEFSIOP3=(DWORD)__w;
	rSEFSIOFUNC=FUNC_fseek;
	return (int)rSEFSIOFUNC;
}


off_t se_ftell(SE_FILE * __f) {
	rSEFSIOP1=(DWORD)__f;
	rSEFSIOFUNC=FUNC_ftell;
	return (off_t)rSEFSIOFUNC;
}

#if 0
struct real_stat
{
	int		st_dev;		/* Equivalent to drive number 0=A 1=B ... */
	int		st_ino;		/* Always zero ? */
	mode_t	st_mode;	/* See above constants */
	short	st_nlink;	/* Number of links. */
	short	st_uid;		/* User: Maybe significant on NT ? */
	short	st_gid;		/* Group: Ditto */
	int		st_rdev;	/* Seems useless (not even filled in) */
	off_t	st_size;	/* File size in bytes */
	time_t	st_atime;	/* Accessed date (always 00:00 hrs local
				 * on FAT) */
	time_t	st_mtime;	/* Modified time */
	time_t	st_ctime;	/* Creation time */
};
int se_stat(const char *path, struct stat *buf) {
struct real_stat rs;
	rSEFSIOP1=(DWORD)path;
	rSEFSIOP2=(DWORD)&rs;
	rSEFSIOFUNC=FUNC_stat;
	buf->st_mode=rs.st_mode;
	buf->st_size=rs.st_size;
	buf->st_atime=rs.st_atime;
	buf->st_mtime=rs.st_mtime;
	buf->st_ctime=rs.st_ctime;
	return (int)rSEFSIOFUNC;
}
#endif

int se_fputs(const char *str, SE_FILE *fp) {
	rSEFSIOP1=(DWORD)str;
	rSEFSIOP2=(DWORD)fp;
	rSEFSIOFUNC=FUNC_fputs;
	return (int)rSEFSIOFUNC;
}


int se_feof(SE_FILE *__f) {
	rSEFSIOP1=(DWORD)__f;
	rSEFSIOFUNC=FUNC_feof;
	return (int)rSEFSIOFUNC;
}


int se_fputc(int ch, SE_FILE *fp) {
	rSEFSIOP1=(DWORD)ch;
	rSEFSIOP2=(DWORD)fp;
	rSEFSIOFUNC=FUNC_fputc;
	return (int)rSEFSIOFUNC;
}


int se_fgetc(SE_FILE *fp) {
	rSEFSIOP1=(DWORD)fp;
	rSEFSIOFUNC=FUNC_fgetc;
	return (int)rSEFSIOFUNC;
}


char *se_fgets(char *str, int size, SE_FILE *fp) {
	rSEFSIOP1=(DWORD)str;
	rSEFSIOP2=(DWORD)size;
	rSEFSIOP3=(DWORD)size;
	rSEFSIOFUNC=FUNC_fgets;
	return (char*)rSEFSIOFUNC;
}


size_t se_fread(void *buf, size_t size, SE_FILE *__f) {
	rSEFSIOP1=(DWORD)buf;
	rSEFSIOP2=(DWORD)size;
	rSEFSIOP3=(DWORD)__f;
	rSEFSIOFUNC=FUNC_fread;
	return (size_t)rSEFSIOFUNC;
}


size_t se_fwrite(const void *buf, size_t size, SE_FILE *__f) {
	rSEFSIOP1=(DWORD)buf;
	rSEFSIOP2=(DWORD)size;
	rSEFSIOP3=(DWORD)__f;
	rSEFSIOFUNC=FUNC_fwrite;
	return (size_t)rSEFSIOFUNC;
}


int se_unlink(const char * file_name) {
	rSEFSIOP1=(DWORD)file_name;
	rSEFSIOFUNC=FUNC_unlink;
	return (size_t)rSEFSIOFUNC;
}


int se_fflush(SE_FILE *__f) {
	rSEFSIOP1=(DWORD)__f;
	rSEFSIOFUNC=FUNC_fflush;
	return (size_t)rSEFSIOFUNC;
}


SE_FILE* se_tmpfile(void) {
	rSEFSIOFUNC=FUNC_tmpfile;
	return (SE_FILE*)rSEFSIOFUNC;
}


int se_ferror(SE_FILE* stream) {
	rSEFSIOP1=(DWORD)stream;
	rSEFSIOFUNC=FUNC_ferror;
	return (int)rSEFSIOFUNC;
}
