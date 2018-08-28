#include <string.h>
#include <stdlib.h>
#include "2440addr.h"
#include "tq_hw_defs.h"
#include "platform/YL_SYS.h"
#include <platform/yl_errno.h>
#include "platform/TQ_BSP.h"

#define S3C2410_SPCON_SMOD_DMA	  (2<<5)	/* DMA mode */
#define S3C2410_SPCON_SMOD_INT	  (1<<5)	/* interrupt mode */
#define S3C2410_SPCON_SMOD_POLL   (0<<5)	/* polling mode */
#define S3C2410_SPCON_ENSCK	  (1<<4)		/* Enable SCK */
#define S3C2410_SPCON_MSTR	  (1<<3)		/* Master/Slave select
										       0: slave, 1: master */
#define S3C2410_SPCON_TAGD	  (1<<0)		/* Tx Auto Garbage Data Mode */

#define S3C2410_SPCON_CPOL_HIGH	  (1<<2)	/* Clock polarity select */
#define S3C2410_SPCON_CPOL_LOW	  (0<<2)	/* Clock polarity select */

#define S3C2410_SPCON_CPHA_FMTB	  (1<<1)	/* Clock Phase Select */
#define S3C2410_SPCON_CPHA_FMTA	  (0<<1)	/* Clock Phase Select */

#define S3C2410_SPCON_TAGD	  (1<<0)	/* Tx auto garbage data mode */

#define S3C2410_SPSTA_DCOL	  (1<<2)	/* Data Collision Error */
#define S3C2410_SPSTA_MULD	  (1<<1)	/* Multi Master Error */
#define S3C2410_SPSTA_READY	  (1<<0)	/* Data Tx/Rx ready */

#define S3C2410_SPPIN_ENMUL	  (1<<2)	/* Multi Master Error detect */
#define S3C2410_SPPIN_RESERVED	  (1<<1)
#define S3C2400_SPPIN_nCS     	  (1<<1)	/* SPI Card Select */
#define S3C2410_SPPIN_KEEP	  (1<<0)	/* Master Out keep */


/* ccchiu: just test code for slave mode receiving dvbt data */
int spislv_int(void);
extern BYTE *dvbts;
extern int ts_byte_cnt;

int	spislv_init(void)
{
#if 1
	BIT_SET(rCLKCON, CLK_CON_SPI);

	rSPCON0 = S3C2410_SPCON_SMOD_INT | S3C2410_SPCON_TAGD;
	BIT_CLR(rSPCON0, S3C2410_SPCON_MSTR);		// slave mode

	rSPPIN0 = 0;
	rSPPRE0 = 0;		// TODO: check if necessary or not => PCLK / 2

	//set interrupt vector table
//	tq_irqSetISR(22, spislv_int);
//	BIT_CLR(rINTMSK,BIT_SPI0);

	//rSPTDAT0 = 0xFF;
#else
	BIT_SET(rSPCON1, S3C2410_SPCON_SMOD_INT);
	BIT_CLR(rSPCON1, S3C2410_SPCON_MSTR);		// slave mode

	BIT_SET(rSPPIN1, 0);

	//set interrupt vector table
	tq_irqSetISR(29, spislv_int);
	BIT_CLR(rINTMSK,BIT_SPI1);

	rSPTDAT1 = 0xFF;
#endif

	return SUCCESS;
}

int spislv_int(void)
{
	U32	spista;

#if 1
	spista = rSPSTA0;
#else
	spista = rSPSTA1;
#endif

//	printf("enter %s\n", __func__);
	if (spista & S3C2410_SPSTA_DCOL) {
		printf("data collison\n");
		return 0;
	}

	if (spista & S3C2410_SPSTA_READY) {
		if (ts_byte_cnt < 5242880) {
#if 1
			dvbts[ts_byte_cnt] = (BYTE)(rSPRDAT0 & 0xFF);
//			rSPTDAT0 = 0xFF;
#else
			dvbts[ts_byte_cnt] = (BYTE)(rSPRDAT1 & 0xFF);
			rSPTDAT1 = 0xFF;
#endif
			ts_byte_cnt ++;
		}
		else {
			printf("5M dvbt data received...\n");
			BIT_SET(rINTMSK,BIT_SPI0);
		}
	}

#if 1
	BIT_SET(rSRCPND,BIT_SPI0);
	BIT_SET(rINTPND,BIT_SPI0);
#else
	BIT_SET(rSRCPND,BIT_SPI1);
	BIT_SET(rINTPND,BIT_SPI1);
#endif

	return 0;
}

