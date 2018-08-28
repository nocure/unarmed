@echo off
rem Convert PVR1300 icon
rem create icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f4   /p ..\dsc21clut.pal /t0 icon_banding.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16  /p ..\dsc21clut.pal /t0 icon_banding_c.bmp >> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_exposure.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_exposure_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_quality.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_quality_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_resolution.bmp >> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_resolution_c.bmp >> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_setup.bmp 		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_setup_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_WB.bmp 		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_WB_c.bmp 		>> ..\icon_yeh.h

echo ..\bmp2pat batch ...Finish!
@echo on