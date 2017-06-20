@echo off

pushd %~dp0
set SHELL_DIR="%CD%"
popd

gradle clean
del "${SHELL_DIR}/dist/windows/*.jar"
rmdir /s/q "${SHELL_DIR}/dist/windows/libs"
rmdir /s/q "${SHELL_DIR}/dist/windows/jniLibs"
