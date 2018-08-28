#include <string.h>
#include <stdlib.h>
#include "2440addr.h"
#include "tq_hw_defs.h"
#include "platform/YL_SYS.h"
#include <platform/yl_errno.h>
#include "platform/TQ_BSP.h"

extern 	int yl_uartPrintf(const char *format, ...);

#if 0
	#define I2C_printf yl_uartPrintf
#else
	#define I2C_printf(...)
#endif

//#define I2C_DELAY dly_tsk(1)
#define I2C_DELAY tq_i2cDelay()
#define I2C_SLAVE_ADD (0x10)//slave address 1010 check AT24C02A device addressing

#define I2c_CON_ACK_EN (1<<7)//acknowledge enable
#define I2c_CON_TXCLK (1<<6)//source clock IIC bus transmit clock prescaler
#define I2C_CON_INT_EN (1<<5)//Tx/Rx interrupt enable
#define I2C_CON_INT_FLAG (1<<4)//Tx/Rx interrupt pending flag
#define I2C_CON_TCLK_VAL (0xf)//Tranmit clock value



#define I2c_STAT_MASTER (1<<7)//0:slave 1:master
#define I2c_STAT_TRAN (1<<6)//0:recieve 1: transmit
#define I2c_STAT_BUSY (1<<5)//busy signal status
#define I2c_STAT_OUTPUT (1<<4)//data output enable
#define I2c_STAT_ARB_STAT (1<<3)//arbitration procedure status flag
#define I2c_STAT_AS_SLAVE (1<<2)//address as slave status flag
#define I2c_STAT_ADDZERO_STAT (1<<1)//address zero status flag
#define I2c_STAT_LAST_REC (1)//last received bit status flag


#define I2c_CLC_FILTER_EN (1<<2)//filter enable
#define I2c_CLC_DELAY (1)//SDA line delay length selection, 5 clocks

#define I2c_SDA (2<<30)
#define I2c_SCL (2<<28)

static const int g_i2cTimeOut=100;	// 100 ms

static void tq_i2cDelay(void) {
int i;
	for (i=0;i<6000;i++);
}

static int  IicInt(void)
{

	I2C_printf("%s enter\n",__func__);

	U32 iicSt;//,i;

// no need to clear pending bits, it's done in assembly
//	rSRCPND = BIT_IIC;			//Clear pending bit
//	rINTPND = BIT_IIC;
	iicSt	= rIICSTAT;

	if(iicSt & I2c_STAT_ARB_STAT)//When bus arbitration is failed.
	{
		I2C_printf("%s I2c_STAT_ARB_STAT\n",__func__);
	}
	if(iicSt & I2c_STAT_AS_SLAVE){
		I2C_printf("%s I2c_STAT_AS_SLAVE\n",__func__);
	} 		  //When a slave address is matched with IICADD
	if(iicSt & I2c_STAT_ADDZERO_STAT){
		I2C_printf("%s I2c_STAT_ADDZERO_STAT\n",__func__);
	} 		  //When a slave address is 0000000b
	if(iicSt & I2c_STAT_LAST_REC){
		I2C_printf("%s I2c_STAT_LAST_REC\n",__func__);
	} 		  //When ACK isn't received

	BIT_SET(rSRCPND,BIT_IIC);
	BIT_SET(rINTPND,BIT_IIC);
	return 0;
}


static int tq_i2cWaitBUSY(void){
	unsigned long timeout=OSTICK+g_i2cTimeOut;
	while(BIT_VAL(rIICSTAT,I2c_STAT_BUSY)){
		if (time_after(OSTICK,timeout))
			return FAILED;
		I2C_DELAY;
	}
	return 0;
}


static int tq_i2cWaitNACK(void) {
	unsigned long timeout=OSTICK+g_i2cTimeOut;
	while( BIT_VAL(rIICSTAT,I2c_STAT_LAST_REC) ) {
		if (time_after(OSTICK,timeout))
			return FAILED;
		I2C_DELAY;
	}
	return 0;
}


void tq_i2cInit(void) {

	//set GPE15:IICSDA , GPE14:IICSCL
	BIT_CLR(rGPECON,(0xf<<28));
	BIT_SET(rGPECON,(I2c_SDA|I2c_SCL));

	//set interrupt vector table
	tq_irqSetISR(27, IicInt);
	BIT_CLR(rINTMSK,BIT_IIC);

}


void tq_i2cEnable(void) {
	//enable i2c power
	BIT_SET(rCLKCON,CLK_CON_IIC);

	I2C_DELAY;

	//Enable ACK, Prescaler IICCLK=PCLK/16, Enable interrupt, Transmit clock value Tx clock=IICCLK/16
	// If PCLK 50.7MHz, IICCLK = 3.17MHz, Tx Clock = 0.198MHz
	//rIICCON = (1<<7) | (0<<6) | (1<<5) | (0xf);
	rIICCON = I2c_CON_ACK_EN | I2C_CON_INT_EN | I2C_CON_TCLK_VAL;
	rIICADD  = I2C_SLAVE_ADD;
	rIICSTAT = I2c_STAT_OUTPUT;
	rIICLC = I2c_CLC_FILTER_EN|I2c_CLC_DELAY;
}


void tq_i2cDisable(void) {
	BIT_SET(rINTMSK,BIT_IIC);
	//disable i2c power
	BIT_CLR(rCLKCON,CLK_CON_IIC);
}


int tq_i2cStart(U8 devid){
	int i =0;
	rIICCON  = I2c_CON_ACK_EN|I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	rIICDS	 = devid;
	for(i=0;i<10;i++);
	rIICSTAT = I2c_STAT_MASTER|I2c_STAT_TRAN|I2c_STAT_BUSY|I2c_STAT_OUTPUT;
	I2C_DELAY;
	if(tq_i2cWaitNACK())
		return FAILED;
	else
		return SUCCESS;
}


int tq_i2cWrite(U8 data) {
	int i =0;
	rIICDS	= data;
	for(i=0;i<10;i++);
	rIICCON = I2c_CON_ACK_EN|I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	I2C_DELAY;
	if(tq_i2cWaitNACK())
		return FAILED;
	else
		return SUCCESS;
}


int tq_i2cWriteStop(void) {
	int i;
	rIICSTAT = I2c_STAT_MASTER|I2c_STAT_TRAN|I2c_STAT_OUTPUT;
	for(i=0;i<10;i++);
	rIICCON = I2c_CON_ACK_EN|I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	I2C_DELAY;
	I2C_DELAY;
	if(tq_i2cWaitBUSY())
		return FAILED;
	else
		return SUCCESS;
}


int tq_i2cRead(U8 devid,U8 *data) {
	int i;
	rIICDS	 = devid;
	for(i=0;i<10;i++);

	rIICSTAT = I2c_STAT_MASTER|I2c_STAT_BUSY|I2c_STAT_OUTPUT;
	for(i=0;i<10;i++);
	rIICCON    = I2c_CON_ACK_EN|I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	I2C_DELAY;
	rIICCON =  I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	I2C_DELAY;
	*data = (U8)rIICDS;
	for(i=0;i<10;i++);
	return 0;
}


int tq_i2cReadStop(void) {
	rIICSTAT = I2c_STAT_MASTER|I2c_STAT_OUTPUT;
	rIICCON =  I2C_CON_INT_EN|I2C_CON_TCLK_VAL;
	I2C_DELAY;
	I2C_DELAY;
	if(tq_i2cWaitBUSY())
		return FAILED;
	else
		return SUCCESS;
}




