//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <platform/yl_sys.h>
#include <lwip/tcpip.h>
#include <netif/loopif.h>

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/ip.h"
//#include "httpd.h"

static err_t client_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t client_connected(void *arg, struct tcp_pcb *pcb, err_t err);

void my_client_init(void)
{
   struct tcp_pcb *pcb;
   struct ip_addr dest;
   err_t ret_val;
   IP4_ADDR(&dest, 127, 0, 0, 1);

   pcb = tcp_new();
//   tcp_bind(pcb, IP_ADDR_ANY, 7000); //client port for outcoming connection
   tcp_arg(pcb, (void *)0);
   ret_val = tcp_connect(pcb, &dest, 8000, client_connected); //server port for incoming connection
   if (ret_val != ERR_OK) {
	   printf("\tcp_connect(): Errors on return value, returned value is %d\n", ret_val);
   }
}

static void client_close(struct tcp_pcb *pcb)
{
   tcp_arg(pcb, NULL);
   tcp_sent(pcb, NULL);
   tcp_close(pcb);

   printf("\nclient_close(): Closing...\n");
}

static err_t client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
   char *string = "Hello!";
   LWIP_UNUSED_ARG(arg);

   if (err != ERR_OK)
	   printf("\nclient_connected(): err argument not set to ERR_OK, but is value is %d\n", err);

   else
   {
       tcp_sent(pcb, client_sent);
       tcp_write(pcb, string, strlen(string)+1, 0);
   }

   return err;
}

static err_t client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
   LWIP_UNUSED_ARG(arg);

   printf("\nclient_sent(): Number of bytes ACK'ed is %d", len);

   client_close(pcb);

   return ERR_OK;
}
