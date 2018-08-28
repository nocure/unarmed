/*
 * irq_vector_table.S
 *
 *  Created on: 2018¦~8¤ë22¤é
 *      Author: ww
 */


.global __isr_do_nothing
__isr_do_nothing:
	mov pc,lr

.global __irq_vector_table
__irq_vector_table:
	.word __isr_do_nothing				//  0 EINT0
	.word __isr_do_nothing				//  1 EINT1
	.word __isr_do_nothing				//  2 EINT2
	.word __isr_do_nothing				//  3 EINT3
	.word __isr_do_nothing				//  4 EINT4_7
	.word __isr_do_nothing				//  5 EINT8_23
	.word __isr_do_nothing				//  6 CAM
	.word __isr_do_nothing				//  7 nBATT_FLT
	.word __isr_do_nothing				//  8 TICK
	.word __isr_do_nothing				//  9 WDT_AC97
	.word __isr_do_nothing				// 10 TIMER0
	.word __isr_do_nothing				// 11 TIMER1
	.word __isr_do_nothing				// 12 TIMER2
	.word __isr_do_nothing				// 13 TIMER3
	.word __isr_do_nothing				// 14 TIMER4
	.word __isr_do_nothing				// 15 UART2
	.word __isr_do_nothing				// 16 LCD
	.word __isr_do_nothing				// 17 DMA0
	.word __isr_do_nothing				// 18 DMA1
	.word __isr_do_nothing				// 19 DMA2
	.word __isr_do_nothing				// 20 DMA3
	.word __isr_do_nothing				// 21 SDI
	.word __isr_do_nothing				// 22 SPI0
	.word __isr_do_nothing				// 23 UART1
	.word __isr_do_nothing				// 24 NFCON
	.word __isr_do_nothing				// 25 USBD
	.word __isr_do_nothing				// 26 USBH
	.word __isr_do_nothing				// 27 IIC
	.word __isr_do_nothing				// 28 UART0
	.word __isr_do_nothing				// 29 SPI1
	.word __isr_do_nothing				// 30 RTC
	.word __isr_do_nothing				// 31 ADC


