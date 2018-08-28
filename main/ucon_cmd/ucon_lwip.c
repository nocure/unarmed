#include <string.h>
#include <lwip/init.h>
#include <lwip/tcpip.h>

#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"
#include "net/ping.h"
#include "net/ftp_client_n/ftpclient.h"

#include <netif/loopif.h>
#include <netif/ethdm9k.h>


void my_server_init(void);
void my_client_init(void);

void setup_netif(void) {
static struct ip_addr IpAddr, NetMaskCode, Gateway;

#if 0
struct netif *loop_netif;
	loop_netif = malloc(sizeof(struct netif));
	/*Assign IP address*/
	IP4_ADDR(&IpAddr, 127,0,0,1);
	IP4_ADDR(&Gateway, 127,0,0,1);
	IP4_ADDR(&NetMaskCode, 255,255,255,0);

	printf("Starting lwIP, local interface IP is %s\n", inet_ntoa(*(struct in_addr*) &IpAddr));

	/*add interface to the list*/
	netif_set_default(netif_add(loop_netif, &IpAddr, &NetMaskCode, &Gateway, NULL, loopif_init,	tcpip_input));

	/*set up the interface*/
	netif_set_up(loop_netif);
#else
struct netif *dm9k_netif;

	dm9k_netif = malloc(sizeof(struct netif));
	/*Assign IP address*/
	IP4_ADDR(&IpAddr, 192,168,1,37);
	IP4_ADDR(&Gateway, 192,168,1,1);
	IP4_ADDR(&NetMaskCode, 255,255,255,0);

	struct in_addr* temp_addr=(struct in_addr*) &IpAddr;
	printf("Starting lwIP, local interface IP is %s\n", inet_ntoa(*temp_addr));
	/*add interface to the list*/
	netif_set_default(netif_add(dm9k_netif, &IpAddr, &NetMaskCode, &Gateway, NULL, ethdm9k_init, tcpip_input));

	/*set up the interface*/
	netif_set_up(dm9k_netif);

#endif

}


static void tcpip_init_done(void *arg) {
	sys_sem_signal((sys_sem_t)arg);
}

static int inited=0;
void lwip(int argc, char** argv) {
//	sys_thread_new("",net_thread,0,2048,2);

	if (argc<2) {
		goto show_usage;
	}

	if (!stricmp(argv[1],"init")) {
	sys_sem_t sem;
		if (!inited) {

			sem=sys_sem_new(0);
			tcpip_init(tcpip_init_done, (void*)sem);
			sys_arch_sem_wait(sem,0);
			sys_sem_free(sem);

			/* init network interfaces */
			setup_netif();
#if 0
			/* init application */
			my_server_init(); /* My Server */
#endif 	/* move to 's' option */

			inited=1;
		}
		return;
	}

	if (!strcmp(argv[1],"s")) {
		/* init application */
		my_server_init(); /* My Server */

		my_client_init(); /* My Client */
		return;
	}

	if (!strcmp(argv[1], "reset")) {
		ethdm9k_reset();

		return;
	}

	if (!strcmp(argv[1], "ping")) {
		struct ip_addr target;
		int s, ping_cnt = 0;
		int timeout = PING_RCV_TIMEO;

		if (argc <= 2) {
			printf("no destination ip addr specified\n");
			return;
		}

		inet_aton(argv[2], (struct in_addr*) &target);
		struct in_addr* temp_addr=(struct in_addr*) &target;
		printf("ping target addr: %s\n", inet_ntoa(*temp_addr));

		if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
			printf("create socket err\n");
			return;
		}

		lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		while (ping_cnt < 4) {

			if (ping_send(s, &target) == ERR_OK) {
				LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
				ip_addr_debug_print(PING_DEBUG, &target);
				LWIP_DEBUGF( PING_DEBUG, ("\n"));

				ping_recv(s);
			} else {
				LWIP_DEBUGF( PING_DEBUG, ("ping: send "));
				ip_addr_debug_print(PING_DEBUG, &target);
				LWIP_DEBUGF( PING_DEBUG, (" - error\n"));
			}
			ping_cnt++;
			sys_msleep(PING_DELAY);
		}

		lwip_close(s);
		return;
	}

	if (!strcmp(argv[1], "ftp")) {
//		ftp_get("192.168.1.34","abc","abc","eftiwall.flf",0,0,DEST_FLASH);
//		ftp_get("192.168.1.34","abc","abc","image.dsu",0,0,DEST_FLASH);
		ftp_get("192.168.1.34","abc","abc","linux_basic.dsu",0,0,DEST_FLASH);
//		ftp_get("192.168.1.34","abc","abc","eftiwall.flf",(char*)0x33800000,0x4000,DEST_RAM);

		return;
	}

show_usage:
	printf(
		"%s <init><s>\n"
	,argv[0]);
	return;
}

