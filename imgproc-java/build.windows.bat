@echo off

set BUILD_INFO=..\build.info.txt

if exist %BUILD_INFO% (
    for /f "tokens=1,2 eol=# delims==" %%G in (%BUILD_INFO%) do (
        set %%G=%%H
    )
) else (
     echo "%BUILD_INFO% not found."
     exit /b 1
)

set PROJECT=imgproc
set BUILD_TYPE=%BuildType%
set APP_VERSION=%BuildVersion%

call clean.windows.bat quiet
call gradle.bat assemble
xcopy /s/e/h/y build\libs\*.jar dist\windows\
mkdir dist\windows\libs
xcopy /s/e/h/y libs dist\windows\libs
mkdir dist\windows\jniLibs
xcopy /s/e/h/y ..\%PROJECT%%BUILD_TYPE%-%APP_VERSION%-Build\output\lib dist\windows\jniLibs

if not "%1"=="quiet" pause
