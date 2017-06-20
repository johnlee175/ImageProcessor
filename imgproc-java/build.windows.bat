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

set PROJECT="imgproc"
set BUILD_TYPE="%BuildType%"
set APP_VERSION="%BuildVersion%"

cd %SHELL_DIR% ^
&& clean.windows.bat ^
&& gradle assemble ^
&& xcopy /s/e/h/y build/libs/*.jar dist/windows/ ^
&& xcopy /s/e/h/y libs dist/windows/ ^
&& xcopy /s/e/h/y ../%PROJECT%%BUILD_TYPE%-%APP_VERSION%-Build/output/lib dist/windows/jniLibs