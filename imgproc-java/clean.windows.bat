@echo off

call gradle.bat clean
del dist\windows\*.jar
rmdir /s/q dist\windows\libs
rmdir /s/q dist\windows\jniLibs

if not "%1"=="quiet" pause
