#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include "2440addr.h"
#include "tq_hw_defs.h"
#define RTCEN (1)//RTC control enable
#define CLKSEL (0<<1)//BCD clock select
#define CLKRST (1<<3)//clock reset
#define RSHIFT 4

#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
//#define YEAR_BASE	2008
#define YEAR_BASE	2000
#define BASE_WDAY      6
#define MONSPERYEAR	12
#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

static int rlastsec = 0; //for rtc show time
//typedef long			time_t;
static const int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
} ;

static const int year_lengths[2] = {
  365,
  366
} ;

extern 	int yl_uartPrintf(const char *format, ...);

static int tq_rtcValidTimeCheck(struct tm* pt) {
	if(pt==0)
		return -1;
	//check input data
	if((pt->tm_sec<0)||(pt->tm_sec>59)){
		yl_uartPrintf("!!invalid sec\n");
		return -1;
	}
	if((pt->tm_min<0)||(pt->tm_min>59)){
		yl_uartPrintf("!!invalid min\n");
		return -1;
	}
	if((pt->tm_hour<0)||(pt->tm_hour>23)){
		yl_uartPrintf("!!invalid hour\n");
		return -1;
	}

	if((pt->tm_mon<0)||(pt->tm_mon>12)){
		yl_uartPrintf("!!invalid month\n");
		return -1;
	}
	if((pt->tm_year<YEAR_BASE)||(pt->tm_year>(YEAR_BASE+99))){
		yl_uartPrintf("!!invalid year\n");
		return -1;
	}
	int yleap = isleap(pt->tm_year);
	if((pt->tm_mday<0)||(pt->tm_mday>mon_lengths[yleap][(pt->tm_mon-1)])){
		yl_uartPrintf("!!invalid day of month\n");
		return -1;
	}
	return 0;
}


static int tq_rtcGetYearDays(int year, int mon, int day) {
	if((year<0)||(mon<0)||(mon>12)||(day<0)){
		return 0;
	}
	int i=0;
	int yleap = isleap(year);
	int yearday = 0;

	for(i=0;i<(mon-1);i++){
		yearday = yearday + mon_lengths[yleap][i];
	}
	yearday = yearday + day;
	return yearday;
}


static int tq_rtcGetWeekDay(int year, int mon, int day) {
	if((year<0)||(mon<0)||(mon>12)||(day<0)){
		return 0;
	}

	int i=0;
	int ybase = YEAR_BASE;
	int bwday =BASE_WDAY;
	int leap = 0;
	int today = 0;

	if(year>YEAR_BASE){
		//year base week day
		int unit = year-YEAR_BASE;
		for(i=0;i<unit;i++){
			leap = isleap(ybase);
			bwday = (bwday + (year_lengths[leap]%7))%7;
			ybase++;
			yl_uartPrintf("index::%d, twdays:%d\n",i,bwday);
		}
		today = tq_rtcGetYearDays(year,mon,day);
		bwday = (bwday + today - 1)%7;
	}
	else if(year==YEAR_BASE){
		today = tq_rtcGetYearDays(year,mon,day);
		bwday = (bwday + today - 1)%7;
	}
	else{
	}
	return bwday;
}


int tq_rtcSetTime(struct tm* pt) {
	if(pt==0)
		return -1;

	if(tq_rtcValidTimeCheck(pt)){
		return -1;
	}

	int ten;
	int unit;

	BIT_SET(rRTCCON,RTCEN);

	//sec
	unit = pt->tm_sec%10;
	ten = pt->tm_sec/10;
	rBCDSEC = unit | (ten<<RSHIFT);

	//min
	unit = pt->tm_min%10;
	ten = pt->tm_min/10;
	rBCDMIN = unit | (ten<<RSHIFT);

	//hour
	unit = pt->tm_hour%10;
	ten = pt->tm_hour/10;
	rBCDHOUR = unit | (ten<<RSHIFT);

	//date of mon
	unit = pt->tm_mday%10;
	ten = pt->tm_mday/10;
	rBCDDATE = unit | (ten<<RSHIFT);

	//month
	unit = pt->tm_mon%10;
	ten = (pt->tm_mon/10);
	rBCDMON = unit | (ten<<RSHIFT);

	//year  0~99
	int baseyear = 0;
	if(pt->tm_year<YEAR_BASE){
		unit = 0;
		baseyear = YEAR_BASE;
	}
	else{
		unit = (pt->tm_year-YEAR_BASE)%100;
		baseyear = pt->tm_year;
	}
	rBCDYEAR = unit;
	rBCDDAY = tq_rtcGetWeekDay(baseyear, pt->tm_mon, pt->tm_mday);

	BIT_CLR(rRTCCON,RTCEN);

	return 0;
}


int tq_rtcGetTime(struct tm* pt) {
	if(pt==0)
		return -1;

	int ten;
	int unit;

	rRTCCON = RTCEN;//rtc write enable
	ten = ((0x70&rBCDSEC)>>RSHIFT)*10;
	unit = (0x0f&rBCDSEC);
	pt->tm_sec = ten +unit;

	ten = ((0x70&rBCDMIN)>>RSHIFT)*10;
	unit = (0x0f&rBCDMIN);
	pt->tm_min= ten +unit;

	ten = ((0x30&rBCDHOUR)>>RSHIFT)*10;
	unit = (0x0f&rBCDHOUR);
	pt->tm_hour= ten +unit;

	ten = ((0x30&rBCDDATE)>>RSHIFT)*10;
	unit = (0x0f&rBCDDATE);
	pt->tm_mday= ten +unit;

	unit = (0x0f&rBCDDAY);
	pt->tm_wday = unit;

	ten = ((0x10&rBCDMON)>>RSHIFT)*10;
	unit = (0x0f&rBCDMON);
	//pt->tm_mon= (0x0f&rBCDMON);
	pt->tm_mon= ten +unit;

	unit = (0xff&rBCDYEAR);
	pt->tm_year = YEAR_BASE + unit;

#ifdef RTC_SUPPORT_YEARDAY
	pt->tm_yday = tq_rtcGetYearDays(pt->tm_year, pt->tm_mon, pt->tm_mday);
#endif
	rRTCCON &= ~(RTCEN);//rtc write disable

	return 0;
}

void tq_rtcInit(void) {
	BIT_SET(rCLKCON,CLK_CON_RTC);//rtc power enable
	rRTCCON = 0x00;
/*
	struct tm a;
	a.tm_sec = 1;
	a.tm_min = 41;
	a.tm_hour = 18;
	//a.tm_wday = 2;
	a.tm_mday = 20;
	a.tm_mon = 1;
	//a.tm_yday;
	a.tm_year = 2010;
	//a.tm_isdst;

	RTCSetTime(&a);
*/
}


void tq_rtcShowTime(void) {
	struct tm rtime;
	tq_rtcGetTime(&rtime);
#ifdef RTC_SUPPORT_YEARDAY
	if(rlastsec != rtime.tm_sec){
		yl_uartPrintf("y:%d, m:%d, d:%d, h:%d, m:%d, s:%d, wday:%d, yday:%d\n",
			rtime.tm_year,rtime.tm_mon,rtime.tm_mday,rtime.tm_hour,rtime.tm_min
			,rtime.tm_sec, rtime.tm_wday,rtime.tm_yday);
		rlastsec = rtime.tm_sec;
	}
#else
	if(rlastsec != rtime.tm_sec){
		yl_uartPrintf("y:%d, m:%d, d:%d, h:%d, m:%d, s:%d, wday:%d\n",
			rtime.tm_year,rtime.tm_mon,rtime.tm_mday,rtime.tm_hour,rtime.tm_min
			,rtime.tm_sec, rtime.tm_wday);
		rlastsec = rtime.tm_sec;
	}

#endif

}

