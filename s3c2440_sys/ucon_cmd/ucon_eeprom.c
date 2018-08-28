#include <string.h>
#include <stdlib.h>
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/yl_errno.h>

#define AT24C02A_SIZE 256
#define AT24C02A_ID 0xa0

int yl_PromWriteByte(DWORD id,DWORD slvAdd,DWORD data){
	if(tq_i2cStart((U8)id))
		return FAILED;

	if(tq_i2cWrite((U8)slvAdd))
		return FAILED;

	if(tq_i2cWrite((U8)data))
		return FAILED;

	if(tq_i2cWriteStop())
		return FAILED;

	return SUCCESS;
}


int yl_PromReadByte(DWORD id,DWORD slvAdd,U8 *data){

	if(tq_i2cStart((U8)id))
		return FAILED;

	if(tq_i2cWrite((U8)slvAdd))
		return FAILED;

	if(tq_i2cRead((U8)id,data))
		return FAILED;

	if(tq_i2cReadStop())
		return FAILED;

	return SUCCESS;
}


int yl_PromRead(U8 *data,DWORD rsize){
	if((data==NULL)||(rsize<0)||(rsize>AT24C02A_SIZE)){
		yl_uartPrintf("%s invalid command\n",__func__);
		return FAILED;
	}
	int i=0;
	for(i=0;i<rsize;i++){
		if(yl_PromReadByte(AT24C02A_ID,i,&data[i])){
			yl_uartPrintf("%s error\n",__func__);
			return FAILED;
		}
	}
	return SUCCESS;
}


int yl_PromWrite(U8 *data,DWORD rsize){
	if((data==NULL)||(rsize<0)||(rsize>AT24C02A_SIZE)){
		yl_uartPrintf("%s invalid command\n",__func__);
		return FAILED;
	}
	int i=0;
	for(i=0;i<rsize;i++){
		if(yl_PromWriteByte(AT24C02A_ID,i,data[i])){
			yl_uartPrintf("%s error\n",__func__);
			return FAILED;
		}
	}
	return SUCCESS;
}




void uconI2C(int argc, char** argv){
	if (argc>=2) {
		if(strcmp(argv[1],"read")==0){
			tq_i2cEnable();
			U8 data[AT24C02A_SIZE];
			memset(data,0,AT24C02A_SIZE);
			yl_PromRead(data,AT24C02A_SIZE);
			int i=0,j=0;
			for(i=0;i<16;i++){
				for(j=0;j<16;j++)
					yl_uartPrintf("%2x ",data[i*16+j]);
				yl_uartPrintf("\n");
			}
			tq_i2cDisable();
		}
		else if(strcmp(argv[1],"write")==0){
			tq_i2cEnable();

			U8 Wdata = 0xff;
			if(argv[2]!=NULL){
				Wdata = (U8)atoi(argv[2]);
			}
			U8 data[AT24C02A_SIZE];
			memset(data,Wdata,AT24C02A_SIZE);
			yl_PromWrite(data,AT24C02A_SIZE);
			tq_i2cDisable();
		}
		else{
			yl_uartPrintf("unknow command\n");
		}
	}
}
