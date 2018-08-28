@echo off
rem Convert PVR1300 icon
rem Add to icon_yeh.h
rem CharlesH.

echo ..\bmp2pat batch ...

..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_auto_redeye.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_auto_redeye_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_DSC_preview.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_DSC_preview_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_DSC_preview_r.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_flash_froce.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_flash_froce_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_flash_off.bmp		>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_flash_off_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_froce_redeye.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_froce_redeye_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_power_empty.bmp	>> ..\icon_yeh.h
..\bmp2pat /f8  /p ..\dsc21clut.pal     icon_power_empty_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_power_full.bmp		>> ..\icon_yeh.h
..\bmp2pat /f8  /p ..\dsc21clut.pal     icon_power_full_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_power_low.bmp		>> ..\icon_yeh.h
..\bmp2pat /f8  /p ..\dsc21clut.pal     icon_power_low_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f4  /p ..\dsc21clut.pal /t0 icon_PVR_preview.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_PVR_preview_c.bmp	>> ..\icon_yeh.h
..\bmp2pat /f16 /p ..\dsc21clut.pal /t0 icon_PVR_preview_r.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_guilog_play.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_guilog_play_c.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_guilog_rec.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4 	/p ..\dsc21clut.pal /t0 icon_guilog_rec_c.bmp	>> ..\icon_yeh.h

..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_battery_full.bmp 	>  ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_battery_low.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4	/p ..\dsc21clut.pal /t0 icon_battery_empty.bmp 	>> ..\icon_yeh.h

..\bmp2pat /f8	/p ..\dsc21clut.pal /t238 icon_zoom_bar.bmp 	>> ..\icon_yeh.h

..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock1.bmp 	> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock2.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock3.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock4.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock5.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock6.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock7.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock8.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock9.bmp 	>> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock10.bmp >> ..\icon_yeh.h
..\bmp2pat /f4   /p ..\dsc21clut.pal /t235 icon_clock11.bmp >> ..\icon_yeh.h

..\bmp2pat /f4	/t240 /p ..\dsc21clut.pal icon_door_open_c.bmp		>> ..\icon_yeh.h
..\bmp2pat /f4	/t240 /p ..\dsc21clut.pal icon_card_remove_c.bmp	>> ..\icon_yeh.h

..\bmp2pat /f8	/p ..\dsc21clut.pal icon_DSC_preview_g.bmp			>> ..\icon_yeh.h
echo ..\bmp2pat batch ...Finish!
@echo on