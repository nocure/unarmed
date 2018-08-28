/*
 * dib7000.c
 *
 *  Created on: 2009/11/18
 *      Author: Chris Chiu
 */

#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "dib7kreg.h"
#include "dib7000.h"

BYTE DIB7KDefaultAddress[2] = {0x12, 0x12};
BYTE DIB7KAddress[2] = {0x80, 0x82};

extern float RF;

BYTE _tps[2];
BYTE _reset[2];
long  reset_cnt;
BYTE rev;

extern float _freq[10];
extern int   _bw[10];
extern int   _IQ[10];
extern int   freq_all;

/* Function Declaration */
static int DIB7KWriteReg(BYTE wDevice, DWORD wReg, DWORD wVal);
static int DIB7KReadReg(BYTE wDevice, DWORD wReg, DWORD *rVal);
static void DIB7KDelay(DWORD DelaySec);
static int DIB7KSetTuneCfg(BYTE Device, struct DIBDVBChannel *CD);
static int DIB7KTuneChannel(BYTE Device);
static int DIB7KAutoSearchStart(BYTE Device, struct DIBDVBChannel *CD);
static int DIB7KAutoSearchIsIrq(BYTE *Device,BYTE *LockAddr,BYTE NumOfDemod);
static int DIB7KDvbtGetChannelDescription(BYTE Device, struct DIBDVBChannel *CD);
static int DIB7KSetScanCfg(BYTE Device, struct DIBDVBChannel *CD);
static int DIB7KSetPowerLevel(BYTE Device, BYTE PowerLevel);
static int DIB7KSetAdcCfg(BYTE Device,BYTE CfgNo);
static int DIB7KPllClkCfg(BYTE Device);
static int DIB7KSadCalib(BYTE Device);
static int DIB7KRestartAgc(BYTE Device);
static int DIB7KRestartSearch(BYTE Device);
static int DIB7KAgcSoftSplit(BYTE Device);
static int DIB7KRestart(BYTE Device);
static void DIB7KDumpDvbtChannelParams(struct DIBDVBChannel *CD);


/**************************************************\
\**************************************************/
static int DIB7KWriteReg(BYTE wDevice, DWORD wReg, DWORD wVal)
{
    int   ret = SUCCESS;
    BYTE Byte[4];
    BYTE Cnt;

	if(tq_i2cStart(wDevice)) {
		return FAILED;
	}

    Byte[0] = wReg >> 8;
    Byte[1] = wReg & 0x00FF;
    Byte[2] = wVal >> 8;
    Byte[3] = wVal & 0x00FF;

    for(Cnt=0; Cnt<4; Cnt++)
    {
        ret = tq_i2cWrite(Byte[Cnt]);
        if(ret)    return ret;
    }

	if(tq_i2cWriteStop())
		return FAILED;

    return ret;
}

/**************************************************\
\**************************************************/
static int DIB7KReadReg(BYTE wDevice, DWORD wReg, DWORD *rVal)
{
    int   ret = SUCCESS;
    BYTE Byte[2];
    BYTE Cnt;

	if(tq_i2cStart(wDevice)) {
		return FAILED;
	}

    Byte[0] = wReg >> 8;
    Byte[1] = wReg & 0x00FF;

    for(Cnt=0; Cnt<2; Cnt++)
    {
        ret = tq_i2cWrite(Byte[Cnt]);
        if(ret)    return ret;
    }

	if(tq_i2cWriteStop())
		return FAILED;

    for(Cnt=0; Cnt<2; Cnt++)
    {
        ret = tq_i2cRead(wDevice, &Byte[Cnt]);
        if(ret)    return ret;
    }

	if(tq_i2cReadStop())
		return FAILED;

    *rVal = (Byte[0] << 8) + Byte[1];

    return ret;
}

int TDA6651_ChangeFreq(BYTE wDevice,DWORD wRf,BYTE wBw)
{
    int   ret = 0, Cnt = 0;
    BYTE Reg[5];
    DWORD Rf;

    Reg[2] = 0xC2;
    Reg[4] = 0x8D;

    Rf = (DWORD)((float)(wRf + 36167) / 166.66);
    Reg[0] = (BYTE)((Rf & 0x0000FF00) >> 8);
    Reg[1] = (BYTE)(Rf & 0x000000FF);

    if(wRf > 794000)         Reg[3] = 0xA4;
    else if(wRf > 584000)    Reg[3] = 0x84;
    else if(wRf > 446000)    Reg[3] = 0x64;
    else if(wRf > 384000)    Reg[3] = 0xA2;
    else if(wRf > 254000)    Reg[3] = 0x82;
    else if(wRf > 161000)    Reg[3] = 0x62;

	if(tq_i2cStart(TunerAddr))
		return FAILED;

    for(Cnt=0; Cnt<5; Cnt++)
    {
        ret = tq_i2cWrite(Reg[Cnt]);
        if(ret)    return ret;
    }

	if(tq_i2cWriteStop())
		return FAILED;

    return ret;
}

static void DIB7KDelay(DWORD DelaySec)
{
    mdelay(DelaySec);
}

int DIB7KEnumeration(BYTE *DefaultAddress,BYTE NumOfDemod)
{
    int   ret = 0;
    BYTE Device;
    int   Cnt;

    /*
     * i2c init com + demod identification
     */
    for (Cnt = NumOfDemod-1; Cnt >= 0; Cnt--)
    {
        /* reset the demod on the initial address */
	    Device = DIB7KAddress[Cnt];

	    ret |= DIB7KWriteReg(DefaultAddress[Cnt], 0xffff, 0xffff);

//		    ret = DIB7KReadReg(default_addr, 896, &value);
//		    if(ret || (value != DIB_VendorID))    printf("Error Vendor ID = %4X\n",value);
//	        else                                  printf("Vendor ID = %4X\n",value);

//		    ret = DIB7KReadReg(default_addr, 897, &value);
//	        if(ret || (value != DIB_DeviceID))    printf("Error Device ID = %4X\n",value);
//	        else                                  printf("Device ID = %4X\n",value);

        // activate diversity OUT bus to pull down the diversity start switch
		// P_esram_pinout_cfg
        ret |= DIB7KWriteReg(DefaultAddress[Cnt], 1805, 0x0c05);

	    // deactivate the host-bus - if ever diversity is used the host-bus, we
	    // have to use the std_divout-flag to know which bus to activate for
	    // i2c-enum
	    ret |= DIB7KWriteReg(DefaultAddress[Cnt], 1795, 0x0000);

#if DVBT_MODE
		ret |= DIB7KWriteReg(DefaultAddress[Cnt], 1796, 0x0); // select DVB-T output
#endif

		// set new i2c address and force divstr
		ret |= DIB7KWriteReg(DefaultAddress[Cnt], 1794,(Device<<2)|0x2);

	    // unforce divstr
	    ret |= DIB7KWriteReg(Device, 1794, Device<<2);

	    // P_esram_pinout_cfg divout off
	    ret |= DIB7KWriteReg(Device, 1805, 0x0005);
    }   // i2c-enumeration done

    return ret;
}

int DIB7KDemodReset(BYTE Device)
{
    int ret = 0;

    // deactivate the host-bus
    ret |= DIB7KWriteReg(Device, 1795, 0x0000);

    // power-up level
    ret |= DIB7KWriteReg(Device, 903, 0);
    ret |= DIB7KWriteReg(Device, 904, 0);
    ret |= DIB7KWriteReg(Device, 905, 0);
    ret |= DIB7KWriteReg(Device, 906, 0);

    //*************************** ADC band gap start-up problem WORKAROUND
    //*************************** DO NOT REMOVE THESE LINES
    // power-up ADC
    ret |= DIB7KWriteReg(Device, 913, 0x0000);
    ret |= DIB7KWriteReg(Device, 914, 0x0003);
    // power-down band gap
    ret |= DIB7KWriteReg(Device, 913, 0x8000);
    ret |= DIB7KWriteReg(Device, 914, 0x0001);
    //************************** END OF WORKAROUND

    //power-down ADC except band gap and power-down all other ADC blocks
    ret |= DIB7KWriteReg(Device, 913, 0x3000);
    ret |= DIB7KWriteReg(Device, 914, 0x003e);

    // reset all
    ret |= DIB7KWriteReg(Device, 898, 0xffff);
    ret |= DIB7KWriteReg(Device, 899, 0xffff);
    ret |= DIB7KWriteReg(Device, 900, 0xff0d); // do not reset GPIOs
    ret |= DIB7KWriteReg(Device, 901, 0xfffc);
    ret |= DIB7KWriteReg(Device, 898, 0);
    ret |= DIB7KWriteReg(Device, 899, 0);
    ret |= DIB7KWriteReg(Device, 900, 0);
    ret |= DIB7KWriteReg(Device, 901, 0);

    // I2C slave confguration
    if (DVBT_MODE && (Device & 0x2))
	    ret |= DIB7KWriteReg(Device, 224, 1);	//1
    else
	    ret |= DIB7KWriteReg(Device, 261, 3);	//2

    // P_i2c_mst_ctrl
    ret |= DIB7KWriteReg(Device, 769, 0x0080);

    return ret;
}

int DIB7KInit(BYTE Device)
{
    int ret = 0;

    //******************************************************
    // Demodulator default configuration
    //******************************************************

    ret |= DIB7KDemodReset(Device);

    ret |= DIB7KWriteReg(Device, 72 ,REG_AGC_CFG_72 );
    ret |= DIB7KWriteReg(Device, 73 ,REG_AGC_CFG_73 );
    ret |= DIB7KWriteReg(Device, 74 ,REG_AGC_CFG_74 );
    ret |= DIB7KWriteReg(Device, 88 ,REG_AGC_CFG_88 );
    ret |= DIB7KWriteReg(Device, 89 ,REG_AGC_CFG_89 );
    ret |= DIB7KWriteReg(Device, 90 ,REG_AGC_CFG_90 );
    ret |= DIB7KWriteReg(Device, 91 ,REG_AGC_CFG_91 );
    ret |= DIB7KWriteReg(Device, 92 ,REG_AGC_CFG_92 );
    ret |= DIB7KWriteReg(Device, 93 ,REG_AGC_CFG_93 );
    ret |= DIB7KWriteReg(Device, 94 ,REG_AGC_CFG_94 );
    ret |= DIB7KWriteReg(Device, 95 ,REG_AGC_CFG_95 );
    ret |= DIB7KWriteReg(Device, 96 ,REG_AGC_CFG_96 );
    ret |= DIB7KWriteReg(Device, 97 ,REG_AGC_CFG_97 );
    ret |= DIB7KWriteReg(Device, 102,REG_AGC_CFG_102);
    ret |= DIB7KWriteReg(Device, 103,REG_AGC_CFG_103);
    ret |= DIB7KWriteReg(Device, 104,REG_AGC_CFG_104);
    ret |= DIB7KWriteReg(Device, 105,REG_AGC_CFG_105);
    ret |= DIB7KWriteReg(Device, 106,REG_AGC_CFG_106);
    ret |= DIB7KWriteReg(Device, 107,REG_AGC_CFG_107);
    ret |= DIB7KWriteReg(Device, 108,REG_AGC_CFG_108);
    ret |= DIB7KWriteReg(Device, 109,REG_AGC_CFG_109);
    ret |= DIB7KWriteReg(Device, 110,REG_AGC_CFG_110);
    ret |= DIB7KWriteReg(Device, 111,REG_AGC_CFG_111);

    // P_sad_calibration
    ret |= DIB7KWriteReg(Device, 929, 1);
    ret |= DIB7KWriteReg(Device, 929, 0);

    // Slow ADC reference voltage
    ret |= DIB7KWriteReg(Device, 930, 4095);

    // Demod AGC loop configuration
    ret |= DIB7KWriteReg(Device, 98, REG_AGC_CFG_98);
    ret |= DIB7KWriteReg(Device, 99, REG_AGC_CFG_99);

    // P_corm_thres Lock algorithms configuration
    ret |= DIB7KWriteReg(Device, 26 ,0x6680);

    // P_palf_alpha_regul, P_palf_filter_freeze, P_palf_filter_on
    ret |= DIB7KWriteReg(Device, 170, 0x0410);

    // P_fft_nb_to_cut
    ret |= DIB7KWriteReg(Device, 182, 8192);

    // P_pha3_thres
    ret |= DIB7KWriteReg(Device, 195, 0x0ccd);

    // P_cti_use_cpe, P_cti_use_prog
    ret |= DIB7KWriteReg(Device, 196,0     );

    // P_cspu_regul, P_cspu_win_cut
    ret |= DIB7KWriteReg(Device, 205,0x200f);

    // P_adp_regul_cnt
    ret |= DIB7KWriteReg(Device, 214,0x023d);

    // P_adp_noise_cnt
    ret |= DIB7KWriteReg(Device, 215,0x00a4);

    // P_adp_regul_ext
    ret |= DIB7KWriteReg(Device, 216,0x00a4);

    // P_adp_noise_ext
    ret |= DIB7KWriteReg(Device, 217,0x7ff0);

    // P_adp_fil
    ret |= DIB7KWriteReg(Device, 218,0x3ccc);

    // P_equal_speedmode
    ret |= DIB7KWriteReg(Device, 261,2);

    // P_i2c_mst_data
    ret |= DIB7KWriteReg(Device, 768,0x4373);

    // P_i2c_mst_ctrl
    ret |= DIB7KWriteReg(Device, 769,24704);

    // P_fec_*
    ret |= DIB7KWriteReg(Device, 281,0x0010);

    // P_smo_mode, P_smo_rs_discard, P_smo_fifo_flush, P_smo_pid_parse, P_smo_error_discard
    ret |= DIB7KWriteReg(Device, 294,0x0062);

    // P_iqc_alpha_pha, P_iqc_alpha_amp, P_iqc_dcc_alpha, ...
#if BaseBand
    ret |= DIB7KWriteReg(Device, 36,0x0755);
#else
    ret |= DIB7KWriteReg(Device, 36,0x1f55);
#endif
    // auto search configuration
    ret |= DIB7KWriteReg(Device, 2  ,0x0004);
    ret |= DIB7KWriteReg(Device, 3  ,0x1000);
    ret |= DIB7KWriteReg(Device, 4  ,0x0814);
    ret |= DIB7KWriteReg(Device, 6  ,0x001b);
    ret |= DIB7KWriteReg(Device, 7  ,0x7740);
    ret |= DIB7KWriteReg(Device, 8  ,0x005b);
    ret |= DIB7KWriteReg(Device, 9  ,0x8d80);
    ret |= DIB7KWriteReg(Device, 10 ,0x01c9);
    ret |= DIB7KWriteReg(Device, 11 ,0xc380);
    ret |= DIB7KWriteReg(Device, 12 ,0x0000);
    ret |= DIB7KWriteReg(Device, 13 ,0x0080);
    ret |= DIB7KWriteReg(Device, 14 ,0x0000);
    ret |= DIB7KWriteReg(Device, 15 ,0x0090);
    ret |= DIB7KWriteReg(Device, 16 ,0x0001);
    ret |= DIB7KWriteReg(Device, 17 ,0xd4c0);
    ret |= DIB7KWriteReg(Device, 263,0x0001);

    // GPIO
    // only reset the GPIOs once, not in the middle of tuning - for now
    // ret |= DIB7KWriteReg(Device, 773, 0x0000);
    // ret |= DIB7KWriteReg(Device, 774, 0x0000);
    // ret |= DIB7KWriteReg(Device, 775, 0xffff);

    // P_clk_cfg1
    ret |= DIB7KWriteReg(Device, 908, 0x0006);

    ret |= DIB7KSetBandWidth(Device, INDEX_BW_8_0_MHZ);

    // Tuner IO bank: max drive (14mA)
    ret |= DIB7KWriteReg(Device, 912 ,0x2c92);

    return ret;
}

int DIB7KSetBandWidth(BYTE Device, BYTE BW_Idx)
{
    int ret = 0;
    DWORD  Cfg_18=0, Cfg_19=0, Cfg_21, Cfg_22,
          Cfg_23=0, Cfg_24=0, Cfg_907, Cfg_910, Cfg_928 ;
    switch(BW_Idx)
    {
        case INDEX_BW_8_0_MHZ:
        	Cfg_21 = REG_CLK_8_0_CFG_21;              /* 2 */ // IF freq setting
        	Cfg_22 = REG_CLK_8_0_CFG_22;              /* 2 */ // IF freq setting
            Cfg_23 = REG_CLK_8_0_CFG_23;              /* 2 */ // oscillator frequency
            Cfg_24 = REG_CLK_8_0_CFG_24;              /* 2 */ // oscillator frequency
            Cfg_907 = REG_CLK_8_0_CFG_907;            /* 1 */
            Cfg_910 = REG_CLK_8_0_CFG_910;            /* 1 */
            Cfg_928 = REG_CLK_8_0_CFG_928;            /* 1 */
	    break;
        case INDEX_BW_7_0_MHZ:
        	Cfg_21 = REG_CLK_7_0_CFG_21;              /* 2 */ // IF freq setting
        	Cfg_22 = REG_CLK_7_0_CFG_22;              /* 2 */ // IF freq setting
        	Cfg_23 = REG_CLK_7_0_CFG_23;              /* 2 */ // oscillator frequency
        	Cfg_24 = REG_CLK_7_0_CFG_24;              /* 2 */ // oscillator frequency
        	Cfg_907 = REG_CLK_7_0_CFG_907;            /* 1 */
        	Cfg_910 = REG_CLK_7_0_CFG_910;            /* 1 */
        	Cfg_928 = REG_CLK_7_0_CFG_928;            /* 1 */
	    break;
        case INDEX_BW_6_0_MHZ:
        	Cfg_21 = REG_CLK_6_0_CFG_21;               /* 2 */ // IF freq setting
        	Cfg_22 = REG_CLK_6_0_CFG_22;               /* 2 */ // IF freq setting
        	Cfg_23 = REG_CLK_6_0_CFG_23;               /* 2 */ // oscillator frequency
        	Cfg_24 = REG_CLK_6_0_CFG_24;               /* 2 */ // oscillator frequency
        	Cfg_907 = REG_CLK_6_0_CFG_907;             /* 1 */
        	Cfg_910 = REG_CLK_6_0_CFG_910;             /* 1 */
        	Cfg_928 = REG_CLK_6_0_CFG_928;             /* 1 */
	    break;
    }
/*
    for(Tcount = 0; Tcount < freq_all; Tcount++)
    {
	if(RF == _freq[Tcount])
	{
	    if(_IQ[Tcount] == 1)
	    {
		Cfg_21 &= 0xFDFF;
	    }
	    else
	    {
		Cfg_21 |= 0x0200;
	    }
	}
    }
*/
    // oscillator frequency should be 30 MHz for the Master (default values in the board_parameters give that value)
    // this is only working only for 30 MHz crystals
#if !quartz_direct
	    Cfg_910 = (0x1f & Cfg_910) | 0x20 ;       /* 1 */ // forcing the predivider to 1

    #if input_clk_is_div_2
	    // if the previous front-end is baseband its output frequency is 15 MHz (prev freq divided by 2)
		Cfg_907 = (0x81ff & Cfg_907) | 0x2000 ;   /* 1 */  // 16
    #else
	// otherwise the previous front-end puts out its input (default 30MHz) - no extra division necessary
		Cfg_907 = (0x81ff & Cfg_907) | 0x1000 ;   /* 1 */  // 8
    #endif
#endif

    ret |= DIB7KWriteReg(Device, 18, Cfg_18);   // mpeg error timer /* 2 */
    ret |= DIB7KWriteReg(Device, 19, Cfg_19);   // mpeg error timer /* 2 */
    ret |= DIB7KWriteReg(Device, 21, Cfg_21);   // dds              /* 2 */
    ret |= DIB7KWriteReg(Device, 22, Cfg_22);   // dds              /* 2 */
    ret |= DIB7KWriteReg(Device, 23, Cfg_23);   // timf             /* 2 */
    ret |= DIB7KWriteReg(Device, 24, Cfg_24);   // timf             /* 2 */
    ret |= DIB7KWriteReg(Device, 910, Cfg_910); // pll cfg          /* 1 */
    ret |= DIB7KWriteReg(Device, 907, Cfg_907); // clk cfg          /* 1 */
    ret |= DIB7KWriteReg(Device, 928, Cfg_928); // sar cfg          /* 1 */

    return ret;
}

int DIB7KDoAutoSearch(BYTE *Device, BYTE NumOfDemod, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    int   Cnt, K;
    BYTE NumFailed = 0, Succeeded = 0;
    BYTE Value;
    BYTE LockAddr=0;

    for(Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    ret |= DIB7KAutoSearchStart(Device[Cnt], CD);
    }

    Cnt = 0;
    while((!Succeeded) && (!NumFailed) && (Cnt < MAX_AUTOSEARCH_ITERATIONS))
    {
        Cnt++;
        Succeeded = 0;
        NumFailed = 1;
	    for (K = 0; K < NumOfDemod; K++)
		{
			Value = DIB7KAutoSearchIsIrq(Device, &LockAddr, NumOfDemod);
			switch (Value)
            {
			    case 0:
	                break;
				case 2:
#if debug
				    printf("-I-  Autosearch succeeded for demod %d - done.\n",K);
#endif
				    Succeeded = 1;
				    break;
				default:
#if debug
				    printf("-E-  Autosearch IRQ return unhandled value for demod %d.\n",K);
#endif
				case 1: /* fall throught wanted */
#if debug
				    printf("-I-  Autosearch failed for demod %d - exluding from list.\n",K);
#endif
				    NumFailed = 0;
				    break;
			}

			if (Succeeded)
			    break;
		    else // no autoseach_irq-function set
		    {
#if debug
			    printf("-E-  demod has to autosearch IRQ callback\n");
#endif
			    NumFailed = 0;
		    }
	    }
    }

    if (!Succeeded)
    {
	    printf("-W-  Autosearch failed for %d demod(s)\n",NumFailed);
	    return FAILED;
    }

    ret |= DIB7KDvbtGetChannelDescription(LockAddr, CD);

    return SUCCESS;
}

static int DIB7KAutoSearchStart(BYTE Device, struct DIBDVBChannel *CD)
{
    int ret = 0;

    ret |= DIB7KSetScanCfg(Device, CD);

    // set power-up level: interf+analog+AGC
    ret |= DIB7KSetPowerLevel(Device, DIB7000_POWER_LEVEL_INTERF_ANALOG_AGC);
    ret |= DIB7KSetAdcCfg(Device, 0);
    ret |= DIB7KPllClkCfg(Device);

    DIB7KDelay(7);

    //AGC initialization
    ret |= DIB7KSadCalib(Device);
    ret |= DIB7KRestartAgc(Device);

    // wait AGC rough lock time
    DIB7KDelay(5);

//    DIB7KAgcSoftSplit(Device);

    // wait AGC accurate lock time
    DIB7KDelay(7);

    // set power-up level: autosearch
    ret |= DIB7KSetPowerLevel(Device, DIB7000_POWER_LEVEL_DOWN_COR4_DINTLV_ICIRM_EQUAL_CFROD);

    // start search
    ret |= DIB7KRestartSearch(Device);

    return ret;
}

static int DIB7KAutoSearchIsIrq(BYTE *Device, BYTE *LockAddr, BYTE NumOfDemod)
{
    int   ret = 0;
    int   Cnt;
    DWORD  IRQ_Pending=0;
    BYTE Check = 0;

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    ret |= DIB7KReadReg(Device[Cnt], 1793, &IRQ_Pending); /* 1 */

	    if(IRQ_Pending & 0x1) // failed
        {
	        printf("#");
	        Check++;
        }

	    if (IRQ_Pending & 0x2) // succeeded
        {
	        printf("!\n");
            *LockAddr = Device[Cnt];
            return 2;
        }
    }
    if (Check >= 1)
    {
        *LockAddr = Device[0];
        return 1;
    }

    return 0; // still pending
}

static int DIB7KDvbtGetChannelDescription(BYTE Device, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    DWORD  Value=0;

    ret |= DIB7KReadReg(Device, 480, &Value);     /* 2 */
    printf("chnnale description: %02x\n", Value);
    CD->nfft = (Value >> 8) & 0x3;
    CD->guard = Value & 0x3;
    CD->nqam = (Value >> 14) & 0x3;
    CD->vit_hrch = (Value >> 13) & 0x1;
    CD->vit_alpha = (Value >> 10) & 0x7;
    CD->vit_code_rate_hp = (Value >> 5) & 0x7;
    CD->vit_code_rate_lp = (Value >> 2) & 0x7;

    ret |= DIB7KReadReg(Device, 481, &Value);     /* 2 */
    CD->intlv_native = (Value >> 5) & 0x1;

    return ret;
}

static int DIB7KSetScanCfg(BYTE Device, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    BYTE Seq, Fs, Gs;
	DWORD  Cfg_0, Cfg_266, Val=0;        /* 2 */
    DWORD Scan_time0, Scan_time1, Scan_time2;
    int   Nfft, Guard;

    Nfft = CD->nfft;
    Guard = CD->guard;

    Seq = 0;
    Fs = (Nfft < 0);
    Gs = (Guard < 0);
    if ((Fs) & (!Gs))		Seq = 2;
    if ((Fs) & (Gs))		Seq = 7;
    if ((!Fs) & (!Gs))		Seq = 0;
    if ((!Fs) & (Gs))		Seq = 3;

    if ((!Gs) && (!Fs))		Cfg_0 = 0x11 | ((Guard & 0x3) << 5) | ((Nfft & 0x3) << 7);  /* 2 */
    else if (!Fs)		    Cfg_0 = 0x11 | ((Nfft & 0x3) << 7);                         /* 2 */
    else if (!Gs)		    Cfg_0 = 0x91 | ((Guard & 0x3) << 5);                        /* 2 */
    else 		            Cfg_0 = 0x91;                                               /* 2 */

	ret = DIB7KReadReg(Device,266,&Val);
    Cfg_266 = 0xfff0 | (Val & 0x000f);

    Scan_time0 =  30 * REG_CLK_8_0_INTERNAL; // always use the setting for 8MHz here
    Scan_time1 = 100 * REG_CLK_8_0_INTERNAL; // lock_time for 7,6 MHz are longer
    Scan_time2 = 500 * REG_CLK_8_0_INTERNAL;

    // stateulator lock configuration
    ret |= DIB7KWriteReg(Device,   0, Cfg_0);                              // nfft guard                                          /* 2 */
    ret |= DIB7KWriteReg(Device,   5, (Seq << 4));                         // search list                                      /* 2 */
    ret |= DIB7KWriteReg(Device,   6, (DWORD)((Scan_time0 >> 16) & 0xffff));// lock0 wait time MSWord  /* 2 */
    ret |= DIB7KWriteReg(Device,   7, (DWORD)(Scan_time0 & 0xffff));        // lock0 wait time LSWord        /* 2 */
    ret |= DIB7KWriteReg(Device,   8, (DWORD)((Scan_time1 >> 16) & 0xffff));// lock1 wait time MSWord  /* 2 */
    ret |= DIB7KWriteReg(Device,   9, (DWORD)(Scan_time1 & 0xffff));        // lock1 wait time LSWord        /* 2 */
    ret |= DIB7KWriteReg(Device,  10, (DWORD)((Scan_time2 >> 16) & 0xffff));// lock1 wait time MSWord  /* 2 */
    ret |= DIB7KWriteReg(Device,  11, (DWORD)(Scan_time2 & 0xffff));        // lock1 wait time LSWord       /* 2 */
    ret |= DIB7KWriteReg(Device,  26, 0x6680);    // timf_d(6xxx)                                       /* 2 */
    ret |= DIB7KWriteReg(Device,  29, 0x1273);    // isi inh1273 on1073                                 /* 2 */
    ret |= DIB7KWriteReg(Device,  32, 0x0003);    // pha_off_max(xxx3)                                  /* 2 */
    ret |= DIB7KWriteReg(Device,  33, 0x0005);    // sfreq(xxx5)                                        /* 2 */
    ret |= DIB7KWriteReg(Device, 214, 0x023d);   // P_adp_regul_cnt 0.07                               /* 2 */
    ret |= DIB7KWriteReg(Device, 215, 0xffdf);   // P_adp_noise_cnt -0.004                             /* 2 */
    ret |= DIB7KWriteReg(Device, 216, 0x00a4);   // P_adp_regul_ext 0.02                               /* 2 */
    ret |= DIB7KWriteReg(Device, 217, 0xfff0);   // P_adp_noise_ext -0.002                             /* 2 */
    ret |= DIB7KWriteReg(Device, 266, Cfg_266);  // dvsy wait                                         /* 2 */

    return ret;
}

static int DIB7KSetPowerLevel(BYTE Device, BYTE PowerLevel)
{
    int   ret = 0;
    DWORD  Cfg_903 = 0, Cfg_904 = 0,
          Cfg_905 = 0, Cfg_906 = 0;

    switch (PowerLevel)
    {
	    case DIB7000_POWER_LEVEL_ALL_UP:
		    break;
	    case DIB7000_POWER_LEVEL_ALL_DOWN:
		    Cfg_903 = 0xffff;
		    Cfg_904 = 0xffff;
		    Cfg_905 = 0xffff;
		    Cfg_906 = 0xffff;
		    break;
	    case DIB7000_POWER_LEVEL_INTERF_ANALOG_AGC:
		    Cfg_903 = 0x33ff;
		    Cfg_904 = 0xffff;
		    Cfg_905 = 0xff0b;
		    Cfg_906 = 0xfffe;
		    break;
	    case DIB7000_POWER_LEVEL_DOWN_COR4_DINTLV_ICIRM_EQUAL_CFROD:
		    // cor4 + dintlv + icirm2 + icirm1 + equal + cfrod1
		    Cfg_904 = 0x801f;
		    break;
	    case DIB7000_POWER_LEVEL_DOWN_COR4_CRY_ESRAM_MOUT_NUD:
		    // cor4
		    Cfg_904 = 0x8000;
		    // cry + esram + Mout + nud
		    Cfg_905 = 0x010b;
		    break;
	    case DIB7000_POWER_LEVEL_UP_GPIO_I2C_SDIO:
		    Cfg_903 = 0xffff;
		    Cfg_904 = 0xffff;
		    // GPIO + I2C + SDIO
		    Cfg_905 = 0xffdb;
		    Cfg_906 = 0xffff;
		    break;
	    default:
		    break;
    }

    ret |= DIB7KWriteReg(Device, 903, Cfg_903);       /* 1 */
    ret |= DIB7KWriteReg(Device, 904, Cfg_904);       /* 1 */
    ret |= DIB7KWriteReg(Device, 905, Cfg_905);       /* 1 */
    ret |= DIB7KWriteReg(Device, 906, Cfg_906);       /* 1 */

    return ret;
}

static int DIB7KSetAdcCfg(BYTE Device, BYTE CfgNo)
{
    int   ret = 0;
    DWORD  Cfg_913 = 0, Cfg_914 = 1;

    switch (CfgNo)
    {
	    case 0:
		    Cfg_913 = 0x0000;
		    Cfg_914 = 0x0001;
		    break;
	    case 1:
		    Cfg_913 = 0x0000;
		    Cfg_914 = 0x0003;
		    break;
	    case 2:
		    Cfg_913 = 0x8000;
		    Cfg_914 = 0x0001;
		    break;
	    case 3:
		    Cfg_913 = 0x3000;
		    Cfg_914 = 0x007e;
		    break;
	    default:
		    break;
    }

    ret |= DIB7KWriteReg(Device, 913, Cfg_913);       /* 1 */
    ret |= DIB7KWriteReg(Device, 914, Cfg_914);       /* 1 */

    return ret;
}

static int DIB7KPllClkCfg(BYTE Device)
{
    int   ret = 0;
    DWORD  Val = 0;

    ret |= DIB7KReadReg(Device, 910, &Val);           /* 1 */
    ret |= DIB7KWriteReg(Device, 910, (Val | 0x1));   //pwr-up pll    /* 1 */
    ret |= DIB7KReadReg(Device, 907, &Val);           /* 1 */
    ret |= DIB7KWriteReg(Device, 907, (Val & 0x7fff) | (1 << 6));     //use High freq clock   /* 1 */

    return ret;
}

static int DIB7KSadCalib(BYTE Device)
{
    int ret = 0;

    // P_sad_calibration
    ret |= DIB7KWriteReg(Device, 929, 1);     /* 1 */
    ret |= DIB7KWriteReg(Device, 929, 0);     /* 1 */

    return ret;
}

static int DIB7KRestartAgc(BYTE Device)
{
    int ret = 0;

    // P_restart_iqc & P_restart_agc
    ret |= DIB7KWriteReg(Device, 898, 0x0c00); /* 1 */
    ret |= DIB7KWriteReg(Device, 898, 0x0000); /* 1 */

    return ret;
}

static int DIB7KRestartSearch(BYTE Device)
{
    int   ret = 0;
    DWORD  Val = 0;

    ret |= DIB7KReadReg(Device, 0, &Val);         /* 2 */
    ret |= DIB7KWriteReg(Device, 0, 0x0200|Val);  /* 2 */
    ret |= DIB7KWriteReg(Device, 1793, 0x0000);   /* 1 */
    ret |= DIB7KWriteReg(Device, 0, Val);         /* 2 */

    return ret;
}

static int DIB7KAgcSoftSplit(BYTE Device)
{
    int   ret = 0;
    DWORD  Agc=0, SplitOffset;

    // n_agc_global
    ret |= DIB7KReadReg(Device, 390, &Agc);

    if (Agc > 45000)
	    SplitOffset = 0;
	else if (Agc < 12200)
	    SplitOffset = 128;
	else
	    SplitOffset = 128 * (Agc - 45000) / (12200 - 45000);

    // P_agc_force_split and P_agc_split_offset
    ret |= DIB7KWriteReg(Device, 103, 0x100 | SplitOffset);

    return ret;
}

static int DIB7KRestart(BYTE Device)
{
    int   ret = 0;
    DWORD  Val = 0;

    if(!DVBT_MODE)
	{
	    ret |= DIB7KReadReg(Device, 5, &Val);
	    ret |= DIB7KWriteReg(Device, 5, (Val & 0xff0f) | (0 << 4));

	    ret |= DIB7KReadReg(Device, 0, &Val);
	    ret |= DIB7KWriteReg(Device, 0, (Val & 0xfdff) | (0 << 9));

	    ret |= DIB7KReadReg(Device, 0, &Val);
	    ret |= DIB7KWriteReg(Device, 0, (Val & 0xfdff) | (1 << 9));

	    ret |= DIB7KReadReg(Device, 0, &Val);
	    ret |= DIB7KWriteReg(Device, 0, (Val & 0xfdff) | (0 << 9));
    }
    else
    {
	    ret |= DIB7KWriteReg(Device, 898, 0x4000);        /* 1 */
	    ret |= DIB7KWriteReg(Device, 898, 0x0000);        /* 1 */
    }

    return ret;
}

static void DIB7KDumpDvbtChannelParams(struct DIBDVBChannel *CD)
{
    printf("\nCOFDM CHANNEL at %d kHz: \n",CD->RfKHz);
    switch (CD->nfft)
    {
	    case 1:  printf("8K ");      break;
	    case 2:  printf("4K ");      break;
	    case 0:  printf("2K ");      break;
	    default: printf("FFT_UNK "); break;
    }
    if(CD->guard == -1)    CD->guard = 10;
    printf("\nGi: 1/%i  \n", 32 / (1 << CD->guard));
    switch (CD->nqam)
    {
	    case 0:  printf("QPSK ");    break;
	    case 1:  printf("16QAM ");   break;
	    case 2:  printf("64QAM ");   break;
	    default: printf("QAM_UNK "); break;
    }
    printf("\n%s \n", CD->intlv_native ? "NATIVE_INTLV" : "EXTENDED_INTLV");
    printf("ALPHA %i \n", CD->vit_alpha);
    printf("Code Rate HP %i/%i \n",   CD->vit_code_rate_hp, CD->vit_code_rate_hp + 1);
    printf("Code Rate LP %i/%i \n", CD->vit_code_rate_lp, CD->vit_code_rate_lp + 1);
    printf("HRCH %i\n", CD->vit_hrch);
}

int DIB7KTune(BYTE Device, struct DIBDVBChannel *CD)
{
    int ret = 0;

    ret |= DIB7KSetTuneCfg(Device,CD);
    ret |= DIB7KTuneChannel(Device);

    return ret;
}

static int DIB7KSetTuneCfg(BYTE Device, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    DWORD  Val = 0;
    DWORD  Cfg_0, Cfg_26, Cfg_32, Cfg_33,
          Cfg_214, Cfg_215, Cfg_216, Cfg_217,
          Cfg_266, Cfg_267;   /* 2 */

    // modulation parameters
    Cfg_0   = (CD->vit_alpha & 0x7) | ((CD->nqam & 0x3) << 3) | ((CD->guard & 0x3) << 5) | ((CD->nfft & 0x3) << 7);     /* 2 */
    Cfg_267 = ((CD->vit_hrch & 0x1) << 4) | ((CD->intlv_native & 0x1) << 6);        /* 2 */

    if((CD->vit_hrch == 0) || (CD->vit_select_hp == 1))
	    Cfg_267 |= ((CD->vit_code_rate_hp & 0x7) << 1) | 0x1;    /* 2 */
    else
	    Cfg_267 |= ((CD->vit_code_rate_lp & 0x7) << 1);          /* 2 */

    /* deactive the possibility of diversity reception if extended interleave */
#if 0
    if (CD->intlv_native)
	    Cfg_266 = 0x6;
    else
	    Cfg_266 = 0x0;
#else
	ret |= DIB7KReadReg(Device, 266, &Val);
    Cfg_266 = Val & 0x000f;
#endif
    // offset loop parameters
    switch (CD->nfft)
	{
	    case 0:
		    Cfg_26   = 0x7680;      // timf_a(7xxx)                                                        /* 2 */
		    Cfg_32   = 0x0006;      // pha_off_max(xxx6)                                                   /* 2 */
		    Cfg_33   = 0x0006;      // sfreq(xxx6)                                                         /* 2 */
		    Cfg_266 |= 0x3ff0;      // dvsy wait                                                           /* 2 */
		    break;
	    case 2:
		    Cfg_26   = 0x8680;      // timf_a(8xxx)                                                        /* 2 */
		    Cfg_32   = 0x0007;      // pha_off_max(xxx7)                                                   /* 2 */
		    Cfg_33   = 0x0007;      // sfreq(xxx7)                                                         /* 2 */
		    Cfg_266 |= 0x7ff0;      // dvsy wait                                                           /* 2 */
		    break;
	    default:
		    Cfg_26   = 0x9680;      // timf_a(9xxx)                                                        /* 2 */
		    Cfg_32   = 0x0008;      // pha_off_max(xxx8)                                                   /* 2 */
		    Cfg_33   = 0x0008;      // sfreq(xxx8)                                                         /* 2 */
		    Cfg_266 |= 0xfff0;      // dvsy wait                                                           /* 2 */
		    break;
    }

    // channel estimation fine configuration
    switch (CD->nqam)
    {
	    case 2:
		    Cfg_214 = 0x0148;       // P_adp_regul_cnt 0.04                                               /* 2 */
		    Cfg_215 = 0xfff0;       // P_adp_noise_cnt -0.002                                             /* 2 */
		    Cfg_216 = 0x00a4;       // P_adp_regul_ext 0.02                                               /* 2 */
		    Cfg_217 = 0xfff8;       // P_adp_noise_ext -0.001                                             /* 2 */
		    break;
	    case 1:
		    Cfg_214 = 0x023d;       // P_adp_regul_cnt 0.07                                               /* 2 */
		    Cfg_215 = 0xffdf;       // P_adp_noise_cnt -0.004                                             /* 2 */
		    Cfg_216 = 0x00a4;       // P_adp_regul_ext 0.02                                               /* 2 */
		    Cfg_217 = 0xfff0;       // P_adp_noise_ext -0.002                                             /* 2 */
		    break;
	    default:
		    Cfg_214 = 0x099a;       // P_adp_regul_cnt 0.3                                                /* 2 */
		    Cfg_215 = 0xffae;       // P_adp_noise_cnt -0.01                                              /* 2 */
		    Cfg_216 = 0x0333;       // P_adp_regul_ext 0.1                                                /* 2 */
		    Cfg_217 = 0xfff8;       // P_adp_noise_ext -0.002                                             /* 2 */
		    break;
    }

    //stateulator locked configuration
    ret |= DIB7KWriteReg(Device, 0,   Cfg_0);   // nfft guard                   /* 2 */
    ret |= DIB7KWriteReg(Device, 5,   0);       // do not force tps, search list 0  /* 2 */
    ret |= DIB7KWriteReg(Device, 26,  Cfg_26);  // timf_a(6xxx)                /* 2 */
    ret |= DIB7KWriteReg(Device, 29,  0x1073);  // isi inh1273 on1073          /* 2 */
    ret |= DIB7KWriteReg(Device, 32,  Cfg_32);  // pha_off_max(xxx3)           /* 2 */
    ret |= DIB7KWriteReg(Device, 33,  Cfg_33);  // sfreq(xxx5)                 /* 2 */
    ret |= DIB7KWriteReg(Device, 214, Cfg_214); // P_adp_regul_cnt 0.07       /* 2 */
    ret |= DIB7KWriteReg(Device, 215, Cfg_215); // P_adp_noise_cnt -0.004     /* 2 */
    ret |= DIB7KWriteReg(Device, 216, Cfg_216); // P_adp_regul_ext 0.02       /* 2 */
    ret |= DIB7KWriteReg(Device, 217, Cfg_217); // P_adp_noise_ext -0.002     /* 2 */
    ret |= DIB7KWriteReg(Device, 266, Cfg_266); // dvsy wait                  /* 2 */
    ret |= DIB7KWriteReg(Device, 267, Cfg_267); // viterbi cfg                /* 2 */

    return ret;
}

static int DIB7KTuneChannel(BYTE Device)
{
    int   ret = 0;
    DWORD  LockParam0=0, LockParam2=0, LockParam1=0, LockParam3=0;

    // set power-up level: interf+analog+AGC
    ret |= DIB7KSetPowerLevel(Device, DIB7000_POWER_LEVEL_INTERF_ANALOG_AGC);
    ret |= DIB7KSetAdcCfg(Device, 0);
    ret |= DIB7KPllClkCfg(Device);

    //stateulator lock configuration
    ret |= DIB7KReadReg(Device, 26, &LockParam0);     /* 2 */
    ret |= DIB7KReadReg(Device, 29, &LockParam1);     /* 2 */
    ret |= DIB7KReadReg(Device, 32, &LockParam2);     /* 2 */
    ret |= DIB7KReadReg(Device, 33, &LockParam3);     /* 2 */
    ret |= DIB7KWriteReg(Device, 26, 0x6680);   //*************** timf(6xxx)          /* 2 */
    ret |= DIB7KWriteReg(Device, 29, 0x1273);   //*************** isi inh1273 on1073  /* 2 */
    ret |= DIB7KWriteReg(Device, 32, 0x0003);   //*************** pha_off_max(xxx3)   /* 2 */
    ret |= DIB7KWriteReg(Device, 33, 0x0005);   //*************** sfreq(xxx5)         /* 2 */

    //AGC initialization
    ret |= DIB7KSadCalib(Device);

    DIB7KDelay(7);
    //AGC initialization
    ret |= DIB7KSadCalib(Device);
    ret |= DIB7KRestartAgc(Device);

    // wait AGC rough lock time
    DIB7KDelay(5);
    ret |= DIB7KAgcSoftSplit(Device);

    // wait AGC accurate lock time
    DIB7KDelay(7);

    // set power-up level
    ret |= DIB7KSetPowerLevel(Device, DIB7000_POWER_LEVEL_DOWN_COR4_DINTLV_ICIRM_EQUAL_CFROD);

    // start dem
    ret |= DIB7KRestart(Device);

    DIB7KDelay(45);
    ret |= DIB7KSetPowerLevel(Device, DIB7000_POWER_LEVEL_DOWN_COR4_CRY_ESRAM_MOUT_NUD);
    ret |= DIB7KWriteReg(Device, 29, LockParam1);      /* 2 */

    // stateulator tracking configuration
    ret |= DIB7KWriteReg(Device, 26, LockParam0);      /* 2 */
    ret |= DIB7KWriteReg(Device, 32, LockParam2);      /* 2 */
    ret |= DIB7KWriteReg(Device, 33, LockParam3);      /* 2 */

    return ret;
}

int DIB7KSetDivIn(BYTE Device, BYTE OnOff)
{
    int   ret = 0;
    DWORD  Value0, Val=0;
#if 0
    if (OnOff)
    {
	    ret |= DIB7KWriteReg(Device, 263, 6);
	    ret |= DIB7KWriteReg(Device, 264, 6);
    }
    else
    {
	    ret |= DIB7KWriteReg(Device, 263, 1);
	    ret |= DIB7KWriteReg(Device, 264, 0);
    }
#else
	ret |= DIB7KReadReg(Device, 266, &Val);
    Value0 = Val & 0xfff0;

    if (OnOff)
    {
	    ret |= DIB7KWriteReg(Device, 264, 6);
	    ret |= DIB7KWriteReg(Device, 266, Value0 | 0x6); // TODO do that at initialization time - deactivate when signal has extended intl do that in tune
    }
    else
    {
	    ret |= DIB7KWriteReg(Device, 264, 0);
	    ret |= DIB7KWriteReg(Device, 266, Value0);
    }
#endif
    return ret;
}

int DIB7KDemodSetOutputMode(BYTE Device, BYTE OutputMode)
{
    int   ret = 0;
    DWORD  OutReg, FiFoThresHold, SmoMode,
    Sram = 0x0005; /* by default SRAM output is disabled */

    OutReg = 0;
    FiFoThresHold = 1792;
    SmoMode = (1 << 1);

    switch (OutputMode)
	{
	    case OUTMODE_MPEG2_PAR_GATED_CLK:   // STBs with parallel gated clock
		    OutReg = (1 << 10);             /* 0x0400 */
		    break;
	    case OUTMODE_MPEG2_PAR_CONT_CLK:    // STBs with parallel continues clock
		    OutReg = (1 << 10) | (1 << 6);  /* 0x0440 */
		    break;
	    case OUTMODE_MPEG2_SERIAL:          // STBs with serial input
		    OutReg = (1 << 10) | (2 << 6) | (1 << 1); /* 0x0482 */
		    break;
	    case OUTMODE_DIVERSITY:
		    Sram   |= 0x0c00;
		    break;
	    case OUTMODE_MPEG2_FIFO:            // e.g. USB feeding
		    SmoMode = (3 << 1);
		    FiFoThresHold = 512;
		    OutReg = (1 << 10) | (5 << 6);
		    break;
	    case OUTMODE_HIGH_Z:                // disable - for Diversity on board where the sram- and host-bus share the lines to the 77x0
		    OutReg = 0;
		    break;
	    default:
		    break;
    }

    if (output_mpeg2_in_188_bytes)
	    SmoMode |= (1 << 5) ;

    ret |= DIB7KWriteReg(Device,  294, SmoMode);        /* 2 */
    ret |= DIB7KWriteReg(Device,  295, FiFoThresHold);  /* 2 */
    ret |= DIB7KWriteReg(Device, 1795, OutReg);         /* 1 */
    ret |= DIB7KWriteReg(Device, 1805, Sram);

    return ret;
}

int DIB7KTuneDiversity(BYTE NumOfDemod, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    int   Cnt;

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    printf("Init = %d\n",DIB7KInit(DIB7KAddress[Cnt]));
	    printf("TunerInit = %d\n",DIB7KTunerInit(DIB7KAddress[Cnt]));
    }

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
		printf("SetTuenr = %d\n",DIB7KSetTuner(DIB7KAddress[Cnt],CD));
		printf("SetBW = %d\n",DIB7KSetBandWidth(DIB7KAddress[Cnt],CD->Bw));//INDEX_BW_8_0_MHZ
    }

//    DIB7KDelay(200);
    printf("search = %d\n",DIB7KDoAutoSearch(DIB7KAddress,NumOfDemod,CD));
//    DIB7KDelay(200);
    DIB7KDumpDvbtChannelParams(CD);

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    printf("tune = %d\n",DIB7KTune(DIB7KAddress[Cnt],CD));
    }

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
#if 0
	    if(Cnt == (NumOfDemod - 1))// last demod in a diversity chain - turn off div-in combination
		    printf("setDivIn0 = %d\n",DIB7KSetDivIn(DIB7KAddress[Cnt],0));
		else
            printf("setDivIn1 = %d\n",DIB7KSetDivIn(DIB7KAddress[Cnt],1));
		if(Cnt == 0)               // first demod in a diversity chain - no diversity output
		    printf("outmode = %d\n",DIB7KDemodSetOutputMode(DIB7KAddress[Cnt],OUTMODE_MPEG2_PAR_GATED_CLK));
		else
		    //printf("outmode = %d\n",DIB7KDemodSetOutputMode(DIB7KAddress[Cnt],OUTMODE_DIVERSITY));
#endif
		    printf("outmode = %d\n",DIB7KDemodSetOutputMode(DIB7KAddress[Cnt],OUTMODE_MPEG2_PAR_GATED_CLK));
    }

    return ret;
}

int DIB7KTuneSerial(BYTE NumOfDemod, struct DIBDVBChannel *CD)
{
    int   ret = 0;
    int   Cnt;

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    printf("Init = %d\n",DIB7KInit(DIB7KAddress[Cnt]));
	    printf("TunerInit = %d\n",DIB7KTunerInit(DIB7KAddress[Cnt]));
    }

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
		printf("SetTuenr = %d\n",DIB7KSetTuner(DIB7KAddress[Cnt],CD));
		printf("SetBW = %d\n",DIB7KSetBandWidth(DIB7KAddress[Cnt],CD->Bw));//INDEX_BW_8_0_MHZ
    }

//    DIB7KDelay(200);
    printf("search = %d\n",DIB7KDoAutoSearch(DIB7KAddress,NumOfDemod,CD));
//    DIB7KDelay(200);
    DIB7KDumpDvbtChannelParams(CD);

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
	    printf("tune = %d\n",DIB7KTune(DIB7KAddress[Cnt],CD));
    }

    for (Cnt = 0; Cnt < NumOfDemod; Cnt++)
    {
#if 0
	    if(Cnt == (NumOfDemod - 1))// last demod in a diversity chain - turn off div-in combination
		    printf("setDivIn0 = %d\n",DIB7KSetDivIn(DIB7KAddress[Cnt],0));
		else
            printf("setDivIn1 = %d\n",DIB7KSetDivIn(DIB7KAddress[Cnt],1));
		if(Cnt == 0)               // first demod in a diversity chain - no diversity output
		    printf("outmode = %d\n",DIB7KDemodSetOutputMode(DIB7KAddress[Cnt],OUTMODE_MPEG2_PAR_GATED_CLK));
		else
#endif
		    printf("outmode = %d\n",DIB7KDemodSetOutputMode(DIB7KAddress[Cnt],OUTMODE_MPEG2_SERIAL));
    }

    return ret;
}

int DIB7KConnect(BYTE Device, BYTE TunerAddress)
{
    int  ret = 0;
    DWORD Val;

    Val = TunerAddress << 7;
    ret |= DIB7KWriteReg(Device, 769, Val);

    return ret;
}

int DIB7KDisConnect(BYTE Device, BYTE TunerAddress)
{
    int  ret = 0;
    DWORD Val;

    Val = (TunerAddress << 7)|(1 << 7);
    ret |= DIB7KWriteReg(Device, 769, Val);

    return ret;
}

int DIB7KTunerInit(BYTE Device)
{
    int ret = 0;

	return ret;
}

int DIB7KSetTuner(BYTE Device, struct DIBDVBChannel *CD)
{
    BYTE ret = 0;

    ret |= DIB7KConnect(Device,TunerAddr);
    ret |= TDA6651_ChangeFreq(Device,CD->RfKHz,(8-CD->Bw));
    ret |= DIB7KDisConnect(Device,TunerAddr);

    return ret;
}

