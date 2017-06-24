@echo off

echo "gen bat to 32 exe ..."
tools\bat2exe_32.exe -bat dist\windows\launcher.bat ^
-save dist\windows\launcher32.exe
echo "gen bat to 64 exe ..."
tools\bat2exe_64.exe -bat dist\windows\launcher.bat ^
-save dist\windows\launcher64.exe

if not "%1"=="quiet" pause
