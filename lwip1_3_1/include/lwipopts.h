/*
 * lwipopts.h
 *
 *  Created on: 2009/8/29
 *      Author: ww
 */

#ifndef LWIPOPTS_H_
#define LWIPOPTS_H_

#define LWIP_DEFAULT_STACK_SIZE		4096

#define DEBUG_ON						(LWIP_DBG_ON|LWIP_DBG_LEVEL_WARNING)
#define DEBUG_OFF						0

#define LWIP_DEBUG
#define LWIP_DBG_MIN_LEVEL				LWIP_DBG_LEVEL_WARNING
#define LWIP_DBG_TYPES_ON				0xff
#define ETHARP_DEBUG					DEBUG_OFF
#define NETIF_DEBUG                     DEBUG_OFF
#define PBUF_DEBUG                      DEBUG_ON
#define API_LIB_DEBUG					DEBUG_OFF
#define API_MSG_DEBUG                   DEBUG_OFF
#define SOCKETS_DEBUG                   DEBUG_OFF
#define ICMP_DEBUG                      DEBUG_OFF
#define IGMP_DEBUG                      DEBUG_OFF
#define INET_DEBUG                      DEBUG_OFF
#define IP_DEBUG                        DEBUG_OFF
#define IP_REASS_DEBUG                  DEBUG_OFF
#define RAW_DEBUG                       DEBUG_OFF
#define MEM_DEBUG                       DEBUG_OFF
#define MEMP_DEBUG                      DEBUG_OFF
#define SYS_DEBUG                       DEBUG_OFF
#define TCP_DEBUG                       DEBUG_OFF
#define TCP_INPUT_DEBUG                 DEBUG_ON
#define TCP_FR_DEBUG                    DEBUG_OFF
#define TCP_RTO_DEBUG                   DEBUG_OFF
#define TCP_CWND_DEBUG                  DEBUG_OFF
#define TCP_WND_DEBUG                   DEBUG_OFF
#define TCP_OUTPUT_DEBUG                DEBUG_OFF
#define TCP_RST_DEBUG                   DEBUG_OFF
#define TCP_QLEN_DEBUG                  DEBUG_OFF
#define UDP_DEBUG                       DEBUG_OFF
#define TCPIP_DEBUG                     DEBUG_OFF
#define PPP_DEBUG                       DEBUG_OFF
#define SLIP_DEBUG                      DEBUG_OFF
#define DHCP_DEBUG                      DEBUG_OFF
#define AUTOIP_DEBUG                    DEBUG_OFF
#define SNMP_MSG_DEBUG                  DEBUG_OFF
#define SNMP_MIB_DEBUG                  DEBUG_OFF
#define DNS_DEBUG                       DEBUG_OFF


#define MEMP_MEM_MALLOC						1
#define MEM_USE_POOLS						0
#define MEM_LIBC_MALLOC						1
#define MEM_ALIGNMENT						0x10
//#define MEM_SIZE							(4*1024*1204)
#define MEMP_OVERFLOW_CHECK					1
#define MEMP_SANITY_CHECK					1
#define LWIP_SO_RCVTIMEO					1
#define LWIP_SO_RCVBUF						1
#define RECV_BUFSIZE_DEFAULT				2048
#define TCP_QUEUE_OOSEQ						1

#define LWIP_SOCKET							1
#define LWIP_NETCONN						1
#define NO_SYS								0
#define LWIP_NETIF_LOOPBACK_MULTITHREADING	1



//   ---------- Network Interfaces options ----------
#define LWIP_NETIF_LOOPBACK             1

//   ---------- LOOPIF options ----------
#define LWIP_HAVE_LOOPIF				1

//   ---------- Thread options ----------
#define TCPIP_THREAD_NAME              "tcpip"
#define TCPIP_THREAD_STACKSIZE          4096
#define TCPIP_THREAD_PRIO               12
#define TCPIP_MBOX_SIZE                 8

#define DEFAULT_THREAD_STACKSIZE		LWIP_DEFAULT_STACK_SIZE
#define DEFAULT_THREAD_PRIO             32


#define DEFAULT_RAW_RECVMBOX_SIZE		16
#define DEFAULT_UDP_RECVMBOX_SIZE       16
#define DEFAULT_TCP_RECVMBOX_SIZE       16
#define DEFAULT_ACCEPTMBOX_SIZE         16


#endif /* LWIPOPTS_H_ */
