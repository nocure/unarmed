/*
 * ScreenSimKey.h
 *
 *  Created on: 2009/3/20
 *      Author: Wesley Wang
 */

#ifndef TOUCHSCREEN_H_
#define TOUCHSCREEN_H_

#define TSKEY_DOWN		0x01
#define TSKEY_UP		0x02

void tsPrepareKeyScreen(void);
int tsPushPointToFifo(int x,int y);
void tsGetScreenXY(int *x,int *y);
int tsIsInsideKeyRange(int x,int y);
void tsDrawkey(U32 Key,U32 kmode);
void tsGenHMIEvent(int x,int y);

#endif /* TOUCHSCREEN_H_ */
