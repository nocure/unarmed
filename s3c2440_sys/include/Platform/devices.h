/*
 * devices.h
 *
 *  Created on: 2010/10/15
 *      Author: Wesley Wang
 */

#ifndef DEVICES_H_
#define DEVICES_H_

// ==== ram drv ===============================================
int ramdrvSystemInit(int drvnum, void* ext);

int mmcsdSystemInit(int drvnum);


#endif /* DEVICES_H_ */
