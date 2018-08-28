@echo off
rem Convert PVR1300 icon
rem create icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...


..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_delete_all.bmp		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_delete_all_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_delete_file.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_delete_file_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_play_setup.bmp		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_play_setup_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_file_info.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_file_info_c.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_unknow_file.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_unknow_file_c.bmp 	>> ..\icon_yeh.h

..\bmp2pat /f4   /p ..\dsc21clut.pal  icon_lock_32.bmp 			>> ..\icon_yeh.h

..\bmp2pat /f4	/p ..\dsc21clut.pal icon_unknow_file80x64.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f8	/p ..\dsc21clut.pal icon_unknow_file_sc.bmp 	>> ..\icon_yeh.h

echo ..\bmp2pat batch ...Finish!
@echo on