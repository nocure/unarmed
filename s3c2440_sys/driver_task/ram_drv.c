/*
 * fs_drv.c
 *
 *  Created on: 2018¦~8¤ë28¤é
 *      Author: nocure
 */
#include <ff.h>
#include <diskio.h>
#include <string.h>

#define RAMDRV_IMAGE_START	_binary_vfat_ima_start
#define RAMDRV_IMAGE_SIZE	_binary_vfat_ima_size
#define RAMDRV_SEC_SIZE		512

extern DWORD RAMDRV_IMAGE_START;
extern DWORD RAMDRV_IMAGE_SIZE;

static BYTE *RAM_disk_ptr;

int RAM_disk_status(void) {
	return 0;
}

int RAM_disk_ioctl(BYTE cmd, void* buf);
int RAM_disk_write(BYTE* buf, DWORD sector, UINT count);
int RAM_disk_read(BYTE* buf, DWORD sector, UINT count);
int RAM_disk_initialize(void);
int RAM_disk_initialize(void) {
	RAM_disk_ptr=(BYTE*)RAMDRV_IMAGE_START;
	return 0;
}

int RAM_disk_read(BYTE* buf, DWORD sector, UINT count) {
	memcpy(buf,(RAM_disk_ptr+sector*RAMDRV_SEC_SIZE),(count*RAMDRV_SEC_SIZE) );
	return 0;
}

int RAM_disk_write(BYTE* buf, DWORD sector, UINT count) {
	memcpy((RAM_disk_ptr+sector*RAMDRV_SEC_SIZE),buf,(count*RAMDRV_SEC_SIZE) );
	return 0;
}

int RAM_disk_ioctl(BYTE cmd, void* buf) {
int ret;
DWORD *pi;
	pi=(DWORD*)buf;

	switch(cmd) {
	case GET_SECTOR_COUNT:
		*pi=RAMDRV_IMAGE_SIZE/RAMDRV_SEC_SIZE;
		ret=0;
		break;

	case GET_SECTOR_SIZE:
	case GET_BLOCK_SIZE:
		*pi=RAMDRV_SEC_SIZE;
		ret=0;
		break;

	case CTRL_SYNC:
		ret=0;
		break;

	case CTRL_TRIM:
	default:
		ret=-1;
		break;

	}

	return ret;

}


int MMC_disk_status(void) {
	return 0;
}

int MMC_disk_initialize(void) {
	return 0;
}

int MMC_disk_read(BYTE* buf, DWORD sector, UINT count) {
	return 0;
}

int MMC_disk_write(BYTE* buf, DWORD sector, UINT count) {
	return 0;
}



int USB_disk_status(void) {
	return 0;
}

int USB_disk_initialize(void) {
	return 0;
}

int USB_disk_read(BYTE* buf, DWORD sector, UINT count) {
	return 0;
}

int USB_disk_write(BYTE* buf, DWORD sector, UINT count) {
	return 0;
}

