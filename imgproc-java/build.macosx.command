#!/usr/bin/env bash

cd $(dirname "$0") \
&& bash clean.macosx.command \
&& bash build.linux.sh \
&& cp -r dist/linux/* dist/macosx/imgproc.app/Contents/MacOS/ \
&& mv dist/macosx/imgproc.app/Contents/MacOS/launcher.sh dist/macosx/imgproc.app/Contents/MacOS/imgproc

if [ $? -ne 0 ]; then
    echo Press enter to continue; read dummy;
fi
