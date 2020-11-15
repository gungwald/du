@echo off
set BIN_DIR=%USERPROFILE%\bin
if not exist mkdir %BIN_DIR%
copy du.exe %BIN_DIR%
set PATH=%PATH%;%BIN_DIR%
reg query hkcu\Environment /v PATH
if ERRORLEVEL 1 (
    REG ADD hkcu\Environment /v Path /t REG_SZ /d %BIN_DIR%
) else (
