@echo off

if not "%1" == "cplib" goto cant_run
if "%2" == "" goto cant_run2
if "%3" == "" goto cant_run3

echo checking directories...
cd ..\..

if not exist "lib" mkdir "lib"
if not exist "lib\include" mkdir "lib\include"
if not exist "lib\%3" mkdir "lib\%3"

echo copying %2 (%3) files ...

xcopy /s /y /c %2\include\*.* lib\include
copy /y %2\%3\*.a lib\%3

goto done

:cant_run
echo can't run directly
goto done

:cant_run2
echo no project
goto done

:cant_run3
echo no configuration
goto done

:done
pause
