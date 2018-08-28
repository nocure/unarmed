/*
 * TQ_BSP.h
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */

#ifndef TQ_BSP_H_
#define TQ_BSP_H_

#include "yl_type.h"

#define MEGA	1000000
#define FIN 	(12*MEGA)

// ==== cpu_init.c =====================================
extern unsigned int CPU_FREQ;
extern unsigned int FCLK;
extern unsigned int HCLK;
extern unsigned int PCLK;
extern unsigned int UCLK;


// ==== tq_irq.c ======================================
#define BIT_EINT4		(0x1<<4)
#define BIT_EINT5		(0x1<<5)
#define BIT_EINT6		(0x1<<6)
#define BIT_EINT7		(0x1<<7)
#define BIT_EINT8		(0x1<<8)
#define BIT_EINT9		(0x1<<9)
#define BIT_EINT10		(0x1<<10)
#define BIT_EINT11		(0x1<<11)
#define BIT_EINT12		(0x1<<12)
#define BIT_EINT13		(0x1<<13)
#define BIT_EINT14		(0x1<<14)
#define BIT_EINT15		(0x1<<15)
#define BIT_EINT16		(0x1<<16)
#define BIT_EINT17		(0x1<<17)
#define BIT_EINT18		(0x1<<18)
#define BIT_EINT19		(0x1<<19)
#define BIT_EINT20		(0x1<<20)
#define BIT_EINT21		(0x1<<21)
#define BIT_EINT22		(0x1<<22)
#define BIT_EINT23		(0x1<<23)
#define BIT_EINT24		(0x1<<24)		// Added for 2440.
#define BIT_EINT25		(0x1<<25)
#define BIT_EINT26		(0x1<<26)
#define BIT_EINT27		(0x1<<27)
#define BIT_EINT28		(0x1<<28)
#define BIT_EINT29		(0x1<<29)
#define BIT_EINT30		(0x1<<30)
#define BIT_EINT31		(0x1<<31)

typedef int (*ISR)(void);
void tq_irqSetISR(DWORD irq, ISR isr);
void tq_irqClearISR(DWORD irq);
void tq_irqInit(void);


// ==== yl_uart.c ======================================
#define UART_BAUD_RATE(br)		( (int)(PCLK/16./br+0.5) -1 )

#define UART_DATA_5_BIT			0	// default
#define UART_DATA_6_BIT			1
#define UART_DATA_7_BIT			2
#define UART_DATA_8_BIT			3

#define UART_PARITY_NONE		0			// default
#define UART_PARITY_ODD			(0x4<<3)
#define UART_PARITY_EVEN		(0x5<<3)
#define UART_PARITY_1			(0x6<<3)
#define UART_PARITY_0			(0x7<<3)

#define UART_STOP_1_BIT			(0<<2)	// default
#define UART_STOP_2_BIT			(1<<2)

#define UART_FLOW_CTRL_NONE		0
#define UART_FLOW_CTRL_HW		0x10
#define UART_FLOW_CTRL_SW		0x01	// not supported

#define UART_INFRAREAD_MODE		(1<<7)

#define UART_DATA_READY			(1<<31)
#define UART_RECV_ERROR			(1<<30)

// Error Codes
#define UART_ERROR_RECV			-1
#define UART_ERROR_TIMEOUT		-2
#define UART_ERROR_SEND			-3


typedef struct _UART_CONFIG_T {
	DWORD BaudRate;
	DWORD DataBits;
	DWORD Parity;
	DWORD StopBits;
	DWORD FlowControl;
	int TimeOut;		// milli-seconds
} UART_CONFIG_T;

// ---- UART HAL ----
void tq_uartInit(void);
void tq_uartEnable(void);
void tq_uartDisable(void);
void tq_uartConfig(UART_CONFIG_T* cfg);
int tq_uartPutChar(DWORD c);
int tq_uartGetChar(void);
int tq_uartGetCharT(int timeout);
int tq_uartGetCharNB(void);
int tq_uartWrite(BYTE* pbData,int length);
int tq_uartRead(BYTE* pbData,int length);


// ==== Timer =====================================================
void tq_timerInit(void);
void tq_precisetimerInit(void);
void dly_tsk_H(int dlytim);

#define mdelay(a) \
	({ \
		unsigned long long t = OSTICK + a; \
		do { } \
		while (!time_after(OSTICK, t)); \
	})

/* 400 MHZ ==> each loop cost 55 tick */
#define udelay(a) \
	({ \
		int i, d = 55 * a; \
		for (i = 0; i < d; i ++) {} \
	})

// ==== yl_gpio.c =====================================================
#define PORT_B			0x01000000
#define LED_1			(PORT_B|(1<<5))		// GPB5
#define LED_2			(PORT_B|(1<<6))		// GPB6
#define LED_3			(PORT_B|(1<<7))		// GPB7
#define LED_4			(PORT_B|(1<<8))		// GPB8

#define PORT_F			0x05000000
#define KEY_1			(1<<1)		//GPF1
#define KEY_2			(1<<4)		//GPF4
#define KEY_3			(1<<2)		//GPF2
#define KEY_4			(1<<0)		//GPF0

#define PORT_G			0x06000000
#define SD_CARD_DET		(1<<8)	// SD Card Detect, GPG8


void tq_gpioInit(void);
void tq_gpioSet(DWORD mask);
void tq_gpioClr(DWORD mask);
void tq_gpioSetOutput(DWORD mask);
void tq_gpioSetInput(DWORD mask);
DWORD tq_gpioGet(DWORD mask);


// ==== yl_rtc.c ====================================================
struct tm;
void tq_rtcInit(void);
int tq_rtcGetTime(struct tm* pt);
int tq_rtcSetTime(struct tm* pt);

// ==== yl_lcd.c =====================================================

int tq_lcdcInit(void);
void* tq_lcdGetBufAddr(int* w, int *h);
void tq_lcdcEnvidOnOff(int onoff);
void tq_lcdcPowerEnable(int invpwren,int pwren);
void tq_lcdcBkLtSet(unsigned int HiRatio);

// ==== yl_eint.c =====================================================
int tq_eintInit(void);

// ==== yl_sdc.c =====================================================
#define SD_BLOCK_LENGTH				512

#define SDC_SPEED_100K		100000
#define SDC_SPEED_400K		400000
#define SDC_SPEED_15M		15000000
#define SDC_SPEED_25M		25000000

#define CURRENT_STATE(s)	(s&0x1e00)
#define STATE_IDLE			(0<<9)
#define STATE_READY			(1<<9)
#define STATE_IDENT			(2<<9)
#define STATE_STBY			(3<<9)
#define STATE_TRAN			(4<<9)
#define STATE_DATA			(5<<9)
#define STATE_RCV			(6<<9)
#define STATE_PRG			(7<<9)
#define STATE_DIS			(8<<9)

#define STATUS_OUT_OF_RANGE			(1<<31)
#define STATUS_ADDRESS_ERROR		(1<<30)
#define STATUS_BLOCK_LEN_ERROR		(1<<29)
#define STATUS_ERASE_SEQ_ERROR		(1<<28)
#define STATUS_ERASE_PARAM			(1<<27)
#define STATUS_WP_VIOLATION			(1<<26)
#define STATUS_CARD_IS_LOCKED		(1<<25)
#define STATUS_LOCK_UNLOCK_FAILED	(1<<24)
#define STATUS_COM_CRC_ERROR		(1<<23)
#define STATUS_ILLEGAL_COMMAND		(1<<22)
#define STATUS_CARD_ECC_FAILED		(1<<21)
#define STATUS_CC_ERROR				(1<<20)
#define STATUS_ERROR				(1<<19)
#define STATUS_CSD_OVERWRITE		(1<<16)
#define STATUS_WP_ERASE_SKIP		(1<<15)
#define STATUS_CARD_ECC_DISABLED	(1<<14)
#define STATUS_ERASE_RESET			(1<<13)
#define STATUS_READY_FOR_DATA		(1<<8)
#define STATUS_APP_CMD				(1<<5)
#define STATUS_AKE_SEQ_ERROR		(1<<3)

#define WAIT_4BIT					(1<<4)
#define WAIT_RECEIVE				(1<<3)
#define WAIT_TRANSMIT				(1<<2)
#define WAIT_BUSY					(1<<1)
#define WAIT_AFTER_CMD				(1<<0)

#define SD_OCR_POWER_UP				(1<<31)
#define SD_OCR_CAPACITY				(1<<30)
#define SD_OCR_VDD_3_5				(1<<23)
#define SD_OCR_VDD_3_4				(1<<22)
#define SD_OCR_VDD_3_3				(1<<21)
#define SD_OCR_VDD_3_2				(1<<20)
#define SD_OCR_VDD_3_1				(1<<19)
#define SD_OCR_VDD_3_0				(1<<18)
#define SD_OCR_VDD_2_9				(1<<17)
#define SD_OCR_VDD_2_8				(1<<16)
#define SD_OCR_VDD_2_7				(1<<15)
#define SD_OCR_VDD_ALL				(0x1ff<<15)


void tq_sdcInit(void);
void tq_sdcEnable(void);
void tq_sdcDisable(void);
void tq_sdcSetBusSpeed(int speed);
void tq_sdcSendCmd(DWORD con, DWORD arg);
void tq_sdcWaitNoRsp(void);
int tq_sdcWaitRsp(DWORD *rsp);
int tq_sdcLongRsp(DWORD* rsp);
void tq_sdcPrepareWait(DWORD opt,DWORD cnt);
int tq_sdcWaitEnd(void);
int tq_sdcTransmit(void *buf,DWORD cnt);
int tq_sdcReceive(void *buf,DWORD cnt);

void tq_sdCMD0(void);
int tq_sdCMD1(DWORD arg, DWORD *pOCR);
int tq_sdCMD2(DWORD* pCID);
int tq_sdCMD3(DWORD arg, DWORD* pStatus);
int tq_sdCMD7(DWORD RCA,DWORD *pstatus);
int tq_sdCMD8(void);
int tq_sdCMD9(DWORD RCA,DWORD* pCSD);
int tq_sdCMD10(DWORD RCA,DWORD* pCID);
int tq_sdCMD12(DWORD* pstatus);
int tq_sdCMD13(DWORD RCA, DWORD *pstatus);
int tq_sdCMD16(DWORD length, DWORD* pstatus);
int tq_sdCMD17(DWORD addr, DWORD* pstatus);
int tq_sdCMD18(DWORD addr, DWORD* pstatus);
int tq_sdCMD24(DWORD addr, DWORD* pstatus);
int tq_sdCMD25(DWORD addr, DWORD* pstatus);
int tq_sdACMD6(DWORD RCA, DWORD arg, DWORD *prsp);
int tq_sdACMD41(DWORD RCA, DWORD arg, DWORD *pOCR);

#define UNSTUFF_BITS(resp,start,size) ({ \
	const int __size = size;                                \
	const unsigned int __mask = (__size < 32 ? 1 << __size : 0) - 1; \
	const int __off = 3 - ((start) / 32);                   \
	const int __shft = (start) & 31;                        \
	unsigned int __res;                                              \
															\
	__res = resp[__off] >> __shft;                          \
	if (__size + __shft > 32)                               \
			__res |= resp[__off-1] << ((32 - __shft) % 32); \
	__res & __mask;                                         \
})

typedef struct _MMCSD_CID {
	unsigned int manfid;
	char prod_name[8];
	unsigned int serial;
	unsigned short oemid;
	unsigned short year;
	unsigned char hwrev;
	unsigned char fwrev;
	unsigned char month;
} MMCSD_CID;

typedef struct _MMCSD_CSD {
	unsigned char mmca_vsn;
	unsigned short cmdclass;
	unsigned short tacc_clks;
	unsigned int tacc_ns;
	unsigned int r2w_factor;
	unsigned int max_dtr;
	unsigned int read_blkbits;
	unsigned int write_blkbits;
	unsigned int capacity;
	unsigned int	read_partial :1,
					read_misalign :1,
					write_partial :1,
					write_misalign :1;
} MMCSD_CSD;


#define CARD_TYPE_NONE		0
#define CARD_TYPE_MMC		0x1
#define CARD_TYPE_SD10		0x2
#define CARD_TYPE_SD20		0x3

#define OPMODE_WRITE_PROTECT		(1<<2)
#define OPMODE_4BIT_BUS				(1<<1)
#define OPMODE_1BIT_BUS				(0<<1)
#define OPMODE_BLOCK_ADDRESS		(1<<0)


typedef struct _MMCSD {
	DWORD ocr_vdd;		// operating voltage

	int type;
	DWORD rca;
	DWORD opmode;
	DWORD blcok_length;

	MMCSD_CID cid;
	MMCSD_CSD csd;

	DWORD cyclinders;
	DWORD heads;
	DWORD sectors;

} MMCSD_CARD_T;

int tq_sdWriteBlock(MMCSD_CARD_T *card, BYTE* buf,DWORD blknum,DWORD count);
int tq_sdReadBlock(MMCSD_CARD_T *card, BYTE* buf,DWORD blknum,DWORD count);
int tq_sdIdentify(MMCSD_CARD_T *card);

// ==== tq_i2c.c =====================================================
void tq_i2cInit(void);
void tq_i2cEnable(void);
void tq_i2cDisable(void);
int tq_i2cStart(U8 devid);
int tq_i2cWrite(U8 data);
int tq_i2cWriteStop(void);
int tq_i2cRead(U8 devid,U8 *data);
int tq_i2cReadStop(void);

// ==== tq_adc.c =====================================================
void tq_adcInit(void);
void TS_StartScan(int channel);
int tq_adcGetCurrentChannel(void);
void tq_adcAIN0Handler(void);
int tq_FlashTrigger(int ms);
int tq_adcGetTSData(int *x, int *y);

enum{
	ADC_TOUCHSCREEN=0,
	ADC_AIN0,
	ADC_LOOPBACK,
};

//==== yl_bayerResizeFilter.c =====================================================
#define RESIZE_TRIANGLE 1
#define RESIZE_BOX 2
#define RESIZE_CUBIC 3
#define RESIZE_HERMITE 4
#define RESIZE_QUADRATIC 5
void yl_bayerResizeFilter(int *src,int srcw,int srch,int *dst,int dstw,int dsth,int *temp, int filtermode) ;
//==== yl_motor.c =====================================================
enum {
	ZOOM_WM = 1,
	ZOOM_WT,
	ZOOM_CW,
	ZOOM_CT,
	ZOOM_MT,
	ZOOM_TM,
	ZOOM_MW,
	ZOOM_TW,
	ZOOM_WC,
	ZOOM_TC,
	ZOOM_CTL,
	ZOOM_TCL,
	ZOOM_RESET,
	SHUTTER_OPEN,
	SHUTTER_CLOSE,
	SHUTTER_BIG,
	IRIS_OPEN,
	IRIS_CLOSE,
	FOCUS_CW,
	FOCUS_CCW,
	MOTO_INIT,
	MOTO_RESET,
	MOTO_POWEROFF,
	MOTO_POWERON,
	MOTO_TASK_WAKEUP,
	ZOOM_WC_1,
	ZOOM_CW_1,
	Z_SPEED,
	AF_SPEED,
	Z_POS,
	AF_POS,
	Z_STA,
	AF_STA,
};
typedef struct _SYS_MSG {
	int id;
	int data_1;
	int data_2;
} SYS_MSG;


void tq_MotoInit(void);
void tq_MotoFocus(int mode);
void tq_MotoShutter(int mode);
void tq_MotoZoom(int mode);
void tq_MotoIris(int mode);
void tq_MotoDelay(U32 time);


void tq_PwmSet(U32 mode,U32 timeH,U32 timeL);//unit is 0.04 micro second
void tq_PwmStop(U32 mode);
U32 tq_PwmGetStatus(void);
void tq_PwmStart(U32 mode);


// ==== tq_i2s.c =====================================================
#define S3C2410_IISCON_LRINDEX	  (1<<8)
#define S3C2410_IISCON_TXFIFORDY  (1<<7)
#define S3C2410_IISCON_RXFIFORDY  (1<<6)
#define S3C2410_IISCON_TXDMAEN	  (1<<5)
#define S3C2410_IISCON_RXDMAEN	  (1<<4)
#define S3C2410_IISCON_TXIDLE	  (1<<3)
#define S3C2410_IISCON_RXIDLE	  (1<<2)
#define S3C2410_IISCON_PSCEN	  (1<<1)
#define S3C2410_IISCON_IISEN	  (1<<0)

#define S3C2440_IISMOD_MPLL	  	  (1<<9)
#define S3C2410_IISMOD_SLAVE	  (1<<8)
#define S3C2410_IISMOD_NOXFER	  (0<<6)
#define S3C2410_IISMOD_RXMODE	  (1<<6)
#define S3C2410_IISMOD_TXMODE	  (2<<6)
#define S3C2410_IISMOD_TXRXMODE	  (3<<6)
#define S3C2410_IISMOD_LR_LLOW	  (0<<5)
#define S3C2410_IISMOD_LR_RLOW	  (1<<5)
#define S3C2410_IISMOD_IIS	  	  (0<<4)
#define S3C2410_IISMOD_MSB	  	  (1<<4)
#define S3C2410_IISMOD_8BIT	   	  (0<<3)
#define S3C2410_IISMOD_16BIT	  (1<<3)
#define S3C2410_IISMOD_BITMASK	  (1<<3)
#define S3C2410_IISMOD_256FS	  (0<<2)
#define S3C2410_IISMOD_384FS	  (1<<2)
#define S3C2410_IISMOD_16FS	  	  (0<<0)
#define S3C2410_IISMOD_32FS	  	  (1<<0)
#define S3C2410_IISMOD_48FS	  	  (2<<0)
#define S3C2410_IISMOD_FS_MASK	  (3<<0)

#if 1
int tq_i2s_enable(void);
int tq_i2s_set_sysclk(int clkid);
int tq_i2s_set_clkdiv(int divid, int div);
int tq_i2s_set_fmt(void);
int tq_i2s_hw_params(int format);
int tq_i2s_trigger(int cmd, int stream);

void l3_setmode(int v);
void l3_setdat(int v);
void l3_setclk(int v);
int l3_write(U8 addr, U8 *data, int len);

int tq_uda134x_startup(int dir);
void tq_uda134x_trigger(int cmd, int dir);
int tq_uda134x_hw_params(unsigned long rate, int format);
int tq_uda134x_init(void);
int tq_uda134x_exit(void);

extern U16 audioBuffer[];
#define AUDIO_BUFSIZE	2048		// 2048 samples in 16 bit PCM, 4096 samples in 8 bit PCM...etc

#define S3C24XX_CLKSRC_PCLK		0
#define S3C24XX_CLKSRC_MPLL		1

#define S3C24XX_DIV_MCLK		0
#define S3C24XX_DIV_BCLK		1
#define S3C24XX_DIV_PRESCALER	2

#define PCM_TRIGGER_STOP			0
#define	PCM_TRIGGER_START			1
#define PCM_TRIGGER_PAUSE_PUSH		3
#define PCM_TRIGGER_PAUSE_RELEASE	4
#define PCM_TRIGGER_SUSPEND			5
#define PCM_TRIGGER_RESUME			6

#define PCM_FORMAT_S8			0
#define PCM_FORMAT_U8			1
#define PCM_FORMAT_S16LE		2
#define PCM_FORMAT_S16BE		3
#define PCM_FORMAT_U16LE		4
#define PCM_FORMAT_U16BE		5
#define PCM_FORMAT_S24LE		6
#define PCM_FORMAT_S24BE		7
#define PCM_FORMAT_U24LE		8
#define PCM_FORMAT_U24BE		9
#define PCM_FORMAT_S32LE		10
#define PCM_FORMAT_S32BE		11
#define PCM_FORMAT_U32LE		12
#define PCM_FORMAT_U32BE		13

#define PCM_FORMAT_FLOAT_LE		14
#define PCM_FORMAT_FLOAT_BE		15



enum {
        PCM_STREAM_PLAYBACK = 0,
        PCM_STREAM_CAPTURE,
        PCM_STREAM_LAST = PCM_STREAM_CAPTURE,
};
#endif
/* TODO: check if necessary, added by ccchiu */


#define S3C2410_IISPSR			(0x08)
#define S3C2410_IISPSR_INTMASK	(31<<5)
#define S3C2410_IISPSR_INTSHIFT	(5)
#define S3C2410_IISPSR_EXTMASK	(31<<0)
#define S3C2410_IISPSR_EXTSHFIT	(0)

#define S3C2410_IISFCON_TXDMA	  (1<<15)
#define S3C2410_IISFCON_RXDMA	  (1<<14)
#define S3C2410_IISFCON_TXENABLE  (1<<13)
#define S3C2410_IISFCON_RXENABLE  (1<<12)
#define S3C2410_IISFCON_TXMASK	  (0x3f << 6)
#define S3C2410_IISFCON_TXSHIFT	  (6)
#define S3C2410_IISFCON_RXMASK	  (0x3f)
#define S3C2410_IISFCON_RXSHIFT	  (0)

#define S3C2400_IISFCON_TXDMA     (1<<11)
#define S3C2400_IISFCON_RXDMA     (1<<10)
#define S3C2400_IISFCON_TXENABLE  (1<<9)
#define S3C2400_IISFCON_RXENABLE  (1<<8)
#define S3C2400_IISFCON_TXMASK	  (0x07 << 4)
#define S3C2400_IISFCON_TXSHIFT	  (4)
#define S3C2400_IISFCON_RXMASK	  (0x07)
#define S3C2400_IISFCON_RXSHIFT	  (0)


#endif /* TQ_BSP_H_ */
