@echo off
rem Convert PVR1300 icon
rem create icon_Audio.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f2 	/p ..\dsc21clut.pal /t0 icon_volcursor_1.bmp	>  ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_volcursor_2.bmp>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_volcursor_2w.bmp>> ..\icon_Audio.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_mute.bmp 		>> ..\icon_Audio.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_mute_c.bmp 	>> ..\icon_Audio.h 
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_speaker.bmp	>> ..\icon_Audio.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_speaker_c.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_speaker_s.bmp	>> ..\icon_Audio.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_speaker_s_c.bmp>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_01.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_02.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_03.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_04.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_05.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_06.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_07.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_08.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_09.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_10.bmp 	>> ..\icon_Audio.h
..\bmp2pat /f2	/p ..\dsc21clut.pal /t0 icon_vol_bar.bmp 	>> ..\icon_Audio.h



echo ..\bmp2pat batch ...Finish!
@echo on