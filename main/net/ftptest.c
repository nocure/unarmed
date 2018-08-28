/*
 * ftptest.c
 *
 *  Created on: 2009/10/2
 *      Author: william
 */
#include <string.h>
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/ip.h"

#include "lwip/sockets.h"

#include <lwip/init.h>
#include <lwip/tcpip.h>
#include <netif/loopif.h>
#include "net/ftp_client_n/ftpclient.h"
#include <platform/YL_FSIO.h>
#include "net/ftp_server/ftpd.h"
#include <lwip/sockets.h>



static err_t ftpc_connected(void *arg, struct tcp_pcb *pcb, err_t err);

void ftp_init(void) {
	struct tcp_pcb *pcb_c;
	struct ip_addr dest;

	/*initial the ftp server*/
	ftpd_init();

	/*Client connect to the server*/
	IP4_ADDR(&dest, 127, 0, 0, 1);
	pcb_c = tcp_new();
	tcp_arg(pcb_c, (void *) 0);
	tcp_connect(pcb_c, &dest, 21, ftpc_connected);



#if 0
	//http://lazyflai.blogspot.com/2009/01/c.html
	/*list the file*/
	DIR *dirp;
	struct dirent *dp;
	char *a="a:/";
	dirp = opendir(a);
	while ((dp = readdir(dirp)) != NULL) {
		printf("file:%s\n", dp->d_name);
	}
	closedir(dirp);
	printf("done!");



	//http://www.csie.ntu.edu.tw/~cprog2003/downloads/Notes%20on%20C%20File%20I-O.htm
	/*read & write the file*/
	FILE * pFile;
	long lSize;
	char * buffer;
	pFile = fopen ("test1.txt" , "rb");
	fseek(pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	printf("size:%d",lSize);
	fclose (pFile);
	free (buffer);
#endif
}

static err_t ftpc_connected(void *arg, struct tcp_pcb *pcb, err_t err) {
	printf("ok!");
	return 0;
}



