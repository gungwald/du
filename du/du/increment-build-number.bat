@echo off
set buildNumberFile=build-number.h
for /f "tokens=3" %%n in (%buildNumberFile%) do set buildNumber=%%n
set /a nextBuildNumber=%buildNumber% + 1
echo #define BUILD_NUMBER %nextBuildNumber% > %buildNumberFile%
