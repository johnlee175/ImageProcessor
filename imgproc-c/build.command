#!/usr/bin/env bash

cd $(dirname "$0") && bash build.sh -G "Xcode" -C "echo everything is ok"

if [ $? -ne 0 ]; then
    echo Press enter to continue; read dummy;
fi