#ifndef __DIB7KREG_H_
#define __DIB7KREG_H_

#define DIB_VendorID                    0x01B3
#define DIB_DeviceID                    0x4000

#define NUM_OF_DEMODS                   1
#define DVBT_MODE                       1
#define BaseBand                        0
#define debug							0

#define quartz_direct                   0
#define input_clk_is_div_2              0
#define output_mpeg2_in_188_bytes       1
#define update_lna                      NULL

#define DIB7000_POWER_LEVEL_ALL_UP                             0
#define DIB7000_POWER_LEVEL_ALL_DOWN                           1
#define DIB7000_POWER_LEVEL_INTERF_ANALOG_AGC                  2
#define DIB7000_POWER_LEVEL_DOWN_COR4_DINTLV_ICIRM_EQUAL_CFROD 3
#define DIB7000_POWER_LEVEL_DOWN_COR4_CRY_ESRAM_MOUT_NUD       4
#define DIB7000_POWER_LEVEL_UP_GPIO_I2C_SDIO                   5

#define OUTMODE_HIGH_Z                      0
#define OUTMODE_MPEG2_PAR_GATED_CLK         1
#define OUTMODE_MPEG2_PAR_CONT_CLK          2
#define OUTMODE_DIVERSITY                   4
#define OUTMODE_MPEG2_FIFO                  5
#define OUTMODE_MPEG2_SERIAL                7

#define MAX_AUTOSEARCH_ITERATIONS       200

#define INDEX_BW_8_0_MHZ                0
#define INDEX_BW_7_0_MHZ                1
#define INDEX_BW_6_0_MHZ                2


#define REG_AGC_CFG_72                  10340
#define REG_AGC_CFG_73                  712
#define REG_AGC_CFG_74                  41
#define REG_AGC_CFG_88                  676
#define REG_AGC_CFG_89                  696
#define REG_AGC_CFG_90                  717
#define REG_AGC_CFG_91                  737
#define REG_AGC_CFG_92                  758
#define REG_AGC_CFG_93                  778
#define REG_AGC_CFG_94                  799
#define REG_AGC_CFG_95                  819
#define REG_AGC_CFG_96                  840
#define REG_AGC_CFG_97                  118
#define REG_AGC_CFG_98                  697
#define REG_AGC_CFG_99                  1840
#define REG_AGC_CFG_103                 256
#define REG_AGC_CFG_102                 4096

#define REG_AGC_CFG_104                 42598 /*(INT)(0.65*65536)*/  /* agc1_max */
#define REG_AGC_CFG_105                 17694 /*(INT)(0.27*65536)*/  /* agc1_min */
#define REG_AGC_CFG_106                 42598//45875 /*(INT)(0.70*65536)*/  /* agc2_max */
#define REG_AGC_CFG_107                 2621  /*(INT)(0.04*65536)*/  /* agc2_min */

#define REG_AGC_CFG_108                 76    /*((int)(0*256)<<8         | (int)(0.20*256*1.5))   */ /* agc1_pt1 pt2 */
#define REG_AGC_CFG_109                 13371 /*((int)(64*1.24/1.5)<<8   | (int)(64*1.24/1.5+7))  */ /* agc1_sl1 sl2 */
#define REG_AGC_CFG_110                 27564 /*((int)(0.28*256*1.5)<<8  | (int)(0.45*256*1.5))   */ /* agc2_pt1 pt2 */
#define REG_AGC_CFG_111                 14662 /*((int)(64*1.53/1.5-8)<<8 | (int)((64*1.53/1.5+5)))*/ /* agc2_sl1 sl2 */

/****************************************************************************
 * PLL Configuration for COFDM BW_MHz = 6.000000
 ****************************************************************************/
//Internal clock frequency = 60.000000 MHz
#define REG_CLK_6_0_CFG_VCXO            279620
#define REG_CLK_6_0_INTERNAL            60000
#define REG_CLK_6_0_SMPL                30000
#define REG_CLK_6_0_CFG_910             39          /* Version SDK */
//#define REG_CLK_6_0_CFG_907  4448                 /* Version SDK */
#define REG_CLK_6_0_CFG_907             4192        /* Version PB - fixed with dvbt_driver */
#define REG_CLK_6_0_CFG_928             53772
//Tuner intermediate frequency = 36.125000 MHz
#define REG_CLK_6_0_CFG_21              918   //921
#define REG_CLK_6_0_CFG_22              55000 //39083
#define REG_CLK_6_0_CFG_23              234         //Sampling clock frequency = 30.000000 MHz
#define REG_CLK_6_0_CFG_24              3745
/****************************************************************************
 * PLL Configuration for COFDM BW_MHz = 7.000000
 ****************************************************************************/
//Internal clock frequency = 60.000000 MHz
#define REG_CLK_7_0_CFG_VCXO            279620
#define REG_CLK_7_0_INTERNAL            60000
#define REG_CLK_7_0_SMPL                30000
#define REG_CLK_7_0_CFG_910             39          /* Version SDK */
//#define REG_CLK_7_0_CFG_907  4448                 /* Version SDK */
#define REG_CLK_7_0_CFG_907             4192        /* Version PB - fixed with dvbt_driver */
#define REG_CLK_7_0_CFG_928             53772
//Tuner intermediate frequency = 36.125000 MHz
#define REG_CLK_7_0_CFG_21              918   // 919
#define REG_CLK_7_0_CFG_22              55000 // 30583
#define REG_CLK_7_0_CFG_23              273         //Sampling clock frequency = 30.000000 MHz
#define REG_CLK_7_0_CFG_24              4369
/****************************************************************************
 * PLL Configuration for COFDM BW_MHz = 8.000000
 ****************************************************************************/
//Internal clock frequency = 60.000000 MHz
#define REG_CLK_8_0_CFG_VCXO            279620
#define REG_CLK_8_0_INTERNAL            60000
#define REG_CLK_8_0_SMPL                30000
#define REG_CLK_8_0_CFG_910             39          /* Version SDK */
//#define REG_CLK_8_0_CFG_907  4448                 /* Version SDK */
#define REG_CLK_8_0_CFG_907             4192        /* Version PB - fixed with dvbt_driver */
#define REG_CLK_8_0_CFG_928             53772
//Tuner intermediate frequency = 36.125000 MHz
#define REG_CLK_8_0_CFG_21              918   //919
#define REG_CLK_8_0_CFG_22              55000 // 30583
#define REG_CLK_8_0_CFG_23              312 //Sampling clock frequency = 30.000000 MHz
#define REG_CLK_8_0_CFG_24              4993

struct DIBDVBChannel
{
    DWORD RfKHz;
    BYTE Bw;
    int   nfft;
    int   guard;
    int   invspec;
    int   nqam;
    int   intlv_native;
    int   vit_hrch;
    int   vit_select_hp;
    int   vit_alpha;
    int   vit_code_rate_hp;
    int   vit_code_rate_lp;
};

#endif
