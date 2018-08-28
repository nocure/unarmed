/******************************************************************************
 * sct.c -- System Configuration Table
 * ============================================================================
 *
 *
 * 2008-0828 ww created
 *
 *
 *(\~~/)(\~~/)
 *('.' )(^.^ )
 * (_(__ (_(__)~*
 *****************************************************************************/
#define _INSIDE_SCT_C_
#include "sct.h"
// ===================================================================
#define HANDLER(x) void x(DWORD thread_input);

// === Define thread prototypes ========================================
HANDLER(init_tsk);
HANDLER(ucon_tsk);
HANDLER(key_tsk);
HANDLER(gui_tsk);
HANDLER(hmi_tsk);
HANDLER(motor_tsk);
HANDLER(wakeup_tsk);
HANDLER(ethdm9k_tsk);

HANDLER(cycSystemTickHandler);
HANDLER(cycPiliLEDHandler);
HANDLER(cycEXTIOPollHandler);
HANDLER(cycADPollHandler);

HANDLER(cycScrClockHandler);



// === Task =============================================================
#define TSK(id,func,stk,atr_pri)	{&id,func,stk,atr_pri},
const TSK_DEF_T info_tsk[]={
//  TASKID				task handler		stack size			atr/pri (0~31)
//=====================================================================
TSK(TSKID_UCON,			ucon_tsk,			DEFAULT_STACK_SIZE*5,		10|TSK_NEEDLPI)
TSK(TSKID_HMI,			hmi_tsk,			DEFAULT_STACK_SIZE,			15)
TSK(TSKID_GUI,			gui_tsk,			DEFAULT_STACK_SIZE,			20)
TSK(TSKID_MOTOR,		motor_tsk,			DEFAULT_STACK_SIZE,			16)
TSK(TSKID_WAKEUP,		wakeup_tsk,			DEFAULT_STACK_SIZE,			17)
TSK(TSKID_DM9K_DRV,		ethdm9k_tsk,		DEFAULT_STACK_SIZE*2,		18)
TSK(TSKID_INIT,			init_tsk,			DEFAULT_STACK_SIZE,			31|TSK_REVMEM|TSK_START)
{0}							// end of task definitions
};
#undef TSK


// === Flag =============================================================
#define FLG(id,iflgptn)	{&id,iflgptn},
const FLG_DEF_T info_evf[]={
//  FLAGID				initial pattern
//=====================================================================
FLG(FLGID_SDI_INT,		0)
FLG(FLGID_DM9K_INT,		0)
{0}							// end of event flag definitions
};
#undef FLG


// === Semaphore =============================================================
#define SEM(id,isemcnt,maxsem)	{&id,isemcnt,maxsem},
const SEM_DEF_T info_sem[]={
//  SEMID					init count		max count
//=====================================================================
SEM(SEMID_UARTPRINTF,			1,			1)
SEM(SEMID_MTX_TIMER,			1,			1)
SEM(SEMID_PRIVMEM,				1,			1)
{0}										// end of semaphore definitions
};
#undef SEM

// === Mutex ================================================================
#define MTX(id)	{&id},
const MTX_DEF_T info_mtx[]={
//  MTXID
//=====================================================================
{0}									// end of mutex definitions
};
#undef MTX


// === Mail Box =============================================================
#define MBX(id,bufcnt)	{&id,bufcnt},
const MBX_DEF_T info_mbx[]={
//  MBXID					buffer count
//=====================================================================
MBX(MBXID_UART_RECV,		300)
MBX(MBXID_HMI,				10)
MBX(MBXID_MOTOR_DRV,		10)
MBX(MBXID_WAKEUP_RECV,		10)
MBX(MBXID_GUI,				10)
{0}									// end of semaphore definitions
};


// === Alarm =========================================================
const ALM_DEF_T info_alm[]={
	{0}									// end of Alarm definitions
#if 0
	{ ALMID,							// Alarm ID
	{	NULL,							// Extened infomation.
		TA_HLNG,						// Alarm attributes. TA_HLNG
		almhdr,							// Alarm Handler Address.
		tmmode,							// Alarm Handler Activation
		almtim							// Alarm time.
	}},
#endif
};


// === Cyclic =========================================================
#define CYC(id,hdr,tim,act)	{&id,hdr,tim,act},
const CYC_DEF_T info_cyc[]={
//  CYCID				cyclic handler		cycle time				activation
//=====================================================================
CYC(CYCID_SYSTEMTICK,	cycSystemTickHandler,	10,					TCY_OFF)
CYC(CYCID_PILI_LED,		cycPiliLEDHandler,		66,					TCY_OFF)
CYC(CYCID_EXTIO_POLL,	cycEXTIOPollHandler,	50,					TCY_OFF)
CYC(CYCID_AD_POLL,		cycADPollHandler,		20,					TCY_OFF)
CYC(CYCID_SCR_CLOCK,	cycScrClockHandler,		1000,				TCY_OFF)
{0}									// end of Cyclic definitions
};

