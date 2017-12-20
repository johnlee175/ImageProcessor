#!/usr/bin/env bash

cd "$(dirname "$0")"
if [ $? -eq 0 ]; then
    . ./clean.linux.sh
    RESULT_A=$?
    rm -rf dist/macosx/ImageProcessor.app/Contents/MacOS/*
    RESULT_B=$?

    if [ ${RESULT_A} -ne 0 -o ${RESULT_B} -ne 0 ]; then
        echo "Press enter to continue"; read dummy;
    fi
else
    echo "cd current script directory failed"
    echo "Press enter to continue"; read dummy;
fi