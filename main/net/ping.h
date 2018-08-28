#ifndef __PING_H__
#define __PING_H__

#if 0
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif
#else
#define PING_DEBUG	(LWIP_DBG_ON|LWIP_DBG_LEVEL_WARNING)
#endif

/** ping receive timeout - in milliseconds */
#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     1000
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

err_t ping_send(int s, struct ip_addr *addr);
void ping_recv(int s);

#endif /* __PING_H__ */
