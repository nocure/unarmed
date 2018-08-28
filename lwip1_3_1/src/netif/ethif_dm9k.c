#include <platform/tq_bsp.h>
#include <platform/yl_sys.h>
#include <platform/yl_errno.h>
#include <string.h>
#include <bsp/2440addr.h>
#include <bsp/tq_hw_defs.h>
#include "ethif_dm9k.h"

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include <sct.h>

//#define PKTDUMP
/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

// Struct declaration
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  /* TODO: add some tx/rx statistics here */
};


// global variable
#define ETH_ADDR_LEN			6
unsigned char DEF_MAC_ADDR[ETH_ADDR_LEN];
static unsigned char dm9k_srom[128];
static struct netif *dm9k_netif;
static u8 SendPacket[DM9000_PKT_MAX];

static void ethdm9k_input(struct netif *netif, struct pbuf *p);


// Code Implementations
static u8 ior(u8 reg) {
	rETHREGADDR = reg;
	return(rETHREGDATB);
}


static void iow(u8 reg, u8 writedata) {
	rETHREGADDR = reg;
	rETHREGDATB = writedata;
}

//static u8 ReceiveData[DM9000_PKT_MAX];
static void dm9k_recv(void) {
//	struct netif *netif;
	struct pbuf *p = NULL, *q = NULL;
	u16 i,j;
//	u8 *ptr;
//	u16 *rdptr;
//	u32 ReceiveLength;
//	u16 calc_len;
	u16 calc_MRR;
	u32 rxbyte;
	u32 GoodPacket, jump_packet;
	u32 rx_status, rx_length;

	/* Check packet ready or not */
//	rdptr = (u16 *)ReceiveData;
//	ptr = ReceiveData;
	do {
//		ReceiveLength = 0;
		jump_packet = FALSE;
		ior(DM9000_MRCMDX);	/* Dummy read */
		calc_MRR = (ior(DM9000_MRRH) << 8) + ior(DM9000_MRRL);
		/* Get most updated data */
		rxbyte = ior(DM9000_MRCMDX);

		/* Status check: this byte must be 0 or 1 */
		if (rxbyte > DM9000_PKT_RDY) {
			printf("status check failed: %d\n", rxbyte);
			iow(DM9000_RCR, 0x00);	/* Stop Device */
			iow(DM9000_ISR, IMR_PAR);	/* Stop INT request */
			return;
		}

		if (rxbyte != DM9000_PKT_RDY)
			return;

		/* A packet ready now  & Get status/length */
		GoodPacket = TRUE;
		rETHREGADDR = DM9000_MRCMD;

		rx_status = rETHREGDATW;
		rx_length = rETHREGDATW;

//		printf("rcv packet length: %d, status: 0x%02x\n", rx_length, rx_status);
		/* Packet Status check */
		if (rx_length < 0x40) {
			GoodPacket = FALSE;
			printf("Bad Packet received (runt)\n");
		}

		if (rx_length > DM9000_PKT_MAX) {
			jump_packet = TRUE;
			printf("RST: RX Len:%x\n", rx_length);
		}

		if (rx_status & 0xbf00) {
			GoodPacket = FALSE;
			if (rx_status & 0x100) {
				printf("fifo error\n");
			}
			if (rx_status & 0x200) {
				printf("crc error\n");
			}
			if (rx_status & 0x8000) {
				printf("length error\n");
			}
		}

		/* Move data from DM9000 */
		if (GoodPacket) {
			p = pbuf_alloc(PBUF_RAW, rx_length-4, PBUF_POOL);
			if (p != NULL) {

				calc_MRR += (rx_length + 4);
				if(rx_length & 0x01) calc_MRR++;
				if(calc_MRR > 0x3fff) calc_MRR -= 0x3400;

				if(jump_packet) {
					iow (DM9000_MRRH, (calc_MRR >> 8) & 0xff);
					iow (DM9000_MRRL, calc_MRR & 0xff );
					continue;
				}
#if 0
				if (rx_length>=1536) {
					calc_len=0;
				}
				calc_len = (rx_length + 1) >> 1;
				for(i = 0 ; i < calc_len ; i++)
					rdptr[i] = rETHREGDATW;

				ReceiveLength = rx_length - 4;

				/* We iterate over the pbuf chain until we have read the entire
				 * packet into the pbuf. */
				for(i=0,q = p; q != NULL; q = q->next) {
					/* Read enough bytes to fill this pbuf in the chain. The
					 * available data in the pbuf is given by the q->len
					 * variable. */
					//printf("%s: q->len: %d, ReceiveLength: %d\n", __func__, q->len, ReceiveLength);

					memcpy(q->payload, ReceiveData+i, q->len);
					//ptr += ReceiveLength ;
					//ReceiveLength -= q->len;
					if (i) {
						ReceiveLength = rx_length - 4;
					}
					i+=q->len;
				}
#else

				for(i=0,q = p; q != NULL; q = q->next) {
					int seg_len,seg_remain;
					seg_len=q->len&0xfffffffe;
					seg_remain=q->len&1;
					for (j=0;j<seg_len;j+=2) {
						*((u16*)&(((u8*)q->payload)[j]) )=rETHREGDATW;
					}
					i+=j;
					if (seg_remain) {
						*((u8*)&(((u8*)q->payload)[j]) )=(u8)rETHREGDATW;
						i+=2;
					}
				}

				for (;i<rx_length;i+=2) rETHREGDATW;

#endif

				//netif = dm9k_netif;
				/* Pass to upper layer */
				//ethdm9k_input(netif, p);
				ethdm9k_input(dm9k_netif, p);

				if(calc_MRR != ((ior(DM9000_MRRH) << 8) + ior(DM9000_MRRL))) {
					iow(DM9000_MRRH, (calc_MRR >> 8) & 0xff);
					iow(DM9000_MRRL, calc_MRR & 0xff);
				}
			}
			else {
				/* TODO: need to dump the packet's data */
				printf("allocate pbuf error\n");
				break;
			}
		}
		else {
			printf("bad packet\n");
			break;
		}
	} while (rxbyte == DM9000_PKT_RDY);

	return;
}


/*
 *  Hardware start transmission.
 *  Send a packet to media from the upper layer.
 */
static void dm9k_xmit(u8 *p_char, u32 length)
{
	u32 SendLength = length;
	u16 *SendData = (u16 *) p_char;
	u32 i;
	u32 calc_len;
	u32 calc_MWR;

#ifdef PKTDUMP
  int j, k;
  printf("%s START LOG\n", __func__);

  for (j = 0; j <= (length) / 16; j ++) {
	  for (k = 0; k < 16; k ++) {
		  printf("%02x ", p_char[j * 16 + k]);
		  if ((j*16+k) >= length)
			  break;
	  }
	  printf("\n");
  }
	printf("%s END LOG, packet length: %d\n", __func__, length);
#endif


	while(ior(DM9000_TCR) & DM9000_TCR_SET)
	{
		udelay (5);
	}

	calc_MWR = (ior(DM9000_MWRH) << 8) + ior(DM9000_MWRL);
	calc_MWR += SendLength;
	if(SendLength & 0x01) calc_MWR++;
	if(calc_MWR > 0x0bff) calc_MWR -= 0x0c00;

	iow(DM9000_TXPLH, (SendLength >> 8) & 0xff);
	iow(DM9000_TXPLL, SendLength & 0xff);

	rETHREGADDR = DM9000_MWCMD;
	calc_len = (SendLength + 1) >> 1;
	for(i = 0; i < calc_len; i++)
		rETHREGDATW = SendData[i];

	iow(DM9000_TCR, DM9000_TCR_SET);

	if(calc_MWR != ((ior(DM9000_MWRH) << 8) + ior(DM9000_MWRL))) {
		iow(DM9000_MWRH, (calc_MWR >> 8) & 0xff);
		iow(DM9000_MWRL, calc_MWR & 0xff);
	}

	return;
}


void dm9k_isr(void) {
	/* Disable all interrupts */
	iow(DM9000_IMR, IMR_PAR);	// ISR_ROOS,ISR_ROS,ISR_PTS,ISR_PRS = 0
	set_flg(FLGID_DM9K_INT,1);
}

#if 0
static int eint4_7InterruptHandler(void)
{

	DWORD eintno,pending,clearpending;
	int int_status;

	pending=rEINTPEND;
//printf("entering %s, pending bits: 0x%08x\n", __func__, pending);

	while(1) {
		eintno=ffs(pending);

		if (!eintno) break;
		eintno--;

		switch(eintno) {
			case 7:
				/* Disable all interrupts */
				iow(DM9000_IMR, IMR_PAR);
				/* Got DM9000 interrupt status */
				int_status = ior(DM9000_ISR);		/* Got ISR */
				//printf("%s: int status: 0x%08x\n", __func__, int_status);
				iow(DM9000_ISR, int_status);	/* Clear ISR status */

				/* Received the coming packet */
				if (int_status & ISR_PRS)
					dm9k_recv();

				/* Trnasmit Interrupt check */
			//	if (int_status & ISR_PTS)
			//		dm9000_tx_done();		// TODO: no check tx_done event

				/* Re-enable interrupt mask */
				iow(DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);

				break;
		}

		clearpending=1<<eintno;
		BIT_SET(rEINTPEND,clearpending);		// clear pending
		BIT_CLR(pending,clearpending);		// clear temp
	}

	BIT_SET(rSRCPND,BIT_EINT4_7);
	BIT_SET(rINTPND,BIT_EINT4_7);

	return 0;
}
#endif

static void dm9k_reset(void) {
	printf("dm9000 resetting...\n");
	/* RESET device */
#if 0
	iow(DM9000_NCR, NCR_RST);
	udelay(10);
	iow(DM9000_NCR, NCR_RST);
	udelay(10);
#else
	iow(DM9000_GPCR, GPCR_GEP_CNTL);
	/* Step 1: Power internal PHY by writing 0 to GPIO0 pin */
	iow(DM9000_GPR, 0);
	/* Step 2: Software reset */
	iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST));

	do {
		printf("resetting the DM9000, 1st reset\n");
		udelay(5); /* Wait at least 20 us */
	} while (ior(DM9000_NCR) & 1);

	iow(DM9000_NCR, 0);
	iow(DM9000_NCR, (NCR_LBK_INT_MAC | NCR_RST)); /* Issue a second reset */

	do {
		printf("resetting the DM9000, 2nd reset\n");
		udelay(5); /* Wait at least 20 us */
	} while (ior(DM9000_NCR) & 1);

	/* Check whether the ethernet controller is present */
	if ((ior(DM9000_PIDL) != 0x0) ||
	    (ior(DM9000_PIDH) != 0x90))
		printf("ERROR: resetting DM9000 -> not responding\n");
#endif
}

/*
 *   Read a word from phyxcer
 */
static u32 dm9k_phy_read(u8 reg) {
	u32 ret;

	/* Fill the phyxcer register into REG_0C */
	iow(DM9000_EPAR, DM9000_PHY | reg);

	iow(DM9000_EPCR, 0xc);	/* Issue phyxcer read command */
	udelay(100);			/* Wait read complete */
	iow(DM9000_EPCR, 0x0);	/* Clear phyxcer read command */

	/* The read data keeps on REG_0D & REG_0E */
	ret = (ior(DM9000_EPDRH) << 8) | ior(DM9000_EPDRL);

	return ret;
}

#if 0
/*
 *   Write a word to phyxcer
 */
static void dm9k_phy_write(u8 reg, u16 value)
{
	/* Fill the phyxcer register into REG_0C */
	iow(DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	iow(DM9000_EPDRL, (value & 0xff));
	iow(DM9000_EPDRH, ((value >> 8) & 0xff));

	iow(DM9000_EPCR, 0xa);	/* Issue phyxcer write command */
	udelay(500);				/* Wait write complete */
	iow(DM9000_EPCR, 0x0);	/* Clear phyxcer write command */

}

static void dm9k_chkmedia(void)
{
	while(!(ior(DM9000_NSR) & DM9000_PHY))
	{
		udelay(2000);
	}
}
#endif

/*
 *  Read a word data from SROM
 */
static u16 read_srom_word(int offset) {
	iow(DM9000_EPAR, offset);
	iow(DM9000_EPCR, EPCR_ERPRR);
	mdelay(8);		/* according to the datasheet 200us should be enough,
				   		but it doesn't work */
	iow(DM9000_EPCR, 0x0);
	return (ior(DM9000_EPDRL) + (ior(DM9000_EPDRH) << 8));
}


static void dm9k_hash_table(void) {
	u32 i;

	for(i = 0; i < 6; i++)
		iow(DM9000_PAR + i, DEF_MAC_ADDR[i]);

	for(i = 0; i < 8; i++)
		iow(DM9000_MAR + i, 0x00);
	iow(DM9000_MAR + 7, 0x80);
}


/*
 * Initilize dm9000 board
 */
static void init_dm9000(void)
{
	u32 io_mode;
	int i, lnk;

	printf("entering %s\n",__func__);

	/* I/O mode */
	io_mode = ior(DM9000_ISR) >> 6;	/* ISR bit7:6 keeps I/O mode */
	if(io_mode != DM9000_WORD_MODE) {
		printf("DM9K_DEBUG ==> DEIVCE NOT WORD MODE, SYSTEM HOLD !!\n");
		while(1);
	}
	else
	{
		printf("DM9K_DEBUG ==> DEIVCE IS WORD MODE !!\n");
	}

	/* Program operating register */
	iow(DM9000_NCR, 0x0);
	iow(DM9000_TCR, 0);	        /* TX Polling clear */
	iow(DM9000_BPTR, 0x3f);		/* Less 3Kb, 200us */
	iow(DM9000_FCTR, 0x38);		/* Flow Control */
	iow(DM9000_FCR, 0x28);
	iow(DM9000_SMCR, 0);        /* Special Mode */
	/* clear TX status */
	iow(DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	iow(DM9000_ISR, ISR_CLR_STATUS); /* Clear interrupt status */

	for (i = 0; i < 64; i++)
		((u16 *) dm9k_srom)[i] = read_srom_word(i);

	/* Set Node Address */
	//for (i = 0; i < 6; i++)
	//	DEF_MAC_ADDR[i] = dm9k_srom[i];
	// TODO: configurable MAC address

	printf("mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
			DEF_MAC_ADDR[0], DEF_MAC_ADDR[1], DEF_MAC_ADDR[2],
			DEF_MAC_ADDR[3], DEF_MAC_ADDR[4], DEF_MAC_ADDR[5]);

	/* Set address filter table */
	dm9k_hash_table();

	/* Activate DM9000 */
	iow(DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
//	iow(DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
	iow(DM9000_IMR, IMR_PAR | IMR_PRM);

	i = 0;
	while(!(dm9k_phy_read(1)& 0x20)) {
		udelay(1000);
		i ++;
		if (i == 10000) {
			printf("could not establish link.\n");
			return;
		}
	}

	lnk = dm9k_phy_read(17) >> 12;
	printf("operating at ");
	switch(lnk){
	case 1:
		printf("10M half duplex ");
		break;
	case 2:
		printf("10M full duplex ");
		break;
	case 4:
		printf("100M half duplex ");
		break;
	case 8:
		printf("100M full duplex ");
		break;
	default:
		printf("unknown: %d ", lnk);
	}
	printf("mode\n");
	return;
}


static int dm9k_initnic(void)
{
	int	i;
	u32 id_val;

	dm9k_reset();

	/* try two times, DM9000 sometimes gets the first read wrong */
	for (i = 0; i < 2; i++) {
		id_val  = ior(DM9000_VIDL);
		id_val |= (u32)ior(DM9000_VIDH) << 8;
		id_val |= (u32)ior(DM9000_PIDL) << 16;
		id_val |= (u32)ior(DM9000_PIDH) << 24;

		if (id_val == DM9000_ID)
			break;
		printf("%s: read wrong id 0x%08x\n", __func__, id_val);
	}

	if (id_val != DM9000_ID) {
		printf("%s: wrong id: 0x%08x\n", __func__, id_val);
		return YLERR_IO;
	}

	init_dm9000();

	// Enable interrupt
	BIT_CLR(rEXTINT0, 7 << 28);
	BIT_SET(rEXTINT0, 1 << 28);
//	printf("rEXTINT0: 0x%08x\n", rEXTINT0);
	// high level triggered interrupt

#if 1
//	tq_irqSetISR(EINT4_7, eint4_7InterruptHandler);
//	EnableIrq(BIT_EINT4_7);
	BIT_CLR(rEINTMASK,BIT_EINT7);
#else
	act_cyc(CYCID_ETH_RCV,TCY_ON);
#endif

//	dm9k_chkmedia();

	return 0;
}


//==========================================================================
//==========================================================================
//==========================================================================

void ethdm9k_reset(void)
{
	dm9k_initnic();

	return;
}

#if 0
void ethdm9k_poll(void) {
int int_status;

	/* Disable all interrupts */
	iow(DM9000_IMR, IMR_PAR);
	/* Got DM9000 interrupt status */
	int_status = ior(DM9000_ISR);		/* Got ISR */
	//printf("%s: int status: 0x%08x\n", __func__, int_status);
	iow(DM9000_ISR, int_status);	/* Clear ISR status */

	/* Received the coming packet */
	if (int_status & ISR_PRS)
		dm9k_recv();

	/* Re-enable interrupt mask */
	iow(DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);

	return;
}
#endif

void ethdm9k_tsk(DWORD thread_input) {
int er;
DWORD flg;
int int_status;
	while(1) {
		er=wai_flg((DWORD*)&flg,FLGID_DM9K_INT,0xffffffff,TWF_ORW|TWF_CLR);
		if (er!=0) continue;
		/* Got DM9000 interrupt status */
		int_status = ior(DM9000_ISR);		/* Got ISR */
		//printf("%s: int status: 0x%08x\n", __func__, int_status);

		/* Received the coming packet */
		if (int_status & ISR_PRS) {
//			LWIP_DEBUGF(NETIF_DEBUG, ("["));
			dm9k_recv();
//			LWIP_DEBUGF(NETIF_DEBUG, ("]"));
		}
		else {
//			LWIP_DEBUGF(NETIF_DEBUG, ("!"));
		}

		iow(DM9000_ISR, int_status);	/* Clear ISR status */

		/* Re-enable interrupt mask */
//		iow(DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);
		iow(DM9000_IMR, IMR_PAR | IMR_PRM);
	}
}


//==========================================================================
//====LWIP INTERFACE========================================================
//==========================================================================


/* LWIP netif interface APIs */
static void ethdm9k_input(struct netif *netif, struct pbuf *p)
{
//  struct ethernetif *ethernetif;
  struct eth_hdr *ethhdr;
#ifdef PKTDUMP
  int j, k;
#endif
//  ethernetif = netif->state;

  /* no packet could be read, silently ignore this */
  if (p == NULL) return;
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

#ifdef PKTDUMP
  printf("%s START LOG, type: %04x\n", __func__, htons(ethhdr->type));

  printf("%s: recv packet dest to %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
		  ethhdr->dest.addr[0], ethhdr->dest.addr[1], ethhdr->dest.addr[2],
		  ethhdr->dest.addr[3], ethhdr->dest.addr[4], ethhdr->dest.addr[5]);
  printf("%s:            src from %02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
		  ethhdr->src.addr[0], ethhdr->src.addr[1], ethhdr->src.addr[2],
		  ethhdr->src.addr[3], ethhdr->src.addr[4], ethhdr->src.addr[5]);

  if (ethhdr->dest.addr[0] == 0x00 && ethhdr->dest.addr[1] == 0x60
		  && ethhdr->dest.addr[2] == 0x6e && ethhdr->dest.addr[3] == 0x90
		  && ethhdr->dest.addr[4] == 0x00 && ethhdr->dest.addr[5] == 0xae)
  {
	  for (j = 0; j <= (p->len) / 16; j ++) {
		  for (k = 0; k < 16; k ++) {
			  printf("%02x ", ((u8 *)p->payload)[j * 16 + k]);
			  if ((j*16+k) >= p->len)
				  break;
		  }
		  printf("\n");
  	  }
	  printf("\n");
  }
  printf("%s END LOG, packet length: %d\n", __func__, p->len);
#endif

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
//		etharp_ip_input(netif, p); /* update ARP table, obtain first queued packet */
//		pbuf_header(p, -14); /* skip Ethernet header */
		netif->input(p, netif); /* pass to network layer */
		break;

  case ETHTYPE_ARP:
		/* pass p to ARP module, get ARP reply or ARP queued packet */
		etharp_arp_input(netif, ((struct ethernetif *)netif->state)->ethaddr, p);
		break;
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif)!=ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}


static err_t ethdm9k_output(struct netif *netif, struct pbuf *p) {
  //struct ethernetif *ethernetif = netif->state;
  struct pbuf *q;
  u8 *sndp = SendPacket;
  u32 pktlen = 0;
#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
	  memcpy(sndp, q->payload, q->len);
	  sndp += q->len;
	  pktlen += q->len;
  }

  dm9k_xmit(SendPacket, pktlen);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}


err_t ethdm9k_init(struct netif *netif) {
	  struct ethernetif *ethernetif;

	  LWIP_ASSERT("netif != NULL", (netif != NULL));

	  ethernetif = mem_malloc(sizeof(struct ethernetif));
	  if (ethernetif == NULL) {
	    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
	    return ERR_MEM;
	  }

#if LWIP_NETIF_HOSTNAME
	  /* Initialize interface hostname */
	  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

	  /*
	   * Initialize the snmp variables and counters inside the struct netif.
	   * The last argument should be replaced with your link speed, in units
	   * of bits per second.
	   */
	  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

	  netif->state = ethernetif;
	  netif->name[0] = IFNAME0;
	  netif->name[1] = IFNAME1;
	  /* We directly use etharp_output() here to save a function call.
	   * You can instead declare your own function an call etharp_output()
	   * from it if you have to do some checks before sending (e.g. if link
	   * is available...) */
	  netif->output = etharp_output;
	  netif->linkoutput = ethdm9k_output;

	  /* set MAC hardware address length */
	  netif->hwaddr_len = ETHARP_HWADDR_LEN;


	  // TODO: configurable MAC address
	  DEF_MAC_ADDR[0] = 0x00;
	  DEF_MAC_ADDR[1] = 0x60;
	  DEF_MAC_ADDR[2] = 0x6e;
	  DEF_MAC_ADDR[3] = 0x90;
	  DEF_MAC_ADDR[4] = 0x00;
	  DEF_MAC_ADDR[5] = 0xae;

	  /* set MAC hardware address */
	  netif->hwaddr[0] = DEF_MAC_ADDR[0];
	  netif->hwaddr[1] = DEF_MAC_ADDR[1];
	  netif->hwaddr[2] = DEF_MAC_ADDR[2];
	  netif->hwaddr[3] = DEF_MAC_ADDR[3];
	  netif->hwaddr[4] = DEF_MAC_ADDR[4];
	  netif->hwaddr[5] = DEF_MAC_ADDR[5];

	  /* maximum transfer unit */
	  netif->mtu = 1500;

	  /* device capabilities */
	  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

	  dm9k_netif = netif;
	  dm9k_initnic();

	  return ERR_OK;
}




