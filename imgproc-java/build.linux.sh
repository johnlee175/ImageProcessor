#!/usr/bin/env bash

cd "$(dirname "$0")"
if [ $? -eq 0 ]; then
    BUILD_INFO='../build.info.txt'
    if [ -f "${BUILD_INFO}" ]; then
        while IFS='=' read -r key value
        do
            eval "${key}='${value}'"
        done < "${BUILD_INFO}"
    else
        echo "${BUILD_INFO} not found."
        exit 1;
    fi

    BuildType="${BuildType:-debug}"
    export PROJECT='imgproc'
    export BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< ${BuildType:0:1})${BuildType:1}"
    export APP_VERSION="${BuildVersion:-1.0.0}"

    . ./clean.linux.sh \
    && gradle assemble \
    && cp -r build/libs/*.jar dist/linux/ \
    && cp -r libs dist/linux/ \
    && cp -r ../${PROJECT}${BUILD_TYPE}-${APP_VERSION}-Build/output/lib dist/linux/jniLibs
else
    echo "cd current script directory failed"
    exit 1
fi
