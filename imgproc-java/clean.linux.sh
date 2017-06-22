#!/usr/bin/env bash

cd "$(dirname "$0")"
if [ $? -eq 0 ]; then
    gradle clean
    rm -rf dist/linux/*.jar
    rm -rf dist/linux/libs
    rm -rf dist/linux/jniLibs
else
    echo "cd current script directory failed"
fi