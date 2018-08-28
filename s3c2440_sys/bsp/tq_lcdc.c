#include "platform/TQ_BSP.h"
#include "platform/yl_sys.h"
#include "tq_hw_defs.h"
#include "2440addr.h"

static void *g_lcdFrameBuffer=0;

void tq_lcdcBkLtSet(unsigned int HiRatio) {
#define FREQ_PWM1		1000

	if(!HiRatio)
	{
		rGPBCON  = rGPBCON & ( (~(3<<2)) | (1<<2) );	//GPB1设置为output
		rGPBDAT &= ~(1<<1);
		return;
	}
	rGPBCON = rGPBCON & ( (~(3<<2)) | (2<<2) );		//GPB1设置为TOUT1

	if( HiRatio > 100 )
		HiRatio = 100 ;

	rTCON = rTCON & (~(0xf<<8)) ;			// clear manual update bit, stop Timer1

	rTCFG0 	&= 0xffffff00;					// set Timer 0&1 prescaler 0
	rTCFG0 |= 15;							//prescaler = 15+1

	rTCFG1 	&= 0xffffff0f;					// set Timer 1 MUX 1/16
	rTCFG1  |= 0x00000030;					// set Timer 1 MUX 1/16

	rTCNTB1	 = ( 100000000>>8 )/FREQ_PWM1;		//if set inverter off, when TCNT2<=TCMP2, TOUT is high, TCNT2>TCMP2, TOUT is low
	rTCMPB1  = ( rTCNTB1*(100-HiRatio))/100 ;	//if set inverter on,  when TCNT2<=TCMP2, TOUT is low,  TCNT2>TCMP2, TOUT is high

	rTCON = rTCON & ( (~(0xf<<8)) | (0x0e<<8) );
	//自动重装,输出取反关闭,更新TCNTBn、TCMPBn,死区控制器关闭
	rTCON = rTCON & ( (~(0xf<<8)) | (0x0d<<8) );		//开启背光控制
}


void tq_lcdcPowerEnable(int invpwren,int pwren)
{
	//GPG4 is setted as LCD_PWREN
	rGPGUP=rGPGUP& ( (~(1<<4))|(1<<4) ); // Pull-up disable
	rGPGCON=rGPGCON& ( (~(3<<8))|(3<<8) ); //GPG4=LCD_PWREN
	rGPGDAT = rGPGDAT | (1<<4) ;
	invpwren=pwren;
	//Enable LCD POWER ENABLE Function
	rLCDCON5=rLCDCON5& ( (~(1<<3))|(pwren<<3) );   // PWREN
	rLCDCON5=rLCDCON5& ( (~(1<<5))|(invpwren<<5) );   // INVPWREN
}


void tq_lcdcEnvidOnOff(int onoff) {
	if(onoff==1)
		rLCDCON1|=1; // ENVID=ON
	else
		rLCDCON1 =rLCDCON1 & 0x3fffe; // ENVID Off
}


#define SCR_XSIZE_TFT LCD_WIDTH
#define FB_SIZE	(LCD_WIDTH*LCD_HEIGHT*4)
int tq_lcdcInit(void) {

	g_lcdFrameBuffer=malloc(FB_SIZE);
	if (!g_lcdFrameBuffer) return -1;
	BIT_SET(rCLKCON,CLK_CON_LCDC);

	rGPCUP  = 0x00000000;
	rGPCCON = 0xaaaa02a9;

	rGPDUP  = 0x00000000;
	rGPDCON=0xaaaaaaaa; //Initialize VD[15:8]

	rLCDCON1=(CLKVAL_TFT<<8)|(MVAL_USED<<7)|(PNRMODE<<5)|(BPPMODE<<1)|0;
	rLCDCON2=(VBPD<<24)|(LINEVAL_TFT<<14)|(VFPD<<6)|(VSPW);
	rLCDCON3=(HBPD<<19)|(HOZVAL_TFT<<8)|(HFPD);
	rLCDCON4=(MVAL<<8)|(HSPW);
	rLCDCON5 = (FRM565<<11) | (INVVLINE<<9) | (INVVFRAME<<8) | (PWREN<<3)  |(BSWP<<1) | (HWSWP);

	rLCDSADDR1=( ( (DWORD)g_lcdFrameBuffer>>22 ) <<21 )|( ( (DWORD)g_lcdFrameBuffer>>1 ) &0x1fffff );
	rLCDSADDR2=( ((DWORD)g_lcdFrameBuffer+FB_SIZE)>>1 )&0x1fffff;
//	rLCDSADDR3=(((SCR_XSIZE_TFT-LCD_XSIZE_TFT)/1)<<11)|(LCD_XSIZE_TFT/1);
	rLCDSADDR3=(((SCR_XSIZE_TFT-LCD_WIDTH)*32/16)<<11)|(LCD_WIDTH*32/16);
	rLCDINTMSK|=(3); // MASK LCD Sub Interrupt
	rTCONSEL &= (~7) ;     // Disable LPC3480
	rTPAL=0; // Disable Temp Palette
	return 0;
}


void* tq_lcdGetBufAddr(int* w, int *h) {
	if (w) *w=LCD_WIDTH;
	if (h) *h=LCD_HEIGHT;
	return g_lcdFrameBuffer;
}

