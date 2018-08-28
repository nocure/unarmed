/**
 * DAISY TECHNOLOGY FTP CLIENT
 *
 * Uses passive mode (control and data connection are locally initiated)
 * Based on the eCos ftp client
 *
 * Author: Iordan Neshev
 * Date:   28-03-2008
 */

/*
//==========================================================================
//
//      ftpclient.c
//
//      A simple FTP client
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Andrew Lunn.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    andrew.lunn@ascom.ch
// Contributors: andrew.lunn@ascom.ch
// Date:         2001-11-4
// Purpose:
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================
*/

/*
 * Copyright (c) 2001, 2002 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

#define CALLBACK_SOCKETS_ENABLED 1

#if CALLBACK_SOCKETS_ENABLED


#define FTP_SRV_IP				"127.0.0.1"
#define FTP_SERVER_CTRL_PORT	21		// remote port for control connection

#define FTP_USER				"user1"
#define FTP_PASS				"pass"

/*--------------*/

#define FTPC_SAVE_FILES_FROM_RECV_CALLBACK 0

//#define FTPC_GETLINE_TIMEOUT_ENABLE			// enable to protect from long input messages
#define FTPC_GETLINE_TIMEOUT		10000	// timeout for messages in ms

#define FTPC_WAIT_SRV_CLOSE_TIMEOUT 10000	// to do:end of received files is signalled with remote close. If not received - use this timeout


// enable/disable debug output for FTP Client application
#define FTPC_DEBUG			printf
//#define FTPC_DEBUG			(void)0;
#define FTPC_ERROR			printf
//#define FTPC_ERROR			(void)0;

#define FTPC_MSG_BUFF 60	// answer to PASV usually is the longest string

#define DEST_RAM		1	// where to store received file
#define DEST_FLASH		2



extern u16_t last_file_size;
extern volatile u32_t TimeoutFTPC;


extern int StoreMBFlash(unsigned long addr, char *src, unsigned int len);

extern int ftp_open_session(struct sockaddr_in* ptr_local_ipp,
							struct sockaddr_in *ptr_remote_ipp,
							char * username, char * passwd, int *ctrl_sockfd, char *msgbuf, unsigned msgbuflen);

extern int opendatasock(int ctrl_sfd,
						struct sockaddr_in* ptr_local_ipp_data,
						struct sockaddr_in* ptr_remote_ipp_data,
						char *msgbuf, u16_t msgbuflen);
extern int receive_file_RAM(int data_sockfd, char *buf, int buf_size);

extern int receive_file_MBF(int data_sockfd, char* flash_addr);	// max size: 32 k
extern int send_file(int data_sockfd, char *buf, int buf_size);
extern int ftp_quit	(int sockfd, char *msgbuf, u16_t msgbuflen);
extern int ftp_get	(char * hostname, char * username, char * passwd,
					 char * filename, char * buf, u16_t buf_size, u8_t where);
extern int ftp_put	(char * hostname, char * username, char * passwd,
					 char * filename, char * buf, u16_t len);

extern int ftp_open_wr(const char* filename, int ctrl_s, int *data_s, char *msgbuf, u16_t buflen, struct sockaddr_in *ptr_local_ipp_data,  struct sockaddr_in *ptr_remote_ipp_data);
extern int ftp_append(int data_s, char *buf, int len);
extern int ftp_closefile(int ctrl_s, int data_s);

/* Error codes from eCos*/
// to do: harmonize with lwip's error codes

#define FTP_BAD				(-2) /* Catch all, socket errors etc. */
#define FTP_NOSUCHHOST		(-3) /* The server does not exist. */
#define FTP_BADUSER			(-4) /* Username/Password failed */
#define FTP_TOOBIG			(-5) /* Out of buffer space or disk space */
#define FTP_BADFILENAME		(-6) /* The file does not exist */
#define FTP_BAD_ARGUMENT	(-7)
#define FTP_TIMEDOUT		(-8)



#endif /* CALLBACK_SOCKETS_ENABLED */
#endif /* _FTP_CLIENT_H_ */
