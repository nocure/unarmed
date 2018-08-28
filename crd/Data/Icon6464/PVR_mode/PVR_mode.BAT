@echo off
rem Convert PVR1300 icon
rem Add to icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_forward.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_loading.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_loading_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_pause.bmp		>> ..\icon_yeh.h

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_playback.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_purgeing.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_purgeing_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_rec_setup.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_rec_setup_c.bmp >> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_reverse.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_stop.bmp		>> ..\icon_yeh.h


..\bmp2pat /f2	/p ..\dsc21clut.pal icon_forward_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f2	/p ..\dsc21clut.pal icon_pause_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f2	/p ..\dsc21clut.pal icon_playback_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f2	/p ..\dsc21clut.pal icon_reverse_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f2	/p ..\dsc21clut.pal icon_stop_c.bmp			>> ..\icon_yeh.h

echo ..\bmp2pat batch ...Finish!
@echo on