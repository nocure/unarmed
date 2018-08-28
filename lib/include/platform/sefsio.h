/*
 * sefsio.h
 *
 *  Created on: 2015/10/30
 *      Author: user
 */

#ifndef SEFSIO_H_
#define SEFSIO_H_

#include "sefsio_api.h"
#define FILE		SE_FILE
#define fopen		se_fopen
#define fclose		se_fclose
#define fseek		se_fseek
#define ftell		se_ftell
#define fputs		se_fputs
#define feof		se_feof
#define fputc		se_fputc
#define fgetc		se_fgetc
#define fgets		se_fgets
#define fread		se_fread
#define fwrite		se_fwrite
#define rename		se_rename
#define unlink		se_unlink
#define fflush		se_fflush
#define tmpfile		se_tmpfile
#define ferror		se_ferror

#endif /* SEFSIO_H_ */
