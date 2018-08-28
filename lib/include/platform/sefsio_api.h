/*
 * sefsio_api.h
 *
 *  Created on: 2015/10/29
 *      Author: user
 *
 * Real World file I/O, used with skyeye simulator ww special version 0.5
 * Read/Write data in simulator's memory from/to real file.
 *
 */

#ifndef SEFSIO_API_H_
#define SEFSIO_API_H_

typedef void* SE_FILE;

#define SEEK_SET	0
#define SEEK_CUR 	1
#define SEEK_END	2

extern SE_FILE *se_fopen(const char *, const char *);
extern int se_fclose(SE_FILE * __f);
extern int se_fseek(SE_FILE * __f, off_t __o, int __w);
extern off_t se_ftell(SE_FILE * __f);
extern int se_fputs(const char *, SE_FILE *);
extern int se_feof(SE_FILE *fp);
extern int se_fputc(int, SE_FILE *);
extern int se_fgetc(SE_FILE *);
extern char *se_fgets(char *, int, SE_FILE *);
extern size_t se_fread(void *, size_t, SE_FILE *);
extern size_t se_fwrite(const void *, size_t, SE_FILE *);
extern int se_rename(const char *, const char *);
extern int se_unlink(const char *);
extern int se_fflush(SE_FILE *);
extern SE_FILE* se_tmpfile(void);
extern int se_ferror(SE_FILE* stream);

#endif /* SEFSIO_API_H_ */
