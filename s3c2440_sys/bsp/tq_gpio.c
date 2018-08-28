/*
 * tq_gpio.c
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */
#include <platform/yl_type.h>
#include "2440addr.h"
#include "tq_hw_defs.h"

// value port
#define B1(p,v) (v<<(p))
// control port
#define B2(p,v) (v<<((p)<<1))
#define B4(p,v) (v<<((p)<<2))

void tq_gpioInit(void) {

	// enable gpio controller here, many io pins depend on it, eg. uart...
	BIT_SET(rCLKCON,CLK_CON_GPIO);

    //CAUTION:Follow the configuration order for setting the ports.
    // 1) setting value(GPnDAT)
    // 2) setting control register  (GPnCON)
    // 3) configure pull-up resistor(GPnUP)

    //32bit data bus configuration
    //*** PORT A GROUP
    //Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12
    //Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1
    //Binary :  1     1      1  , 1   1   1    1   ,  1     1     1     1
    //Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
    //Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0
    //Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1
    rGPACON = 0x7fffff;

    //**** PORT B GROUP
    //Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
    //Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
    //Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT
    //Binary :   00  ,  01       00  ,   01      00   ,  01       01  ,   01     01   ,  01        01
    //rGPBCON = 0x000150;(youlong)
    rGPBDAT |=0x1e0;	// turn off 4 LEDs, GPB5-GPB8
    rGPBCON = 0x015550;
    rGPBUP  = 0x7ff;     // The pull up function is disabled GPB[10:0]

    //*** PORT C GROUP for youlong
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND
    //Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
    //rGPCCON = 0xaaaaaaaa;
    //rGPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0]

    //*** PORT C GROUP
    //Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
    //Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND
    //Binary :  10   10  , 10    10  , 10    10  , 10   01  , 01     01  ,  01   10 , 10     10 , 10   10
    rGPCCON = 0xaaa956aa;
    rGPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0]

    //*** PORT D GROUP
    //Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
    //Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
    //Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
    rGPDCON = 0xaaaaaaaa;
    rGPDUP  = 0xffff;     // The pull up function is disabled GPD[15:0]

    //*** PORT E GROUP
    //Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4
    //Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK IN
    //Binary :  10     10  ,  10      10  ,  10      10   ,  10      10   ,   10    10  , 10     00  ,
    //-------------------------------------------------------------------------------------------------------
    //Ports  :  GPE3   GPE2  GPE1    GPE0
    //Signal :  IN     IN    IN      IN
    //Binary :  00     00  ,  00      00
//    rGPECON = 0xaaaaaaaa;
    //rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]
	rGPECON = 0xa02aa800; // For added AC97 setting
    rGPEUP  = 0xf83f;	// pull up for SD cmd & dat pins

     //*** PORT F GROUP
    //Ports  : GPF7      GPF6      GPF5      GPF4      GPF3      GPF2      GPF1      GPF0
    //  0    : Input     Input     Input     Input     Input     Input     Input     Input
    //  1    : Output    Output    Output    Output    Output    Output    Output    Output
    //  2    : EINT7     EINT6     EINT5     EINT4     EINT3     EINT2     EINT1     EINT0
    //  3    : Reserved  Reserved  Reserved  Reserved  Reserved  Reserved  Reserved  Reserved
    //config : EINT7     x         x         key2      x         key3      key1      key4
    rGPFCON =  B2( 7,2)| B2( 6,1)| B2( 5,1)| B2( 4,2)| B2( 3,1)| B2( 2,2)| B2( 1,2)| B2( 0,2);
    rGPFUP  =  B1( 7,1)| B1( 6,1)| B1( 5,1)| B1( 4,1)| B1( 3,1)| B1( 2,1)| B1( 1,1)| B1( 0,1);


    //*** PORT G GROUP
    //Ports  : GPG15     GPG14     GPG13     GPG12     GPG11     GPG10     GPG9      GPG8
    //  0    : Input     Input     Input     Input     Input     Input     Input     Input
    //  1    : Output    Output    Output    Output    Output    Output    Output    Output
    //  2    : EINT23    EINT22    EINT21    EINT20    EINT19    EINT18    EINT17    EINT16
    //  3    : Reserved  Reserved  Reserved  Reserved  TCLK[1]   nCTS1     nRTS1     Reserved
    //config : x         x         x         x         x         x         x         card det
    //-----------------------------------------------------------------------------------------
    //Ports  : GPG7      GPG6      GPG5      GPG4      GPG3      GPG2      GPG1      GPG0
    //  0    : Input     Input     Input     Input     Input     Input     Input     Input
    //  1    : Output    Output    Output    Output    Output    Output    Output    Output
    //  2    : EINT15    EINT14    EINT13    EINT12    EINT11    EINT10    EINT09    EINT08
    //  3    : SPICLK1   SPIMOSI1  SPIMISO1  LCD_PWRDN nSS1      nSS0      Reserved  Reserved
    //config : x         x         x         lcd_en    x         x         x         x
    rGPGCON =  B2(15,0)| B2(14,0)| B2(13,0)| B2(12,0)| B2(11,0)| B2(10,0)| B2( 9,0)| B2( 8,0)
			|  B2( 7,0)| B2( 6,0)| B2( 5,0)| B2( 4,3)| B2( 3,0)| B2( 2,0)| B2( 1,0)| B2( 0,0);

    rGPGUP  =  B1(15,1)| B1(14,1)| B1(13,1)| B1(12,1)| B1(11,1)| B1(10,1)| B1( 9,1)| B1( 8,1)
			|  B1( 7,1)| B1( 6,1)| B1( 5,1)| B1( 4,1)| B1( 3,1)| B1( 2,1)| B1( 1,1)| B1( 0,1);
    /* NOTE:
    Card detect, EINT16 can not be used as interrupt,
    because the SD card jack mechanism, that causes
    hi and lo edge interrupts when poll out card
    */




    //*** PORT H GROUP
    //Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0
    //Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
    //Binary :   10   ,  10     10 , 11    11  , 10   10 , 10   10 , 10    10
    rGPHCON = 0x00faaa;
    rGPHUP  = 0x7ff;    // The pull up function is disabled GPH[10:0]

	// Added for S3C2440X, DonGo
	//*** PORT J GROUP
    //Ports  : GPJ12   GPJ11       GPJ10    GPJ9    GPJ8      GPJ7      GPJ6      GPJ5      GPJ4      GPJ3      GPJ2      GPJ1     GPJ0
    //Signal : CAMRESET CAMPCLKOUT CAMHREF CAMVSYNC CAMPCLKIN CAMDAT[7] CAMDAT[6] CAMDAT[5] CAMDAT[4] CAMDAT[3] CAMDAT[2] CAMDAT[1] CAMDAT[0]
    //Binary :   10      10       10        10       10        10        10        10       10         10        10        10      10
    rGPJCON = 0x02aaaaaa;
    rGPJUP  = 0x1fff;    // The pull up function is disabled GPH[10:0]

    //    rEXTINT0 = 0x22222222;
    //    rEXTINT1 = 0x22222222;
    //    rEXTINT2 = 0x22222222;
    // EINT[7:0]
    rEXTINT0=  B4( 7,2)| B4( 6,2)| B4( 5,2)| B4( 4,2)| B4( 3,2)| B4( 2,2)| B4( 1,2)| B4( 0,2);
    // EINT[15:8]
    rEXTINT1=  B4( 7,2)| B4( 6,2)| B4( 5,2)| B4( 4,2)| B4( 3,2)| B4( 2,2)| B4( 1,2)| B4( 0,2);
    // EINT[23:16]
    rEXTINT2=  B4( 7,2)| B4( 6,2)| B4( 5,2)| B4( 4,2)| B4( 3,2)| B4( 2,2)| B4( 1,2)| B4( 0,2);

}


#define GPCON(p)    (*(volatile unsigned *)(0x56000000+(p*0x10)))
#define GPDAT(p)    (*(volatile unsigned *)(0x56000004+((p)<<4)))
// mask  PMMMMMMM --> 32bit DWORD
// P:[31:24] port number
// M:[23:00] bit mask
void tq_gpioSet(DWORD mask) {
DWORD p=mask>>24;
	if (p&0x10) p+=5;
	BIT_SET(GPDAT(p),mask&0x00ffffff);
}


void tq_gpioClr(DWORD mask) {
DWORD p=mask>>24;
	if (p&0x10) p+=5;
	BIT_CLR(GPDAT(p),mask&0x00ffffff);
}


DWORD tq_gpioGet(DWORD mask){
	DWORD p=mask>>24;
	if (p&0x10)
		p+=5;
	return BIT_VAL(GPDAT(p),mask&0x00ffffff);
}


#if 0
void tq_gpioSetOutput(DWORD mask) {
DWORD p=mask>>24;
DWORD m;

	if (p&0x10) p+=5;
	if (!p) { // GPA
		mask=(~mask)&0x00ffffff;
	}
	else { // GPB - GPJ
		m=mask&0x0000ffff;
		__asm__ (
			"mov r1,#0;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000003;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x0000000c;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000030;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x000000c0;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000300;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000c00;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00003000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x0000c000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00030000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x000c0000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00300000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00c00000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x03000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x0c000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x30000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0xc0000000;"

			"mov %0,r1;"

			: "=r"(mask)
			: "r"(m)
			: "r1","cc"
		);
	}
	m=GPCON(p);
	GPCON(p)=m&mask;
}


void tq_gpioSetInput(DWORD mask) {
DWORD p=mask>>24;
DWORD m;

	if (p&0x10) p+=5;
	if (!p) { // GPA
		mask=mask&0x00ffffff;
	}
	else { // GPB - GPJ
		m=mask&0x0000ffff;
		__asm__ (
			"sub r1,r1,r1;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000001;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000004;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000010;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000040;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000100;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00000400;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00001000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00004000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00010000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00040000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00100000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x00400000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x01000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x04000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x10000000;"

			"mov %0,%0,lsr #1;"
			"orrcc r1,r1,#0x40000000;"

			"mov %0,r1;"

			: "=r"(mask)
			: "r"(m)
			: "r1","cc"
		);
	}
	m=GPCON(p)&mask;
	GPCON(p)=m|=mask;
}
#endif



