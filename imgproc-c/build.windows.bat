@echo off

pushd %~dp0
set SHELL_DIR="%CD%"
popd

set BUILD_INFO="%SHELL_DIR%/../build.info.txt"

IF EXIST "%BUILD_INFO%" (
    FOR /F "tokens=1,2 eol=# delims==" %%G IN ("%BUILD_INFO%") DO (
        set %%G="%%H"
    )
) ELSE (
     echo "%BUILD_INFO% not found."
     exit /b 1
)

REM set CC=/path/to/your/c/compiler
REM set CXX=/path/to/your/c++/compiler
set PROJECT="imgproc"
set BUILD_TYPE="%BuildType%"
set APP_VERSION="%BuildVersion%"

set BUILD_DIR="%SHELL_DIR%/../%PROJECT%%BUILD_TYPE%-%APP_VERSION%-Build"
echo "BUILD_DIR=%BUILD_DIR%"
rmdir /s/q "%BUILD_DIR%" && mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%" && cmake "%SHELL_DIR%"
