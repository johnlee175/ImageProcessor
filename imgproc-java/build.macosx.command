#!/usr/bin/env bash

cd "$(dirname "$0")" \
&& . ./clean.macosx.command \
&& . ./build.linux.sh \
&& cp -r dist/linux/* dist/macosx/ImageProcessor.app/Contents/MacOS/ \
&& mv dist/macosx/ImageProcessor.app/Contents/MacOS/launcher.sh dist/macosx/ImageProcessor.app/Contents/MacOS/ImageProcessor

if [ $? -ne 0 ]; then
    echo "Press enter to continue"; read dummy;
fi
