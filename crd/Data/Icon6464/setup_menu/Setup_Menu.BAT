@echo off
rem Convert PVR1300 icon
rem create icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_auto_power_off.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_auto_power_off_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal 	icon_auto_power_off_w.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_CF.bmp 				>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_CF_c.bmp		 		>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_date_time.bmp			>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_date_time_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_information.bmp		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_information_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_NTSC_PAL.bmp			>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_NTSC_PAL_c.bmp			>> ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_reset_default.bmp 		>> ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_reset_default_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_card_capacity.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_card_capacity_c.bmp	>> ..\icon_yeh.h

echo ..\bmp2pat batch ...Finish!
@echo on