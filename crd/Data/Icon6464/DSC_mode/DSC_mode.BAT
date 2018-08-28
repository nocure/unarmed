@echo off
rem Convert PVR1300 icon
rem create icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

rem ..\bmp2pat /f4 /p ..\dsc21clut.pal /t0 icon_cadture.bmp 	>> ..\icon_yeh.h
rem ..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_cadture_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_flash.bmp 			>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_flash_c.bmp	 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_lock.bmp		 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_lock_c.bmp 		>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_lock_image.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_lock_image_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_selftimer.bmp 		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_selftimer_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f8  /p ..\dsc21clut.pal /t0 icon_lock_32_c.bmp		>> ..\icon_yeh.h

echo ..\bmp2pat batch ...Finish!
@echo on