#!/usr/bin/env bash

cd $(dirname "$0") \
&& bash clean.linux.sh \
&& rm -rf dist/macosx/imgproc.app/Contents/MacOS/*

if [ $? -ne 0 ]; then
    echo Press enter to continue; read dummy;
fi
