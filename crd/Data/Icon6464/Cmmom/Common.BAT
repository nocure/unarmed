@echo off
rem Convert PVR1300 icon
rem create icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f4 	/p ..\dsc21clut.pal     icon_down.bmp		> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_return.bmp		>> ..\icon_yeh.h
..\bmp2pat /f8	/p ..\dsc21clut.pal /t0 icon_return_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t235 icon_select_1.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t235 icon_select_2.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t235 icon_select_3.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t235 icon_select_4.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal     icon_top.bmp		>> ..\icon_yeh.h


echo ..\bmp2pat batch ...Finish!
@echo on