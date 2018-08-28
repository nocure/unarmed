/*
 * yl_sd_api.c
 *
 *  Created on: 2008/12/16
 *      Author: ww
 */
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "../bsp/2440addr.h"

// ==============================================================================
// ==== SDI Protocol/APIs ============================================================
// ==============================================================================

static const unsigned int tran_exp[] = {
	10000,		100000,		1000000,	10000000,
	0,			0,			0,			0
};

static const unsigned char tran_mant[] = {
	0,	10,	12,	13,	15,	20,	25, 30,
	35,	40, 45,	50,	55,	60,	70,	80,
};

static const unsigned int tacc_exp[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0, 	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

/*
 * Given the decoded CSD structure, decode the raw CID to our CID structure.
 */
static int mmc_decode_cid(MMCSD_CID *cid, DWORD *cxd, int ver) {

	/*
	 * The selection of the format here is based upon published
	 * specs from sandisk and from what people have reported.
	 */
	switch (ver) {

	case 0: /* MMC v1.0 - v1.2 */
	case 1: /* MMC v1.4 */
		cid->manfid = UNSTUFF_BITS(cxd, 104, 24);
		cid->prod_name[0] = UNSTUFF_BITS(cxd, 96, 8);
		cid->prod_name[1] = UNSTUFF_BITS(cxd, 88, 8);
		cid->prod_name[2] = UNSTUFF_BITS(cxd, 80, 8);
		cid->prod_name[3] = UNSTUFF_BITS(cxd, 72, 8);
		cid->prod_name[4] = UNSTUFF_BITS(cxd, 64, 8);
		cid->prod_name[5] = UNSTUFF_BITS(cxd, 56, 8);
		cid->prod_name[6] = UNSTUFF_BITS(cxd, 48, 8);
		cid->hwrev = UNSTUFF_BITS(cxd, 44, 4);
		cid->fwrev = UNSTUFF_BITS(cxd, 40, 4);
		cid->serial = UNSTUFF_BITS(cxd, 16, 24);
		cid->month = UNSTUFF_BITS(cxd, 12, 4);
		cid->year = UNSTUFF_BITS(cxd, 8, 4) + 1997;
		break;

	case 2: /* MMC v2.0 - v2.2 */
	case 3: /* MMC v3.1 - v3.3 */
	case 4: /* MMC v4 */
		cid->manfid = UNSTUFF_BITS(cxd, 120, 8);
		cid->oemid = UNSTUFF_BITS(cxd, 104, 16);
		cid->prod_name[0] = UNSTUFF_BITS(cxd, 96, 8);
		cid->prod_name[1] = UNSTUFF_BITS(cxd, 88, 8);
		cid->prod_name[2] = UNSTUFF_BITS(cxd, 80, 8);
		cid->prod_name[3] = UNSTUFF_BITS(cxd, 72, 8);
		cid->prod_name[4] = UNSTUFF_BITS(cxd, 64, 8);
		cid->prod_name[5] = UNSTUFF_BITS(cxd, 56, 8);
		cid->serial = UNSTUFF_BITS(cxd, 16, 32);
		cid->month = UNSTUFF_BITS(cxd, 12, 4);
		cid->year = UNSTUFF_BITS(cxd, 8, 4) + 1997;
		break;

	default:
		return -1;

	}
	return 0;
}
/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static int mmc_decode_csd(MMCSD_CSD *csd, DWORD *cxd) {

	unsigned int e, m, csd_struct;

	/*
	 * We only understand CSD structure v1.1 and v1.2.
	 * v1.2 has extra information in bits 15, 11 and 10.
	 */
	csd_struct = UNSTUFF_BITS(cxd, 126, 2);
	if (csd_struct != 1 && csd_struct != 2) { // unrecognised CSD structure version
		return -1;
	}

	csd->mmca_vsn = UNSTUFF_BITS(cxd, 122, 4);
	m = UNSTUFF_BITS(cxd, 115, 4);
	e = UNSTUFF_BITS(cxd, 112, 3);
	csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
	csd->tacc_clks = UNSTUFF_BITS(cxd, 104, 8) * 100;

	m = UNSTUFF_BITS(cxd, 99, 4);
	e = UNSTUFF_BITS(cxd, 96, 3);
	csd->max_dtr = tran_exp[e] * tran_mant[m];
	csd->cmdclass = UNSTUFF_BITS(cxd, 84, 12);

	e = UNSTUFF_BITS(cxd, 47, 3);
	m = UNSTUFF_BITS(cxd, 62, 12);
	csd->capacity = (1 + m) << (e + 2);

	csd->read_blkbits = UNSTUFF_BITS(cxd, 80, 4);
	csd->read_partial = UNSTUFF_BITS(cxd, 79, 1);
	csd->write_misalign = UNSTUFF_BITS(cxd, 78, 1);
	csd->read_misalign = UNSTUFF_BITS(cxd, 77, 1);
	csd->r2w_factor = UNSTUFF_BITS(cxd, 26, 3);
	csd->write_blkbits = UNSTUFF_BITS(cxd, 22, 4);
	csd->write_partial = UNSTUFF_BITS(cxd, 21, 1);

	return 0;
}


static int sd_decode_cid(MMCSD_CID *cid, DWORD *cxd) {

	/*
	 * SD doesn't currently have a version field so we will
	 * have to assume we can parse this.
	 */
	cid->manfid = UNSTUFF_BITS(cxd, 120, 8);
	cid->oemid = UNSTUFF_BITS(cxd, 104, 16);
	cid->prod_name[0] = UNSTUFF_BITS(cxd, 96, 8);
	cid->prod_name[1] = UNSTUFF_BITS(cxd, 88, 8);
	cid->prod_name[2] = UNSTUFF_BITS(cxd, 80, 8);
	cid->prod_name[3] = UNSTUFF_BITS(cxd, 72, 8);
	cid->prod_name[4] = UNSTUFF_BITS(cxd, 64, 8);
	cid->hwrev = UNSTUFF_BITS(cxd, 60, 4);
	cid->fwrev = UNSTUFF_BITS(cxd, 56, 4);
	cid->serial = UNSTUFF_BITS(cxd, 24, 32);
	cid->year = UNSTUFF_BITS(cxd, 12, 8);
	cid->month = UNSTUFF_BITS(cxd, 8, 4);

	cid->year += 2000; /* SD cards year offset */
	return 0;
}


/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static int sd_decode_csd(MMCSD_CSD *csd, DWORD *cxd) {
	unsigned int e, m, csd_struct;

	csd_struct = UNSTUFF_BITS(cxd, 126, 2);

	switch (csd_struct) {
	case 0:
		m = UNSTUFF_BITS(cxd, 115, 4);
		e = UNSTUFF_BITS(cxd, 112, 3);
		csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		csd->tacc_clks = UNSTUFF_BITS(cxd, 104, 8) * 100;

		m = UNSTUFF_BITS(cxd, 99, 4);
		e = UNSTUFF_BITS(cxd, 96, 3);
		csd->max_dtr = tran_exp[e] * tran_mant[m];
		csd->cmdclass = UNSTUFF_BITS(cxd, 84, 12);

		e = UNSTUFF_BITS(cxd, 47, 3);
		m = UNSTUFF_BITS(cxd, 62, 12);
		csd->capacity = (1 + m) << (e + 2);

		csd->read_blkbits = UNSTUFF_BITS(cxd, 80, 4);
		csd->read_partial = UNSTUFF_BITS(cxd, 79, 1);
		csd->write_misalign = UNSTUFF_BITS(cxd, 78, 1);
		csd->read_misalign = UNSTUFF_BITS(cxd, 77, 1);
		csd->r2w_factor = UNSTUFF_BITS(cxd, 26, 3);
		csd->write_blkbits = UNSTUFF_BITS(cxd, 22, 4);
		csd->write_partial = UNSTUFF_BITS(cxd, 21, 1);
		break;
	case 1:
		/*
		 * This is a block-addressed SDHC card. Most
		 * interesting fields are unused and have fixed
		 * values. To avoid getting tripped by buggy cards,
		 * we assume those fixed values ourselves.
		 */

		csd->tacc_ns = 0; /* Unused */
		csd->tacc_clks = 0; /* Unused */

		m = UNSTUFF_BITS(cxd, 99, 4);
		e = UNSTUFF_BITS(cxd, 96, 3);
		csd->max_dtr = tran_exp[e] * tran_mant[m];
		csd->cmdclass = UNSTUFF_BITS(cxd, 84, 12);

		m = UNSTUFF_BITS(cxd, 48, 22);
		csd->capacity = (1 + m) << 10;

		csd->read_blkbits = 9;
		csd->read_partial = 0;
		csd->write_misalign = 0;
		csd->read_misalign = 0;
		csd->r2w_factor = 4; /* Unused */
		csd->write_blkbits = 9;
		csd->write_partial = 0;
		break;
	default:
		return -1;
	}

	return 0;
}

// ======================================================================


int tq_sdWriteBlock(MMCSD_CARD_T *card, BYTE* buf,DWORD blknum,DWORD count) {
int ret;
DWORD status;

	tq_sdcPrepareWait(WAIT_TRANSMIT|WAIT_AFTER_CMD|(BIT_VAL(card->opmode,OPMODE_4BIT_BUS)?WAIT_4BIT:0),count);

	if (!(BIT_VAL(card->opmode,OPMODE_BLOCK_ADDRESS))) blknum*=card->blcok_length;	// standard capacity card
	if (count==1) {	// single
		ret=tq_sdCMD24(blknum,&status);
	}
	else { //multi
		ret=tq_sdCMD25(blknum,&status);
	}
	if (ret) goto out0;

	ret=tq_sdcTransmit(buf,count*(card->blcok_length/4));	// DWORD unit count
	if (ret) goto out0;

	ret=tq_sdcWaitEnd();	// wait data finish
	if (ret) goto out0;

	if (count==1) {	// single
	}
	else { //multi
		ret=tq_sdCMD12(&status);
		if (ret) goto out0;
	}
	ret=0;

out0:
	return ret;
}


int tq_sdReadBlock(MMCSD_CARD_T *card, BYTE* buf,DWORD blknum,DWORD count) {
int ret;
DWORD status;

	tq_sdcPrepareWait(WAIT_RECEIVE|WAIT_AFTER_CMD|(BIT_VAL(card->opmode,OPMODE_4BIT_BUS)?WAIT_4BIT:0),count);

	if (!(BIT_VAL(card->opmode,OPMODE_BLOCK_ADDRESS))) blknum*=card->blcok_length;	// standard capacity card
	if (count==1) {	// single
		ret=tq_sdCMD17(blknum,&status);
	}
	else { //multi
		ret=tq_sdCMD18(blknum,&status);
	}
	if (ret) goto out0;

	ret=tq_sdcReceive(buf,count*(card->blcok_length/4));	// DWORD unit count
	if (ret) goto out0;

	ret=tq_sdcWaitEnd();	// wait data finish
	if (ret) goto out0;

	if (count==1) {	// single
	}
	else { //multi
		ret=tq_sdCMD12(&status);
		if (ret) goto out0;
	}
	ret=0;

out0:
	return ret;
}


int tq_sdIdentify(MMCSD_CARD_T *card) {
int ret,i;
DWORD raw_cid[4];
DWORD raw_csd[4];
DWORD status;

	tq_sdcSetBusSpeed(SDC_SPEED_400K);

	tq_sdCMD0();	// go idle

	if (!card->ocr_vdd) card->ocr_vdd=SD_OCR_VDD_ALL;

	ret=tq_sdCMD8();	// check for SD2.0
	if (ret==ERR_SDC_TIMEOUT) { // failed, possible SD1.0 or mmc spec
		for (i=0;i<10;i++) {
			ret=tq_sdCMD1(card->ocr_vdd,&status);
			if (ret) break;
			if (BIT_VAL(status,SD_OCR_POWER_UP)) {
				card->type=CARD_TYPE_MMC;
				break;
			}
			dly_tsk(100);
		}

		if (!card->type) {	// not mmc, try if it's SD1.0
			for (i=0;i<10;i++) {
				ret=tq_sdACMD41(0,card->ocr_vdd, &status);		// RCA=0, HCS(30)=0, 0x100000 for voltage range 3.2-3.3, or 0xff8000 for all ranges
				if (BIT_VAL(status,SD_OCR_POWER_UP)) {
					card->type=CARD_TYPE_SD10;
					break;
				}
				dly_tsk(100);
			}
			if (ret) return ERR_SDC_FAILED;
		}
	}
	else if (!ret) { // must be SD2.0 (SDHC)
		for (i=0;i<10;i++) {
			ret=tq_sdACMD41(0,SD_OCR_CAPACITY|card->ocr_vdd, &status);	// RCA=0, HCS(30)=1, 0x100000 for voltage range 3.2-3.3, or 0xff8000 for all ranges
			if (BIT_VAL(status,SD_OCR_POWER_UP)) {
				card->type=CARD_TYPE_SD20;
				BIT_SET(card->opmode,OPMODE_BLOCK_ADDRESS);
				break;
			}
			dly_tsk(100);
		}
		if (ret) return ERR_SDC_FAILED;
	}
	else return  ERR_SDC_FAILED;;

	if (!card->type) return ERR_SDC_FAILED;

	ret=tq_sdCMD2(raw_cid);	// read CID
	if (ret) return ERR_SDC_FAILED;

	if (card->type==CARD_TYPE_MMC) {
		card->rca=((rand()%0xffff)+1)<<16;
		ret=tq_sdCMD3(card->rca,&status);	// MMC: set RCA
		if (ret) return ERR_SDC_FAILED;

		ret=tq_sdCMD9(card->rca,raw_csd);
		if (!ret) {
			ret=mmc_decode_csd(&card->csd,raw_csd);
			if (ret) return ERR_SDC_FAILED;
			mmc_decode_cid(&card->cid,raw_cid,card->csd.mmca_vsn);
		}
	}
	else {	// sd/sdhc
		ret=tq_sdCMD3(0,&card->rca);	// SD: ask for RCA
		card->rca&=0xffff0000;
		if (ret) return ERR_SDC_FAILED;

		ret=tq_sdCMD9(card->rca,raw_csd);
		if (!ret) {
			ret=sd_decode_csd(&card->csd,raw_csd);
			if (ret) return ERR_SDC_FAILED;
			sd_decode_cid(&card->cid,raw_cid);
		}
	}

	tq_sdcSetBusSpeed(card->csd.max_dtr);	// set maxisum allowed bus speed, step on it !!!

// --- After CMD3, The card's in transfer mode ----------------------

	ret=tq_sdCMD7(card->rca,&status);	// select the RCA
	if (ret) return ERR_SDC_FAILED;

	if (card->type==CARD_TYPE_MMC) {
		BIT_CLR(card->opmode,OPMODE_4BIT_BUS);	// clear 4bit opmode
	}
	else {
		ret=tq_sdACMD6(card->rca, 2, &status);	// set bus width 4 bit
		if (ret) return ret;
		BIT_SET(card->opmode,OPMODE_4BIT_BUS);	// set 4bit opmode
	}

	// setup block length
	ret=tq_sdCMD16(SD_BLOCK_LENGTH,&status);
	if (ret) return ret;
	if (CURRENT_STATE(status)!=STATE_TRAN) return -1;
	rSDIBSIZE=SD_BLOCK_LENGTH;		// tell the controller the block length
	card->blcok_length=SD_BLOCK_LENGTH;
/*
	for (i=0;i<10;i++) {
		ret=tq_sdCMD13(card->rca,&status);	// make sure that the card is in TRAN state
		if (ret) return ret;
		if (CURRENT_STATE(status)==STATE_TRAN) break;
	}
	if (CURRENT_STATE(status)!=STATE_TRAN) return -1;
*/
	// ==== calculate geometrics ====
	card->cyclinders = 1024;
	card->heads = 16;

	i = card->cyclinders * card->heads;
	card->sectors = card->csd.capacity / i;
	if (card->csd.capacity % i) {
		card->sectors++;
		i = card->cyclinders * card->sectors;
		card->heads = card->csd.capacity / i;

		if (card->csd.capacity % i) {
			card->heads++;
			i = card->heads * card->sectors;
			card->cyclinders = card->csd.capacity / i;
		}
	}

//	if ((card->cyclinders*card->heads*card->sectors)!=card->csd.capacity) {
//		return 1;
//	}

	return ret;
}


