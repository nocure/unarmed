/*
 * yl_sd_ctlr.c
 *
 *  Created on: 2008/12/12
 *      Author: Wesley Wang
 */

#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "2440addr.h"
#include "tq_hw_defs.h"
#include <sct.h>

// ==============================================================================
// ==== SD Controller ===========================================================
// ==============================================================================
#if 1
void tq_sdcInit(void) {
	BIT_CLR(rCLKCON,CLK_CON_SDI);	// power down
}


void tq_sdcEnable(void) {
	BIT_SET(rCLKCON,CLK_CON_SDI);	// power up
	BIT_SET(rSDICON,SDC_CON_RESET);	// reset SDC
	rSDIDTIMER=0x7fffff;			// Set timeout count
	rSDIBSIZE=SD_BLOCK_LENGTH;
	BIT_SET(rSDICON,SDC_CON_ENABLE);	// enable sdc clock
	dly_tsk(1);	// Wait 74SDCLK for card
}


void tq_sdcDisable(void) {
	rSDICON=0;						// disable sdc
	BIT_CLR(rCLKCON,CLK_CON_SDI);	// power down
}


void tq_sdcSetBusSpeed(int speed) {
	rSDIPRE= (PCLK/speed)-1;
}


void tq_sdcSendCmd(DWORD con, DWORD arg) {
int stat;
	rSDICARG=arg;		// stuff bits
	rSDICCON=con|SDC_CMD_START_BIT|SDC_CMD_START;

	// better to use interrupt and OS waiting functions to prevent hanging
	do {
		stat=rSDICSTA;
	} while (BIT_VAL(stat,SDC_CSTA_IN_PROGRESS));

}


void tq_sdcWaitNoRsp(void) {
DWORD stat;

	do {
		stat=rSDICSTA;
	} while (!BIT_VAL(stat,SDC_CSTA_CMD_SENT));

	BIT_SET(rSDICSTA,stat&(0
			|SDC_CSTA_CMD_SENT
		)
	);		// clear status
}


// R1 R1b R6 R7
int tq_sdcWaitRsp(DWORD *rsp) {
DWORD stat;
int ret;

	do {
		stat=rSDICSTA;
	} while (!BIT_VAL(stat,SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT));

	if (BIT_VAL(stat,SDC_CSTA_CMD_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	*rsp=rSDIRSP0;

	if (BIT_VAL(stat,SDC_CSTA_CRC_FAIL)) {
		ret=ERR_SDC_CRC; // crc failed
		goto out;
	}
	ret=stat&0x3f;

out:
	BIT_SET(rSDICSTA,stat&(0
			|SDC_CSTA_CRC_FAIL
			|SDC_CSTA_CMD_SENT
			|SDC_CSTA_CMD_TIMEOUT
			|SDC_CSTA_RSP_END
		)
	);		// clear status
	return ret;
}


// R2 R3
int tq_sdcLongRsp(DWORD* rsp) {
DWORD stat;
int ret;

	do {
		stat=rSDICSTA;
	} while (!BIT_VAL(stat,SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT));

	if (BIT_VAL(stat,SDC_CSTA_CMD_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	*rsp++=rSDIRSP0;
	*rsp++=rSDIRSP1;
	*rsp++=rSDIRSP2;
	*rsp=rSDIRSP3;

	if (BIT_VAL(stat,SDC_CSTA_CRC_FAIL)) {
		ret=ERR_SDC_CRC; // CRC failed
		goto out;
	}

	ret=stat&0x3f;
out:
	BIT_SET(rSDICSTA,stat&(0
			|SDC_CSTA_CRC_FAIL
			|SDC_CSTA_CMD_SENT
			|SDC_CSTA_CMD_TIMEOUT
			|SDC_CSTA_RSP_END
		)
	);		// clear status
	return ret;
}


void tq_sdcPrepareWait(DWORD opt,DWORD cnt) {
DWORD con;

	con=SDC_DCON_BLOCK_MODE|SDC_DCON_TRANSFER_START;

	if (BIT_VAL(opt,WAIT_BUSY)) {
		con|=SDC_DCON_BUSY_CHECK;
		if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
			con|=SDC_DCON_BUSY_AFTER_CMD;
		}
	}
	else {
		BIT_SET(rSDIFSTA,SDC_FSTA_FIFO_RESET|SDC_FSTA_FIFO_FAIL);	// reset fifo & clear fail status

		if (BIT_VAL(opt,WAIT_TRANSMIT)) {
			con|=SDC_DCON_4BYTE_TRANSFER|SDC_DCON_TRANSMIT|cnt;
			if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
				con|=SDC_DCON_TX_AFTER_RSP;
			}
		}
		else if (BIT_VAL(opt,WAIT_RECEIVE)) {
			con|=SDC_DCON_4BYTE_TRANSFER|SDC_DCON_RECEIVE|cnt;
			if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
				con|=SDC_DCON_RX_AFTER_CMD;
			}
		}

		if (BIT_VAL(opt,WAIT_4BIT)) {
			con|=SDC_DCON_WIDE_BUS;
		}
	}

	rSDIDCON=con;

}


// wait busy on data line
int tq_sdcWaitEnd(void) {
DWORD stat;
int ret;

	do {
		stat=rSDIDSTA;
	} while (!BIT_VAL(stat,SDC_DSTA_NO_BUSY|SDC_DSTA_TX_CRC_FAIL|SDC_DSTA_RX_CRC_FAIL|SDC_DSTA_DATA_TIMEOUT|SDC_DSTA_DATA_FINISH|SDC_DSTA_BUSY_FINISH));

	if (BIT_VAL(stat,SDC_DSTA_DATA_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	if (BIT_VAL(stat,SDC_DSTA_TX_CRC_FAIL|SDC_DSTA_RX_CRC_FAIL)) {
		ret=ERR_SDC_CRC;
		goto out;
	}

	ret=0;
out:
	BIT_SET(rSDIDSTA,stat&(0
			|SDC_DSTA_NO_BUSY
			|SDC_DSTA_TX_CRC_FAIL
			|SDC_DSTA_RX_CRC_FAIL
			|SDC_DSTA_DATA_TIMEOUT
			|SDC_DSTA_DATA_FINISH
			|SDC_DSTA_BUSY_FINISH
		)
	);		// clear status

	rSDIDCON=0;
	return ret;
}


// cnt is number of DWORD to transmit
int tq_sdcTransmit(void *buf,DWORD cnt) {
	while (cnt--) {
		while(!BIT_VAL(rSDIFSTA,SDC_FSTA_TX_READY|SDC_FSTA_FIFO_FAIL));
		if (BIT_VAL(rSDIFSTA,SDC_FSTA_FIFO_FAIL)) return ERR_SDC_FIFO;
		rSDIDAT=*((DWORD*)buf);
		buf+=4;
	}
	return 0;
}


// cnt is number of DWORD to receive
int tq_sdcReceive(void *buf,DWORD cnt) {
	while (cnt--) {
		while(!BIT_VAL(rSDIFSTA,SDC_FSTA_RX_READY|SDC_FSTA_FIFO_FAIL));
		if (BIT_VAL(rSDIFSTA,SDC_FSTA_FIFO_FAIL)) return ERR_SDC_FIFO;
		*((DWORD*)buf)=rSDIDAT;
		buf+=4;
	}
	return 0;
}
#endif

#if 0


int tq_sdcISR(void) {
DWORD cstat,dstat;
DWORD flg;

	cstat=rSDICSTA;
	dstat=rSDIDSTA;

	flg=(dstat<<16)|cstat;
	if (!flg) goto out0;
	set_flg(FLGID_SDI_INT,flg);

	BIT_SET(rSDICSTA,cstat&(SDC_CSTA_CMD_SENT|SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT|SDC_CSTA_CRC_FAIL));
	BIT_SET(rSDIDSTA,dstat&(SDC_DSTA_NO_BUSY|SDC_DSTA_BUSY_FINISH));
//	BIT_SET(rSDICSTA,cstat);
//	BIT_SET(rSDIDSTA,dstat);

out0:
	BIT_SET(rSRCPND,BIT_SDI);
	BIT_SET(rINTPND,BIT_SDI);

	return 0;
}

void tq_sdcInit(void) {
	BIT_CLR(rCLKCON,CLK_CON_SDI);	// power down
	tq_irqSetISR(INT_SDI,tq_sdcISR);
}


void yl_sdcEnable(void) {
	BIT_SET(rCLKCON,CLK_CON_SDI);	// power up
	BIT_SET(rSDICON,SDC_CON_RESET);	// reset SDC
	rSDIDTIMER=0x7fffff;			// Set timeout count
	rSDIBSIZE=SD_BLOCK_LENGTH;
	BIT_SET(rSDICON,SDC_CON_ENABLE);	// enable sdc clock
	dly_tsk(1);	// Wait 74SDCLK for card
//	rSDIIMSK=SDI_INT_RspEnd|SDI_INT_CmdTout|SDI_INT_CmdSent|SDI_INT_RspCrc;
	rSDIIMSK=0;
	EnableIrq(BIT_SDI);
}


void tq_sdcDisable(void) {
	DisableIrq(BIT_SDI);
	rSDIIMSK=0;
	rSDICON=0;						// disable sdc
	BIT_CLR(rCLKCON,CLK_CON_SDI);	// power down
}


void tq_sdcSetBusSpeed(int speed) {
	rSDIPRE= (PCLK/speed)-1;
}


void tq_sdcSendCmd(DWORD con, DWORD arg) {

	if (BIT_VAL(con,SDC_CMD_WAIT_RSP)) {
		BIT_SET(rSDIIMSK,SDI_INT_RspEnd|SDI_INT_CmdTout);
	}
	else {
		BIT_SET(rSDIIMSK,SDI_INT_CmdSent);
	}
	rSDICARG=arg;		// stuff bits
	rSDICCON=con|SDC_CMD_START_BIT|SDC_CMD_START;
}


void tq_sdcWaitNoRsp(void) {
ULONG flg;

	wai_flg(&flg,FLGID_SDI_INT,SDC_CSTA_CMD_SENT,TWF_ORW|TWF_CLR);
	clr_flg(FLGID_SDI_INT,~(SDC_CSTA_CMD_SENT|0xff));
	BIT_CLR(rSDIIMSK,SDI_INT_CmdSent);

}


// R1 R1b R6 R7
int tq_sdcWaitRsp(DWORD *rsp) {
ULONG flg;
int ret;

	ret=wai_flg(&flg,FLGID_SDI_INT,SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT,TWF_ORW|TWF_CLR);
	ret=clr_flg(FLGID_SDI_INT,~(SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT|SDC_CSTA_CMD_TIMEOUT|SDC_CSTA_CRC_FAIL|0xff));

	if (BIT_VAL(flg,SDC_CSTA_CMD_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	*rsp=rSDIRSP0;

	if (BIT_VAL(flg,SDC_CSTA_CRC_FAIL)) {
		ret=ERR_SDC_CRC; // crc failed
		goto out;
	}
	ret=flg&0x3f;

out:
	BIT_CLR(rSDIIMSK,SDI_INT_RspEnd|SDI_INT_CmdTout);
	return ret;
}


// R2 R3
int tq_sdcLongRsp(DWORD* rsp) {
ULONG flg;
int ret;

	ret=wai_flg(&flg,FLGID_SDI_INT,SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT,TWF_ORW|TWF_CLR);
	ret=clr_flg(FLGID_SDI_INT,~(SDC_CSTA_RSP_END|SDC_CSTA_CMD_TIMEOUT|SDC_CSTA_CMD_TIMEOUT|SDC_CSTA_CRC_FAIL|0xff));

	if (BIT_VAL(flg,SDC_CSTA_CMD_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	*rsp++=rSDIRSP0;
	*rsp++=rSDIRSP1;
	*rsp++=rSDIRSP2;
	*rsp=rSDIRSP3;

	if (BIT_VAL(flg,SDC_CSTA_CRC_FAIL)) {
		ret=ERR_SDC_CRC; // CRC failed
		goto out;
	}

	ret=flg&0x3f;

out:
	BIT_CLR(rSDIIMSK,SDI_INT_RspEnd|SDI_INT_CmdTout);
	return ret;
}


void tq_sdcPrepareWait(DWORD opt,DWORD cnt) {
DWORD con;

	con=SDC_DCON_BLOCK_MODE|SDC_DCON_TRANSFER_START;

	if (BIT_VAL(opt,WAIT_BUSY)) {
		con|=SDC_DCON_BUSY_CHECK;
		if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
			con|=SDC_DCON_BUSY_AFTER_CMD;
		}
		BIT_SET(rSDIIMSK,SDI_INT_NoBusy|SDI_INT_BusyFin);	// set up interrupt
	}
	else {
		BIT_SET(rSDIFSTA,SDC_FSTA_FIFO_RESET|SDC_FSTA_FIFO_FAIL);	// reset fifo & clear fail status

		if (BIT_VAL(opt,WAIT_TRANSMIT)) {
			con|=SDC_DCON_4BYTE_TRANSFER|SDC_DCON_TRANSMIT|cnt;
			if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
				con|=SDC_DCON_TX_AFTER_RSP;
			}
		}
		else if (BIT_VAL(opt,WAIT_RECEIVE)) {
			con|=SDC_DCON_4BYTE_TRANSFER|SDC_DCON_RECEIVE|cnt;
			if (BIT_VAL(opt,WAIT_AFTER_CMD)) {
				con|=SDC_DCON_RX_AFTER_CMD;
			}
		}

		if (BIT_VAL(opt,WAIT_4BIT)) {
			con|=SDC_DCON_WIDE_BUS;
		}
	}

	rSDIDCON=con;

}


// wait busy on data line
int tq_sdcWaitBusy(void) {
int ret;
ULONG flg;

	ret=wai_flg(&flg,FLGID_SDI_INT,(SDC_DSTA_NO_BUSY|SDC_DSTA_BUSY_FINISH)<<16,TWF_ORW|TWF_CLR);
	ret=clr_flg(FLGID_SDI_INT,~((SDC_DSTA_NO_BUSY|SDC_DSTA_BUSY_FINISH)<<16));

	ret=0;
out:
	BIT_CLR(rSDIIMSK,SDI_INT_NoBusy|SDI_INT_BusyFin);
	rSDIDCON=0;
	return ret;
}


// wait busy on data line
int tq_sdcWaitEnd(void) {
DWORD stat;
int ret;

	do {
		stat=rSDIDSTA;
	} while (!BIT_VAL(stat,SDC_DSTA_DATA_TIMEOUT|SDC_DSTA_DATA_FINISH));

	if (BIT_VAL(stat,SDC_DSTA_DATA_TIMEOUT)) {
		ret=ERR_SDC_TIMEOUT; // no rsp ???
		goto out;
	}

	if (BIT_VAL(stat,SDC_DSTA_TX_CRC_FAIL|SDC_DSTA_RX_CRC_FAIL)) {
		ret=ERR_SDC_CRC;
		goto out;
	}

	ret=0;
out:
	BIT_SET(rSDIDSTA,stat&(0
			|SDC_DSTA_TX_CRC_FAIL
			|SDC_DSTA_RX_CRC_FAIL
			|SDC_DSTA_DATA_TIMEOUT
			|SDC_DSTA_DATA_FINISH
		)
	);		// clear status

	rSDIDCON=0;
	return ret;
}


// cnt is number of DWORD to transmit
int tq_sdcTransmit(void *buf,DWORD cnt) {
	while (cnt--) {
		while(!BIT_VAL(rSDIFSTA,SDC_FSTA_TX_READY|SDC_FSTA_FIFO_FAIL));
		if (BIT_VAL(rSDIFSTA,SDC_FSTA_FIFO_FAIL)) return ERR_SDC_FIFO;
		rSDIDAT=*((DWORD*)buf);
		buf+=4;
	}
	return 0;
}


// cnt is number of DWORD to receive
int tq_sdcReceive(void *buf,DWORD cnt) {
	while (cnt--) {
		while(!BIT_VAL(rSDIFSTA,SDC_FSTA_RX_READY|SDC_FSTA_FIFO_FAIL));
		if (BIT_VAL(rSDIFSTA,SDC_FSTA_FIFO_FAIL)) return ERR_SDC_FIFO;
		*((DWORD*)buf)=rSDIDAT;
		buf+=4;
	}
	return 0;
}


#endif
