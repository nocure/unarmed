#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "platform/YL_SYS.h"
#include "2440addr.h"
#include "mmu.h"

#include "uda134x.h"

static unsigned int rates[33 * 2];
struct uda134x_priv {
	int sys_clk;
	int dai_fmt;
	int direction;
};
static struct uda134x_priv uda134x;

U16	audioBuffer[AUDIO_BUFSIZE];

/* In-data addresses are hard-coded into the reg-cache values */
static unsigned char uda134x_reg[UDA134X_REGS_NUM] = {
	/* Extended address registers */
	0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* Status, data regs */
	0x00, 0x83, 0x00, 0x40, 0x80, 0x00,
};

static int uda134x_write(unsigned int reg, unsigned int value);

/*
 * The codec has no support for reading its registers except for peak level...
 */
static inline unsigned int uda134x_read_reg_cache(unsigned int reg)
{
	U8 *cache = uda134x_reg;

	if (reg >= UDA134X_REGS_NUM)
		return -1;
	return cache[reg];
}

/*
 * Write the register cache
 */
static inline void uda134x_write_reg_cache(U8 reg, unsigned int value)
{
	U8 *cache = uda134x_reg;

	if (reg >= UDA134X_REGS_NUM)
		return;
	cache[reg] = value;
}

static inline void uda134x_reset(void)
{
	U8 reset_reg = uda134x_read_reg_cache(UDA134X_STATUS0);
	uda134x_write(UDA134X_STATUS0, reset_reg | (1<<6));
	mdelay(1);
	uda134x_write(UDA134X_STATUS0, reset_reg & ~(1<<6));
}


/*
 * Write to the uda134x registers
 *
 */
static int uda134x_write(unsigned int reg, unsigned int value)
{
	int ret;
	U8 addr;
	U8 data = value;
	if (reg >= UDA134X_REGS_NUM) {
		printf("%s unkown register: reg: %d", __func__, reg);
		return YLERR_INVAL;
	}

	uda134x_write_reg_cache(reg, value);

	switch (reg) {
	case UDA134X_STATUS0:
	case UDA134X_STATUS1:
		addr = UDA134X_STATUS_ADDR;
		break;
	case UDA134X_DATA000:
	case UDA134X_DATA001:
	case UDA134X_DATA010:
		addr = UDA134X_DATA0_ADDR;
		break;
	case UDA134X_DATA1:
		addr = UDA134X_DATA1_ADDR;
		break;
	default:
		/* It's an extended address register */
		addr =  (reg | UDA134X_EXTADDR_PREFIX);

		ret = l3_write(UDA134X_DATA0_ADDR, &addr, 1);
		if (ret != 1)
			return YLERR_IO;

		addr = UDA134X_DATA0_ADDR;
		data = (value | UDA134X_EXTDATA_PREFIX);
		break;
	}

	ret = l3_write(addr, &data, 1);
	if (ret != 1)
		return YLERR_IO;

	return 0;
}

int uda134x_mute(int mute)
{
	U8 mute_reg = uda134x_read_reg_cache(UDA134X_DATA010);

	if (mute)
		mute_reg |= (1<<2);
	else
		mute_reg &= ~(1<<2);

	uda134x_write(UDA134X_DATA010, mute_reg & ~(1<<2));

	return 0;
}


int uda134x_init(void)
{

	uda134x_reset();

	/* TODO: add controls for uda134x, ex. volumn */
#if 0
	ret = uda134x_add_controls(codec);
	if (ret < 0) {
		printk(KERN_ERR "UDA134X: failed to register controls\n");
		goto pcm_err;
	}

	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "UDA134X: failed to register card\n");
		goto card_err;
	}
#endif

	return 0;
}

// ===================================================================================


#define S3C24XX_PRESCALE(a,b)	\
	(((a - 1) << S3C2410_IISPSR_INTSHIFT) | ((b - 1) << S3C2410_IISPSR_EXTSHFIT))

#define UDA134x_UDA1340	1
#define UDA134x_UDA1341	2
#define UDA134x_UDA1344	3

U8	codec_model;

int tq_uda134x_startup(int dir)
{

	unsigned long xtal = 12 * 1000 * 1000;
	int	i, j;

	uda134x.direction = dir;

	for (i = 0; i < 2; i++) {
		int fs = i ? 256 : 384;

		rates[i*33] = xtal / fs;
		for (j = 1; j < 33; j++)
			rates[i*33 + j] = PCLK / (j * fs);
	}

	return 0;

}

/* TODO: use DMA */
void tq_uda134x_trigger(int cmd, int dir)
{
	tq_i2s_trigger(cmd, dir);

}

int tq_uda134x_hw_params(unsigned long rate, int format)
{
	unsigned int clk = 0;
	int ret = 0;
	int clk_source, fs_mode;
	long err, cerr;
	unsigned int div;
	int i, bi;
	U8 hw_params;

	err = 999999;
	bi = 0;

	for (i = 0; i < 2*33; i++) {
		cerr = rates[i] - rate;
		if (cerr < 0)
			cerr = -cerr;
		if (cerr < err) {
			err = cerr;
			bi = i;
		}
	}

	if (bi / 33 == 1)
		fs_mode = S3C2410_IISMOD_256FS;
	else
		fs_mode = S3C2410_IISMOD_384FS;
	if (bi % 33 == 0) {
		clk_source = S3C24XX_CLKSRC_MPLL;
		div = 1;
	} else {
		clk_source = S3C24XX_CLKSRC_PCLK;
		div = bi % 33;
	}

	clk = (fs_mode == S3C2410_IISMOD_384FS ? 384 : 256) * rate;
	printf("%s will use: %s %s %d sysclk %d err %ld\n", __func__,
		 fs_mode == S3C2410_IISMOD_384FS ? "384FS" : "256FS",
		 clk_source == S3C24XX_CLKSRC_MPLL ? "MPLLin" : "PCLK",
		 div, clk, err);

	if ((err * 100 / rate) > 5) {
		printf("S3C24XX_UDA134X: effective frequency too different from desired (%ld%%)\n",
		       err * 100 / rate);
		return YLERR_INVAL;
	}

	hw_params = uda134x_read_reg_cache(UDA134X_STATUS0);
	hw_params &= STATUS0_SYSCLK_MASK;
	hw_params &= STATUS0_DAIFMT_MASK;

//	uda134x.dai_fmt = SND_DAIFMT_I2S | SND_DAIFMT_NB_NF | SND_DAIFMT_CBS_CFS;
	uda134x.dai_fmt = PCM_FORMAT_S16LE;

	tq_i2s_set_fmt();
	tq_i2s_set_sysclk(clk_source);
	ret = tq_i2s_set_clkdiv(S3C24XX_DIV_MCLK, fs_mode);
	if (ret)
		return ret;

	ret = tq_i2s_set_clkdiv(S3C24XX_DIV_BCLK, S3C2410_IISMOD_32FS);
	if (ret)
		return ret;

	ret = tq_i2s_set_clkdiv(S3C24XX_DIV_PRESCALER, S3C24XX_PRESCALE(div,div));
	if (ret)
		return ret;

	uda134x.sys_clk = clk;
	/* set SYSCLK / fs ratio */
	switch (clk / rate) {
		case 512:
			break;
		case 384:
			hw_params |= (1<<4);
			break;
		case 256:
			hw_params |= (1<<5);
			break;
		default:
			printf("%s unsupported fs\n", __func__);
			return YLERR_INVAL;
	}

	uda134x_write(UDA134X_STATUS0, hw_params);

	ret = tq_i2s_hw_params(format);
	if (ret)
		return ret;



	return 0;
}

int tq_uda134x_init(void)
{
	codec_model = UDA134x_UDA1341;

	BIT_CLR(rGPBUP,(0x7 << 2));		/* L3_MODE:GPB2, L3_DATA:GPB3, L3_CLOCK:GPB4 */

	uda134x_init();
	tq_i2s_enable();

	return 0;
}

int tq_uda134x_exit(void)
{
	BIT_SET(rGPBUP,(0x7 << 2));		/* L3_MODE:GPB2, L3_DATA:GPB3, L3_CLOCK:GPB4 */

	return 0;
}

void l3_setmode(int v)
{
	if (v)							/* GPB2 */
		BIT_SET(rGPBDAT, 1 << 2);
	else
		BIT_CLR(rGPBDAT, 1 << 2);
}

void l3_setclk(int v)
{
	if (v)							/* GPB4 */
		BIT_SET(rGPBDAT, 1 << 4);
	else
		BIT_CLR(rGPBDAT, 1 << 4);
}

void l3_setdat(int v)
{
	if (v)							/* GPB4 */
		BIT_SET(rGPBDAT, 1 << 3);
	else
		BIT_CLR(rGPBDAT, 1 << 3);
}

