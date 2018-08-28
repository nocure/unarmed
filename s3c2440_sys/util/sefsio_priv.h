/*
 * sefsio.h
 *
 *  Created on: 2015/10/29
 *      Author: user
 */

#ifndef SEFSIO_H_
#define SEFSIO_H_

#define SEFSIO_REG_BASE	0x67ffffe0
#define rSEFSIOFUNC	(*(volatile unsigned *)(SEFSIO_REG_BASE+0))
#define rSEFSIOP1	(*(unsigned *)(SEFSIO_REG_BASE+0x4))
#define rSEFSIOP2	(*(unsigned *)(SEFSIO_REG_BASE+0x8))
#define rSEFSIOP3	(*(unsigned *)(SEFSIO_REG_BASE+0xc))

#define FUNC_fopen		0
#define FUNC_fclose		1
#define FUNC_fseek		2
#define FUNC_ftell		3
//#define FUNC_stat		4
//#define FUNC_truncate	5
#define FUNC_fputs		6
#define FUNC_feof		7
#define FUNC_fputc		8
#define FUNC_fgetc		9
#define FUNC_fgets		10
#define FUNC_fread		11
#define FUNC_fwrite		12
#define FUNC_rename		13
#define FUNC_unlink		14
#define FUNC_fflush		15
#define FUNC_tmpfile	16
#define FUNC_ferror		17

#endif /* SEFSIO_H_ */
