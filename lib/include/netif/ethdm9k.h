#ifndef ETHERNETIF_H_
#define ETHERNETIF_H_

#include "lwip/opt.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

err_t ethdm9k_init(struct netif *netif);
void ethdm9k_poll(void);
void ethdm9k_reset(void);

#ifdef __cplusplus
}
#endif


#endif /*ETHERNETIF_H_*/
