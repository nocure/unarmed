/*
 * tq_hw_defs.h
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */

#ifndef TQ_HW_DEFS_H_
#define TQ_HW_DEFS_H_

// ==== CLOCK =======================
#define CLK_CON_AC97				(1<<20)
#define CLK_CON_CAMERA				(1<<19)
#define CLK_CON_SPI					(1<<18)
#define CLK_CON_IIS					(1<<17)
#define CLK_CON_IIC					(1<<16)
#define CLK_CON_ADC					(1<<15)
#define CLK_CON_RTC					(1<<14)
#define CLK_CON_GPIO				(1<<13)
#define CLK_CON_UART2				(1<<12)
#define CLK_CON_UART1				(1<<11)
#define CLK_CON_UART0				(1<<10)
#define CLK_CON_SDI					(1<<9)
#define CLK_CON_PWMTIMER			(1<<8)
#define CLK_CON_USBD				(1<<7)
#define CLK_CON_USBH				(1<<6)
#define CLK_CON_LCDC				(1<<5)
#define CLK_CON_NFC					(1<<4)
#define CLK_CON_SLEEP				(1<<3)
#define CLK_CON_IDLE				(1<<2)

// ==== Interrupts ==================

#define INT_ADC			31
#define INT_RTC			30
#define INT_SPI1		29
#define INT_UART0		28
#define INT_IIC			27
#define INT_USBH		26
#define INT_USBD		25
#define INT_NFCON		24
#define INT_UART1		23
#define INT_SPI0		22
#define INT_SDI			21
#define INT_DMA3		20
#define INT_DMA2		19
#define INT_DMA1		18
#define INT_DMA0		17
#define INT_LCD			16
#define INT_UART2		15
#define INT_TIMER4		14
#define INT_TIMER3		13
#define INT_TIMER2		12
#define INT_TIMER1		11
#define INT_TIMER0		10
#define INT_WDT_AC97	9
#define INT_TICK		8
#define nBATT_FLT		7
#define INT_CAM			6
#define EINT8_23		5
#define EINT4_7			4
#define	EINT3			3
#define	EINT2			2
#define	EINT1			1
#define EINT0			0

// ==== UART ========================

#define UART_TRSTAT_DR			(1<<0)

#define UART_ERSTAT_BR			(1<<3)
#define UART_ERSTAT_FE			(1<<2)
#define UART_ERSTAT_PE			(1<<1)
#define UART_ERSTAT_OV			(1<<0)

#define UART_UFSTAT_TX_FULL		(1<<14)

// ==== LCDC =========================
#if 1
// LTM035A776C 240x320
#define LCD_WIDTH	240
#define LCD_HEIGHT	320

#define CLKVAL_TFT 	(4)
#define MVAL_USED 	(0)			// MMODE 0=each frame   1=rate by MVAL
#define PNRMODE		(3)			// TFTÆÁ
#define BPPMODE		(0x0d)		// 0x0c: 16bpp, 0x0d: 24bpp

#define VFPD 		(5)			// Vertical front porch
#define VSPW 		(1)			// Vertical sync pulse width
#define VBPD 		(0)	//(1)	// Vertical back porch

#define HFPD 		(4)			// Horizontal front porch
#define HSPW 		(8)			// Horizontal sync pulse width
#define HBPD 		(6)	//(2)	// Horizontal back porch

#define HOZVAL_TFT	(LCD_WIDTH-1)
#define LINEVAL_TFT	(LCD_HEIGHT-1)

#define MVAL		(0x0d)

#define HWSWP		0
#define BSWP		0
#define ENLEND		0
#define PWREN		1
#define INVLEND		0
#define INVPWREN	0
#define INVVDEN		0			// 0=normal       1=inverted
#define INVVD		0
#define INVVFRAME	1
#define INVVLINE	1
#define INVVCLK		0
#define FRM565		1
#define BPP24BL		0
#else
//WXCAT35-TG3 320x240
#define LCD_WIDTH	240
#define LCD_HEIGHT	320

#define CLKVAL_TFT 	(4)
#define MVAL_USED 	(0)			// MMODE 0=each frame   1=rate by MVAL
#define PNRMODE		(3)			// TFTÆÁ
#define BPPMODE		(0x0d)		// 0x0c: 16bpp, 0x0d: 24bpp

#define VFPD 		(12)			// Vertical front porch
#define VSPW 		(3)			// Vertical sync pulse width
#define VBPD 		(15)	//(1)	// Vertical back porch

#define HFPD 		(20)			// Horizontal front porch
#define HSPW 		(30)			// Horizontal sync pulse width
#define HBPD 		(38)	//(2)	// Horizontal back porch

#define HOZVAL_TFT	(LCD_WIDTH-1)
#define LINEVAL_TFT	(LCD_HEIGHT-1)

#define MVAL		(0x0d)

#define HWSWP		0
#define BSWP		0
#define ENLEND		0
#define PWREN		1
#define INVLEND		0
#define INVPWREN	0
#define INVVDEN		0			// 0=normal       1=inverted
#define INVVD		0
#define INVVFRAME	1
#define INVVLINE	1
#define INVVCLK		0
#define FRM565		1
#define BPP24BL		0


#endif

// ==== SDC =========================

#define SDC_CMD_START_BIT		0x40


#define SDC_CMD_ABORT			(1<<12)
#define SDC_CMD_WITH_DATA		(1<<11)
#define SDC_CMD_LONG_RSP		(3<<9)	// 136bit long response
#define SDC_CMD_WAIT_RSP		(1<<9)
#define SDC_CMD_START			(1<<8)	// start sending, clear automaticaly

#define SDC_CON_RESET			(1<<8)
#define SDC_CON_BYTE_ORDER_B	(1<<4)
#define SDC_CON_SDIO_INT		(1<<3)
#define SDC_CON_READ_WAIT		(1<<2)
#define SDC_CON_ENABLE			(1<<0)

#define SDC_CSTA_CRC_FAIL		(1<<12)
#define SDC_CSTA_CMD_SENT		(1<<11)
#define SDC_CSTA_CMD_TIMEOUT	(1<<10)
#define SDC_CSTA_RSP_END		(1<<9)
#define SDC_CSTA_IN_PROGRESS	(1<<8)


#define SDC_DCON_BURST4_ENABLE		(0<<24)
#define SDC_DCON_1BYTE_TRANSFER		(0<<22)
#define SDC_DCON_2BYTE_TRANSFER		(1<<22)
#define SDC_DCON_4BYTE_TRANSFER		(2<<22)
#define SDC_DCON_PERIOD_TYPE		(1<<21)
#define SDC_DCON_TX_AFTER_RSP		(1<<20)
#define SDC_DCON_RX_AFTER_CMD		(1<<19)
#define SDC_DCON_BUSY_AFTER_CMD		(1<<18)
#define SDC_DCON_BLOCK_MODE			(1<<17)
#define SDC_DCON_WIDE_BUS			(1<<16)
#define SDC_DCON_DMA_ENABLE			(1<<15)
#define SDC_DCON_TRANSFER_START		(1<<14)
#define SDC_DCON_TRANSMIT			(3<<12)
#define SDC_DCON_RECEIVE			(2<<12)
#define SDC_DCON_BUSY_CHECK			(1<<12)

#define SDC_DSTA_NO_BUSY			(1<<11)
#define SDC_DSTA_READ_WAIT			(1<<10)
#define SDC_DSTA_SDIO_INT			(1<<9)
#define SDC_DSTA_TX_CRC_FAIL		(1<<7)
#define SDC_DSTA_RX_CRC_FAIL		(1<<6)
#define SDC_DSTA_DATA_TIMEOUT		(1<<5)
#define SDC_DSTA_DATA_FINISH		(1<<4)
#define SDC_DSTA_BUSY_FINISH		(1<<3)
#define SDC_DSTA_TX_IN_PROGRESS		(1<<1)
#define SDC_DSTA_RX_IN_PROGRESS		(1<<0)

#define SDC_FSTA_FIFO_RESET			(1<<16)
#define SDC_FSTA_FIFO_FAIL			(3<<14)
#define SDC_FSTA_TX_READY			(1<<13)
#define SDC_FSTA_RX_READY			(1<<12)

#define SDI_INT_NoBusy				(1<<18)
#define SDI_INT_RspCrc				(1<<17)
#define SDI_INT_CmdSent				(1<<16)
#define SDI_INT_CmdTout				(1<<15)
#define SDI_INT_RspEnd				(1<<14)
#define SDI_INT_RWaitReq			(1<<13)
#define SDI_INT_IOIntDet			(1<<12)
#define SDI_INT_FFfail				(1<<11)
#define SDI_INT_CrcSta				(1<<10)
#define SDI_INT_DatCrc				(1<<9)
#define SDI_INT_DatTout				(1<<8)
#define SDI_INT_DatFin				(1<<7)
#define SDI_INT_BusyFin				(1<<6)
#define SDI_INT_TFHalf				(1<<4)
#define SDI_INT_TFEmpty				(1<<3)
#define SDI_INT_RFLast				(1<<2)
#define SDI_INT_RFFull				(1<<1)
#define SDI_INT_RFHalf				(1<<0)

#endif /* TQ_HW_DEFS_H_ */
