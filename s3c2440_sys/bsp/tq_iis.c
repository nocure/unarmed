/*
 * ww created
 */
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "2440addr.h"
#include "tq_hw_defs.h"

U32 IIS_CLK;

#define I2S_LRCK 	(2<<0)
#define I2S_SCLK 	(2<<2)
#define CDCLK		(2<<4)
#define I2S_SDI		(2<<6)
#define I2S_SDO		(2<<8)

#define I2S_DELAY	tq_i2sDelay()

static void i2s_rxctrl(int on);
static void i2s_txctrl(int on);
static int iis_lrsync(void);

static void tq_i2sDelay(void) {
int i;
	for (i=0;i<6000;i++);
}

int tq_i2s_enable(void)
{
	BIT_SET(rCLKCON, CLK_CON_IIS);

	//set GPE0: I2SLRCK
	//    GPE1: I2SSCLK
	//    GPE2: CDCLK
	//    GPE3: I2SSDI
	//	  GPE4: I2SSDO
	BIT_CLR(rGPECON,0x3ff);
	BIT_SET(rGPECON,(I2S_LRCK|I2S_SCLK|CDCLK|I2S_SDI|I2S_SDO));

	rIISCON = S3C2410_IISCON_IISEN;

	i2s_txctrl(0);
	i2s_rxctrl(0);

	return 0;
}

static void i2s_rxctrl(int on)
{
	/* TODO: use DMA mode */
	if (on) {
		BIT_SET(rIISMOD, S3C2410_IISMOD_RXMODE);
//		BIT_SET(rIISFCON, S3C2410_IISFCON_RXDMA | S3C2410_IISFCON_RXENABLE);
		BIT_SET(rIISFCON, S3C2410_IISFCON_RXENABLE);
		BIT_CLR(rIISCON, S3C2410_IISCON_RXIDLE);
//		BIT_SET(rIISCON, S3C2410_IISCON_RXDMAEN | S3C2410_IISCON_IISEN);
		BIT_SET(rIISCON, S3C2410_IISCON_IISEN);

	} else {
		//BIT_CLR(rIISFCON, S3C2410_IISFCON_RXDMA | S3C2410_IISFCON_RXENABLE);
		BIT_CLR(rIISFCON, S3C2410_IISFCON_RXENABLE);
		//BIT_CLR(rIISCON, S3C2410_IISFCON_RXDMA);
		BIT_SET(rIISCON, S3C2410_IISCON_RXIDLE);
		BIT_CLR(rIISMOD, S3C2410_IISMOD_RXMODE);
	}
}


static void i2s_txctrl(int on)
{

	/* TODO: use DMA mode */
	if (on) {
		BIT_SET(rIISMOD, S3C2410_IISMOD_TXMODE);
		//BIT_SET(rIISFCON, S3C2410_IISFCON_TXDMA | S3C2410_IISFCON_TXENABLE);
		BIT_SET(rIISFCON, S3C2410_IISFCON_TXENABLE);
		BIT_CLR(rIISCON, S3C2410_IISCON_TXIDLE);
		//BIT_SET(rIISCON, S3C2410_IISCON_TXDMAEN | S3C2410_IISCON_IISEN);
		BIT_SET(rIISCON, S3C2410_IISCON_IISEN);

	} else {
		//BIT_CLR(rIISFCON, S3C2410_IISFCON_TXDMA | S3C2410_IISFCON_TXENABLE);
		BIT_CLR(rIISFCON, S3C2410_IISFCON_TXENABLE);
		//BIT_CLR(rIISCON, S3C2410_IISFCON_TXDMA);
		BIT_SET(rIISCON, S3C2410_IISCON_TXIDLE);
		BIT_CLR(rIISMOD, S3C2410_IISMOD_TXMODE);
	}

}


int tq_i2s_set_sysclk(int clkid)
{
	BIT_CLR(rIISMOD, S3C2440_IISMOD_MPLL);

	if (clkid == S3C24XX_CLKSRC_MPLL)
		BIT_SET(rIISMOD, S3C2440_IISMOD_MPLL);

	return 0;
}


int tq_i2s_set_clkdiv(int divid, int div)
{
	U32	reg;

	switch (divid) {
		case S3C24XX_DIV_BCLK:
			reg = rIISMOD & ~(S3C2410_IISMOD_FS_MASK);
			rIISMOD = reg | div;
		break;
		case S3C24XX_DIV_MCLK:
			reg = rIISMOD & ~(S3C2410_IISMOD_384FS);
			rIISMOD = reg | div;
			break;
		case S3C24XX_DIV_PRESCALER:
			rIISPSR = div;
			BIT_SET(rIISCON, S3C2410_IISCON_PSCEN);
			break;
		default:
			return YLERR_INVAL;
	}

	return 0;
}


/* TODO: add more supported formats, and not only slave */
int tq_i2s_set_fmt(void)
{

	BIT_CLR(rIISMOD, S3C2410_IISMOD_SLAVE);
	BIT_CLR(rIISMOD, S3C2410_IISMOD_MSB);

	return 0;
}


int tq_i2s_hw_params(int format)
{

	switch(format) {
	case PCM_FORMAT_S8:
		BIT_CLR(rIISMOD, S3C2410_IISMOD_16BIT);
		break;
	case PCM_FORMAT_S16LE:
		BIT_SET(rIISMOD, S3C2410_IISMOD_16BIT);
		break;
	default:
		return YLERR_INVAL;
	}

	return 0;
}

/*
 * Check whether CPU is the master or slave
 */
static inline int iis_is_clkmaster(void)
{
	return (BIT_VAL(rIISMOD, S3C2410_IISMOD_SLAVE)? 0:1);
}

static int iis_lrsync(void)
{
	int timeout = 50; /* 5ms */

	while (1) {

		if (BIT_VAL(rIISCON, S3C2410_IISCON_LRINDEX))
			break;

		if (!timeout--)
			return YLERR_TIMEOUT;
		I2S_DELAY;
	}

	return 0;
}


int tq_i2s_trigger(int cmd, int stream)
{
	int ret = 0;

	switch (cmd) {
		case PCM_TRIGGER_START:
		case PCM_TRIGGER_RESUME:
		case PCM_TRIGGER_PAUSE_RELEASE:
			if (!iis_is_clkmaster()) {
				ret = iis_lrsync();
				if (ret)
					goto exit_err;
			}

		if (stream == PCM_STREAM_CAPTURE)
			i2s_rxctrl(1);
		else {
			int i = 0, txcnt;
			i2s_txctrl(1);
			do {
				txcnt = (rIISFCON >> 6) & 0x3F;

				if (txcnt == 32) {
					udelay(100);
					continue;
				}
				else {
					*IISFIFO = audioBuffer[i];
					i ++;
				}
			} while(i < AUDIO_BUFSIZE);
		}
		break;
	case PCM_TRIGGER_STOP:
	case PCM_TRIGGER_SUSPEND:
	case PCM_TRIGGER_PAUSE_PUSH:
		if (stream == PCM_STREAM_CAPTURE)
			i2s_rxctrl(0);
		else
			i2s_txctrl(0);
		break;
	default:
		ret = YLERR_INVAL;
		break;
	}

exit_err:
	return ret;
}

