/*
 * ucon_sdtest.c
 *
 *  Created on: 2008/12/12
 *      Author: Wesley Wang
 */

#include <sct.h>
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>
#include "../bsp/2440addr.h"
#include <string.h>

#define write_size	1024
BYTE write_buf[write_size];
BYTE read_buf[write_size];



void ucon_sdtest(int argc, char** argv) {
int ret;
int i;
MMCSD_CARD_T card;
char* cmdbuf;

	for (i=0;i<write_size;i++) {
		write_buf[i]=(i+0x40)&0xff;
	}
	memset(read_buf,0,write_size);

// ==================================================================

	tq_sdcEnable();

	memset(&card,0,sizeof(MMCSD_CARD_T));

	card.ocr_vdd=SD_OCR_VDD_3_2;		// 0x100000 for voltage range 3.2-3.3, or 0xff8000 for all ranges

	ret=tq_sdIdentify(&card);
	if (ret) {
		printf("NO card found\n");
		return;
	}

	printf("manfid: %d\n",card.cid.manfid);
	printf("product name: %s\n",card.cid.prod_name);
	printf("serial: %08x\n",card.cid.serial);
	printf("oemid: %d\n",card.cid.oemid);
	printf("hwrev: %d\n",card.cid.hwrev);
	printf("fwrev: %d\n",card.cid.fwrev);
	printf("year: %d\n",card.cid.year);
	printf("month: %d\n\n",card.cid.month);

	printf("max_dtr: %dkHz\n",card.csd.max_dtr/1000);
	printf("capacity: %d sectors\n\n",card.csd.capacity);


//	ret=tq_sdWriteBlock(&card,write_buf,0x0,2);
//	if (ret) printf("write %d\n",ret);

	ret=tq_sdReadBlock(&card,read_buf,0x0,1);
	if (ret) printf("read %d\n",ret);
	cmdbuf=uconGetCmdBuf();
	sprintf(cmdbuf,"d -b 0x%08x 0x200",read_buf);
	uconExec();

//	ret=memcmp(read_buf,write_buf,write_size);
//	if (ret) printf("read/write error\n");
/*
	// deselect
	ret=yl_sdCMD7(0,&status);
	if (ret) printf("CMD7 %d %d\n",ret);


	ret=yl_sdCMD0();
	yl_sdcDisable();
*/

	return;
}
