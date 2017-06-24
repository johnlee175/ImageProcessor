@echo off

pushd %~dp0
set SHELL_DIR="%CD%"
popd

set BUILD_INFO=%SHELL_DIR%\..\build.info.txt

if exist %BUILD_INFO% (
    for /f "tokens=1,2 eol=# delims==" %%G in (%BUILD_INFO%) do (
        set %%G=%%H
    )
) else (
     echo "%BUILD_INFO% not found."
     exit /b 1
)

REM set CC=/path/to/your/c/compiler
REM set CXX=/path/to/your/c++/compiler
set PROJECT=imgproc
set BUILD_TYPE=%BuildType%
set APP_VERSION=%BuildVersion%

set BUILD_DIR=%SHELL_DIR%\..\%PROJECT%%BUILD_TYPE%-%APP_VERSION%-Build
echo "BUILD_DIR=%BUILD_DIR%"
rmdir /s/q %BUILD_DIR%
mkdir %BUILD_DIR%
cd %BUILD_DIR% && cmake %SHELL_DIR%

if not "%1"=="quiet" pause
