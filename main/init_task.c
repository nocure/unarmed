/*
 * init_task.c
 *
 *  Created on: 2008/12/10
 *      Author: ww
 */

#include "sct.h"
#include <platform/yl_sys.h>
#include <platform/yl_fsio.h>
#include <platform/tq_bsp.h>
#include <platform/devices.h>
#include <ucos_ii.h>
//#include <fx_api.h>
//=======================================================
#define RAMDRV_IMAGE_SYMBOL _binary_vfat_ima_start
extern unsigned long RAMDRV_IMAGE_SYMBOL;
//=======================================================

int osdInit(void);

//========================================================================================
static const UART_CONFIG_T g_UARTConfigInfo={
	.BaudRate=		115200,
//	.BaudRate=		57600,
	.DataBits=		UART_DATA_8_BIT,
	.Parity=		UART_PARITY_NONE,
	.StopBits=		UART_STOP_1_BIT,
	.FlowControl=	UART_FLOW_CTRL_NONE,
	.TimeOut=		2000,	// milli-seconds
};


static void hwInit(void) {

// power up gpio controller, many pins depend on gpio, i.e. UART, buzzer...
	tq_gpioInit();
//	tq_gpioSetOutput(LED_1|LED_2|LED_3|LED_4);

	tq_irqInit();

	tq_eintInit();

// init timer
	tq_timerInit();

// init uart for console
	tq_uartInit();
	tq_uartConfig((UART_CONFIG_T*)&g_UARTConfigInfo);
	tq_uartEnable();

// SD Card Controller
	tq_sdcInit();

// RTC
	tq_rtcInit();

// I2C
	tq_i2cInit();

//Touch screen
	tq_adcInit();

}

int InitGUI (int argc, const char *argv[]);
const char* const GUI_Settings[]={
		"RGB32:0",
		"240x180-32"
};
void init_tsk(DWORD thread_input) {
int er=thread_input;
struct tm tm;

	hwInit();	// hardware initialization

	OSStatInit();

	osdInit();
	er=InitGUI(2,(const char **)GUI_Settings);
	if (er) dbgout("OSD Init error\n");

	crdInit();

	tq_rtcGetTime(&tm);

	// set up random seed
	srand(tm.tm_year+tm.tm_mon+tm.tm_mday+tm.tm_hour+tm.tm_min+tm.tm_sec);

#if 0
	er =ramdrvSystemInit(0,&RAMDRV_IMAGE_SYMBOL);
//	er|=yl_dmMount(0);		// mount b:
	if (er) dbgout("RAMDRV Init error\n");

	er =mmcsdSystemInit(1);
	if (er) dbgout("MMCSD_DRV Init error\n");
#endif

	// Start tasks
	er = sta_tsk(TSKID_UCON, 0);

	er|= sta_tsk(TSKID_GUI, 0);
	er|= sta_tsk(TSKID_HMI, 0);

	er|= sta_tsk(TSKID_DM9K_DRV, 0);

	er|= act_cyc(CYCID_SYSTEMTICK,TCY_ON);	// start pili-led cyclic #1
	er|= act_cyc(CYCID_EXTIO_POLL,TCY_ON);	// External IO Polling
	er|= act_cyc(CYCID_AD_POLL,TCY_ON);		// ADC Polling (Touch screen and others)
	er|= act_cyc(CYCID_SCR_CLOCK,TCY_ON);	// Screen Clock
#ifndef __LUSIMUS__
	er|= act_cyc(CYCID_PILI_LED,TCY_ON);	// Pili LED
#endif

	if (er) dbgout("System Init Error\n");	// panic!!

	void post_init(void);
	post_init();

	exd_tsk();
}


