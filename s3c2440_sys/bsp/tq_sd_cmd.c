/*
 * yl_sd_cmd.c
 *
 *  Created on: 2008/12/16
 *      Author: ww
 */
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "2440addr.h"
#include "tq_hw_defs.h"

// ==============================================================================
// ==== SDI Commands ============================================================
// ==============================================================================
/*
 * CMD0 --> none GO_IDLE_STATE
Asks any card to send the CID numbers
on the CMD line (any card that is
connected to the host will respond)
[31:0] stuff bits
*/
void tq_sdCMD0(void) {

	tq_sdcSendCmd(0,0);	// send cmd0, stuff bit as arguemnt
	tq_sdcWaitNoRsp(); 	// wait

}


/*
 * CMD1 --> R3 SEND_OP_COND
MMC
Window(OCR[23:0])
*/
int tq_sdCMD1(DWORD arg, DWORD *pOCR) {
int ret;

	tq_sdcSendCmd(1|SDC_CMD_WAIT_RSP,arg);
	ret=tq_sdcWaitRsp(pOCR);	// wait R3
	if (ret==ERR_SDC_CRC) ret=0;

	return ret;
}


/*
 * CMD2 --> R2 ALL_SEND_CID
Asks any card to send the CID numbers
on the CMD line (any card that is
connected to the host will respond)
[31:0] stuff bits
 */
int tq_sdCMD2(DWORD* pCID) {
int ret;

	tq_sdcSendCmd(2|SDC_CMD_LONG_RSP,0);
	ret=tq_sdcLongRsp(pCID);	// wait R2
	if (ret<0) return ret;

	return 0;
}


/*
 * CMD3 --> R6 SEND_RELATIVE_ADDR
Ask the card to publish a new relative
address (RCA)
[31:0] stuff bits
*/
int tq_sdCMD3(DWORD arg, DWORD* pStatus) {
int ret;

	tq_sdcSendCmd(3|SDC_CMD_WAIT_RSP,arg);
	ret=tq_sdcWaitRsp(pStatus);	// wait R6
	if (ret<0) return ret;
	if (ret!=3) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
 * CMD7 --> R1b/none SELECT/DESELECT_CARD
Command toggles a card between the
stand-by and transfer states or between
the programming and disconnect states.
In both cases, the card is selected by its
own relative address and gets deselected
by any other address; address 0 deselects
all. In the case that the RCA equals 0,
then the host may do one of the following:
- Use other RCA number to perform
card de-selection.
- Re-send CMD3 to change its RCA
number to other than 0 and then use
CMD7 with RCA=0 for card deselection.
[31:16] RCA
[15:0] stuff bits
 */
int tq_sdCMD7(DWORD RCA,DWORD *pstatus) {
int ret;

	if (RCA) {
		tq_sdcPrepareWait(WAIT_BUSY|WAIT_AFTER_CMD,0);

		tq_sdcSendCmd(7|SDC_CMD_WAIT_RSP,RCA);
		ret=tq_sdcWaitRsp(pstatus);	// wait R1b
		if (ret<0) goto out;
		if (ret!=7) {
			ret= ERR_SDC_CMD_MISMATCH;
			goto out;
		}
		ret=tq_sdcWaitEnd();
//		ret=yl_sdcWaitBusy();
		if (ret) goto out;

		ret=0;
	}
	else {
		tq_sdcSendCmd(7,0);
		tq_sdcWaitNoRsp();	// wait
		*pstatus=0;
		ret=0;
	}
	out:
	return ret;
}


/*
 * CMD8 --> R7 SNED_IF_COND
Sends SD Memory Card interface
condition, which includes host supply
voltage information and asks the card
whether card supports voltage. Reserved
bits shall be set to '0'.
[31:12]reserved bits
[11:8]supply voltage(VHS)
[7:0]check pattern
 */
int tq_sdCMD8(void) {
int ret;
DWORD rsp;

#define VHS	1	/* 0001b 2.7-3.6V */
#define CHECK_PATTERN	0xAA	/* recommended by spec */
	tq_sdcSendCmd(8|SDC_CMD_WAIT_RSP,VHS<<8|CHECK_PATTERN);
	ret=tq_sdcWaitRsp(&rsp);	// wait R7
	if (ret<0) return ret;
	if (ret!=8) return ERR_SDC_CMD_MISMATCH;

	if ( (((rsp>>8)&0xff)!=VHS) || ((rsp&0xff)!=CHECK_PATTERN) ) return ERR_SDC_IF_COND;

	return 0;
}


/*
 * CMD9 --> R2 SEND_CSD
Addressed card sends its card-specific
data (CSD) on the CMD line.
[31:16] RCA
[15:0] stuff bits
 */
int tq_sdCMD9(DWORD RCA,DWORD* pCSD) {
int ret;

	tq_sdcSendCmd(9|SDC_CMD_LONG_RSP,RCA);
	ret=tq_sdcLongRsp(pCSD);	// wait R2
	if (ret<0) return ret;

	return 0;
}


/*
 * CMD10 --> R2 SEND_CID
Addressed card sends its card identification
(CID) on CMD the line.
[31:16] RCA
[15:0] stuff bits
 */
int tq_sdCMD10(DWORD RCA,DWORD* pCID) {
int ret;

	tq_sdcSendCmd(10|SDC_CMD_LONG_RSP,RCA);
	ret=tq_sdcLongRsp(pCID);	// wait R2
	if (ret<0) return ret;

	return 0;
}


/*
CMD12 --> R1b STOP_TRANSMISSION
Forces the card to stop transmission
[31:0] stuff bits
 */
int tq_sdCMD12(DWORD* pstatus) {
int ret;

	tq_sdcPrepareWait(WAIT_BUSY|WAIT_AFTER_CMD,0);

	tq_sdcSendCmd(12|SDC_CMD_WAIT_RSP,0);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) goto out;
	if (ret!=12) {
		ret=ERR_SDC_CMD_MISMATCH;
		goto out;
	}
	ret=tq_sdcWaitEnd();
//	ret=tq_sdcWaitBusy();
	if (ret) goto out;

	ret=0;
out:
	return ret;
}


/*
 * CMD13 --> R1 SEND_STATUS
Addressed card sends its status
register
[31:16] RCA
[15:0] stuff bits
*/
int tq_sdCMD13(DWORD RCA, DWORD *pstatus) {
int ret;

	tq_sdcSendCmd(13|SDC_CMD_WAIT_RSP,RCA);
	ret=tq_sdcWaitRsp(pstatus);	// wait R6
	if (ret<0) return ret;
	if (ret!=13) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
CMD16 --> R1 SET_BLOCKLEN
In the case of a Standard Capacity SD
Memory Card, this command sets the
block length (in bytes) for all following
block commands (read, write, lock).
Default block length is fixed to 512 Bytes.
Set length is valid for memory access
commands only if partial block read
operation are allowed in CSD.
In the case of a High Capacity SD
Memory Card, block length set by CMD16
command does not affect the memory
read and write commands. Always 512
Bytes fixed block length is used. This
command is effective for LOCK_UNLOCK
command.
In both cases, if block length is set larger
than 512Bytes, the card sets the
BLOCK_LEN_ERROR bit.
[31:0] block length
*/
int tq_sdCMD16(DWORD length, DWORD* pstatus) {
int ret;
	tq_sdcSendCmd(16|SDC_CMD_WAIT_RSP,length);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) return ret;
	if (ret!=16) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
CMD17 --> R1 READ_SINGLE_BLOCK
In the case of a Standard Capacity SD
Memory Card, this command, this
command reads a block of the size
selected by the SET_BLOCKLEN
command.
In the case of a High Capacity Card, block
length is fixed 512 Bytes regardless of the
SET_BLOCKLEN command.
[31:0] data address
 */
int tq_sdCMD17(DWORD addr, DWORD* pstatus) {
int ret;
	tq_sdcSendCmd(17|SDC_CMD_WAIT_RSP,addr);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) return ret;
	if (ret!=17) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
CMD18 --> R1 READ_MULTIPLE_BLOCK
Continuously transfers data blocks from
card to host until interrupted by a
STOP_TRANSMISSION command.
Block length is specified the same as
READ_SINGLE_BLOCK command.
[31:0] data address
 */
int tq_sdCMD18(DWORD addr, DWORD* pstatus) {
int ret;
	tq_sdcSendCmd(18|SDC_CMD_WAIT_RSP,addr);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) return ret;
	if (ret!=18) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
CMD24 --> R1 WRITE_BLOCK
In the case of a Standard Capacity SD
Memory Card, this command writes a
block of the size selected by the
SET_BLOCKLEN command.
In the case of a High Capacity Card,
block length is fixed 512 Bytes regardless
of the SET_BLOCKLEN command.
[31:0] data address
 */
int tq_sdCMD24(DWORD addr, DWORD* pstatus) {
int ret;
	tq_sdcSendCmd(24|SDC_CMD_WAIT_RSP,addr);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) return ret;
	if (ret!=24) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


/*
CMD25 --> R1 WRITE_MULTIPLE_BLOCK
Continuously writes blocks of data until a
STOP_TRANSMISSION follows.
Block length is specified the same as
WRITE_BLOCK command.
[31:0] data address
 */
int tq_sdCMD25(DWORD addr, DWORD* pstatus) {
int ret;
	tq_sdcSendCmd(25|SDC_CMD_WAIT_RSP,addr);
	ret=tq_sdcWaitRsp(pstatus);
	if (ret<0) return ret;
	if (ret!=25) return ERR_SDC_CMD_MISMATCH;

	return 0;
}


// =================================================================================
// =================================================================================
// =================================================================================
/*
ACMD6 --> R1 SET_BUS_WIDTH
Defines the data bus width (・00・=1bit
or ・10・=4 bits bus) to be used for data
transfer. The allowed data bus widths are
given in SCR register.
[31:2] stuff bits
[1:0]bus width
*/
int tq_sdACMD6(DWORD RCA, DWORD arg, DWORD *prsp) {
int ret;

	tq_sdcSendCmd(55|SDC_CMD_WAIT_RSP,RCA);	// send cmd0x55, stuff bit as arguemnt
	ret=tq_sdcWaitRsp(prsp);	// wait R1
	if (ret<0) return ret;
	if (ret!=55) return ERR_SDC_CMD_MISMATCH;

	tq_sdcSendCmd(6|SDC_CMD_WAIT_RSP,arg);
	ret=tq_sdcWaitRsp(prsp);	// wait R1
	if (ret<0) return ret;
	if (ret!=6) return ERR_SDC_CMD_MISMATCH;

	return 0;
}



/*
 * ACMD41 --> R3 SD_SEND_OP_COND
Sends host capacity support information
(HCS) and asks the accessed card to
send its operating condition register
(OCR) content in the response on the
CMD line. HCS is effective when card
receives SEND_IF_COND command.
Reserved bit shall be set to ．0・. CCS bit
is assigned to OCR[30].
[31]reserved bit
[30]HCS(OCR[30])
[29:24]reserved bits
[23:0] VDD Voltage
Window(OCR[23:0])
*/
int tq_sdACMD41(DWORD RCA, DWORD arg, DWORD *pOCR) {
int ret;

	tq_sdcSendCmd(55|SDC_CMD_WAIT_RSP,RCA);	// send cmd0x55, stuff bit as arguemnt
	ret=tq_sdcWaitRsp(pOCR);	// wait R1
	if (ret<0) return ret;
	if (ret!=55) return ERR_SDC_CMD_MISMATCH;

	tq_sdcSendCmd(41|SDC_CMD_WAIT_RSP,arg);
	ret=tq_sdcWaitRsp(pOCR);	// wait R3
	if (ret==ERR_SDC_CRC) ret=0;

	return ret;
}



