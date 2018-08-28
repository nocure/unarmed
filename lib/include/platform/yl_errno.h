/*
 * YL_Errno.h
 *
 *  Created on: 2008/12/12
 *      Author: Wesley Wang
 */

#ifndef yl_errno_H_
#define yl_errno_H_

#define SUCCESS					0
#define FAILED					-1

#define YLERR_OK				0
#define YLERR_FAILURE			-1		// failed, and guess the reason !!
#define YLERR_ARG				-1000	// argument error
#define YLERR_FS				-1001	// file system error, possible un-formated media
#define YLERR_FAT				-1002	// corrupt FAT
#define YLERR_IO				-1003	// driver error
#define YLERR_DRIVE				-1004	// drive invalid
//#define YLERR_CALLBACK			-1005	// callback return failed
#define YLERR_NOTSUPP			-1005	///< IO controller is not supported

// ccchiu
#define YLERR_NOENT				-1006	// No such file or directory
#define YLERR_BADF				-1007	// Bad File Stream
#define YLERR_NMENT				-1008	// No more dir entry
#define YLERR_NOMEM				-1009	// Not enough memory
#define YLERR_EXIST				-1010	// File exists
#define YLERR_NOTDIR			-1011	// Not a directory
#define YLERR_ISDIR				-1012	// Is a directory
#define YLERR_ENOTEMPTY			-1013 	// Directory not empty
#define YLERR_E2BIG				-1014	// Arg list too long
#define YLERR_INVAL				-1015	// Invalid argument
#define YLERR_BUSY				-1016	// Mount device busy
#define YLERR_MFILE				-1017	// Too many open files
#define YLERR_FBIG				-1018	// File too large
#define YLERR_NOSPC				-1019	// No space left on media
#define YLERR_NAMETOOLONG		-1020	// File or path name too long
#define	YLERR_ACCES				-1021	// Access denied
#define YLERR_WP				-1022	// write protect
#define YLERR_INVLPATH			-1023	// invalid path
#define YLERR_TIMEOUT			-1024	// TIMEOUT
#define YLERR_NMFILE			-1025	///< no more files

#define ERR_SDC_FAILED			-1100
#define ERR_SDC_SEND			-1101
#define ERR_SDC_TIMEOUT			-1102
#define ERR_SDC_CMD_MISMATCH	-1103
#define ERR_SDC_NO_RSP			-1104
#define ERR_SDC_CRC				-1105
#define ERR_SDC_IF_COND			-1106
#define ERR_SDC_R1_RSP			-1107
#define ERR_SDC_WRONG_RSP		-1108
#define ERR_SDC_FIFO			-1109

#endif /* yl_errno_H_ */
