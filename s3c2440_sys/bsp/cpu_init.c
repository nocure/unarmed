/*
 * Board_Init.c
 *
 *  Created on: 2008/12/6
 *      Author: ww
 */

#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include "mmu.h"

U32 CPU_FREQ;
U32 FCLK;
U32 HCLK;
U32 PCLK;
U32 UCLK;


void Clk0_Enable(int clock_sel)
{	// 0:MPLLin, 1:UPLL, 2:FCLK, 3:HCLK, 4:PCLK, 5:DCLK0
	rMISCCR = (rMISCCR&~(7<<4)) | (clock_sel<<4);
	rGPHCON = (rGPHCON&~(3<<18)) | (2<<18);
}
void Clk1_Enable(int clock_sel)
{	// 0:MPLLout, 1:UPLL, 2:RTC, 3:HCLK, 4:PCLK, 5:DCLK1
	rMISCCR = (rMISCCR&~(7<<8)) | (clock_sel<<8);
	rGPHCON = (rGPHCON&~(3<<20)) | (2<<20);
}
void Clk0_Disable(void) {
	rGPHCON = rGPHCON&~(3<<18);	// GPH9 Input
}
void Clk1_Disable(void) {
	rGPHCON = rGPHCON&~(3<<20);	// GPH10 Input
}


//*************************[ MPLL ]*******************************
void ChangeMPllValue(int mdiv,int pdiv,int sdiv)
{
    rMPLLCON = (mdiv<<12) | (pdiv<<4) | sdiv;
}


//************************[ HCLK, PCLK ]***************************
/*
// for 2410.
void ChangeClockDivider(int hdivn,int pdivn)
{
     // hdivn,pdivn FCLK:HCLK:PCLK
     //     0,0         1:1:1
     //     0,1         1:1:2
     //     1,0         1:2:2
     //     1,1         1:2:4
    rCLKDIVN = (hdivn<<1) | pdivn;

    if(hdivn)
        MMU_SetAsyncBusMode();
    else
        MMU_SetFastBusMode();
}
*/
// Modified for 2440.
void ChangeClockDivider(int hdivn_val,int pdivn_val)
{
	int hdivn=2, pdivn=0;

     // hdivn_val (FCLK:HCLK)ratio hdivn
     // 11           1:1       (0)
     // 12           1:2       (1)
     // 13           1:3       (3)
     // 14           1:4       (2)
     // pdivn_val (HCLK:PCLK)ratio pdivn
     // 11           1:1       (0)
     // 12           1:2       (1)
	switch(hdivn_val) {
		case 11: hdivn=0; break;
		case 12: hdivn=1; break;
		case 13:
		case 16: hdivn=3; break;
		case 14:
		case 18: hdivn=2; break;
	}

	switch(pdivn_val) {
		case 11: pdivn=0; break;
		case 12: pdivn=1; break;
	}

	//Uart_Printf("Clock division change [hdiv:%x, pdiv:%x]\n", hdivn, pdivn);
	rCLKDIVN = (hdivn<<1) | pdivn;

	switch(hdivn_val) {
		case 16:		// when 1, HCLK=FCLK/8.
			rCAMDIVN = (rCAMDIVN & ~(3<<8)) | (1<<8);
		break;
		case 18: 	// when 1, HCLK=FCLK/6.
			rCAMDIVN = (rCAMDIVN & ~(3<<8)) | (1<<9);
		break;
	}

    if(hdivn!=0)
        MMU_SetAsyncBusMode();
    else
        MMU_SetFastBusMode();
}


static void cal_cpu_bus_clk(void)
{
	U32 val;
	U8 m, p, s;
	U32 UPLL;

	val = rMPLLCON;
	m = (val>>12)&0xff;
	p = (val>>4)&0x3f;
	s = val&3;

	//(m+8)*FIN*2 不要超出32位数!
	FCLK = ((m+8)*(FIN/100)*2)/((p+2)*(1<<s))*100;

	val = rCLKDIVN;
	m = (val>>1)&3;
	p = val&1;
	val = rCAMDIVN;
	s = val>>8;

	switch (m) {
	case 0:
		HCLK = FCLK;
		break;
	case 1:
		HCLK = FCLK>>1;
		break;
	case 2:
		if(s&2)
			HCLK = FCLK>>3;
		else
			HCLK = FCLK>>2;
		break;
	case 3:
		if(s&1)
			HCLK = FCLK/6;
		else
			HCLK = FCLK/3;
		break;
	}

	if(p)
		PCLK = HCLK>>1;
	else
		PCLK = HCLK;

	if(s&0x10)
		CPU_FREQ = HCLK;
	else
		CPU_FREQ = FCLK;

	val = rUPLLCON;
	m = (val>>12)&0xff;
	p = (val>>4)&0x3f;
	s = val&3;
	UPLL = ((m+8)*FIN)/((p+2)*(1<<s));
	UCLK = (rCLKDIVN&8)?(UPLL>>1):UPLL;
}


void clock_init(void) {
int i;
U8 key;
U32 mpll_val = 0 ;

	i = 2 ;	//don't use 100M!
	switch ( i ) {
	case 0:	//200
		key = 12;
		mpll_val = (92<<12)|(4<<4)|(1);
		break;
	case 1:	//300
		key = 13;
		mpll_val = (67<<12)|(1<<4)|(1);
		break;
	case 2:	//400
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	case 3:	//440!!!
		key = 14;
		mpll_val = (102<<12)|(1<<4)|(1);
		break;
	default:
		key = 14;
		mpll_val = (92<<12)|(1<<4)|(1);
		break;
	}

	//init FCLK=400M, so change MPLL first
	ChangeMPllValue((mpll_val>>12)&0xff, (mpll_val>>4)&0x3f, mpll_val&3);
	ChangeClockDivider(key, 12);
	cal_cpu_bus_clk();

}


void cpu_init(void) {

	clock_init();
	MMU_Init();
	Clk0_Disable();
	Clk1_Disable();

}

