@echo off

REM cd %cd%
REM cd %~dp0

java -version > NUL 2>&1
REM download and copy jre to this directory
if not "%errorlevel%" == "0" (
	pushd %~dp0
	set SHELL_DIR="%CD%"
	popd

    set JAVA_BIN=%SHELL_DIR%\jre\bin\java.exe
    echo "java not found, using built-in jre in %JAVA_BIN%"
    doskey java=%JAVA_BIN% $*
)

set DEBUG_PARAM=
set __DEBUG_PARAM__=-agentlib:jdwp=transport=dt_socket,address=127.0.0.1:51230,suspend=y,server=y

if "%1"=="debug" set DEBUG_PARAM=%__DEBUG_PARAM__%
if "%1"=="-debug" set DEBUG_PARAM=%__DEBUG_PARAM__%
if "%1"=="--debug" set DEBUG_PARAM=%__DEBUG_PARAM__%
if "%1"=="/debug" set DEBUG_PARAM=%__DEBUG_PARAM__%
if "%1"=="/D" set DEBUG_PARAM=%__DEBUG_PARAM__%

java ^
%DEBUG_PARAM% ^
-Xmx2048M -Xms1024M ^
-Dfile.encoding="UTF-8" ^
-Djava.library.path="jniLibs" ^
-classpath "imgproc-1.0.0.jar;libs/*" ^
-Dsun.java2d.opengl=true ^
-splash:"splash.png" ^
com.johnsoft.MainLauncher

if not "%1"=="quiet" pause
