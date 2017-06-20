#!/usr/bin/env bash

SHELL_DIR="$(cd $(dirname "$0"); pwd)"
BUILD_INFO="${SHELL_DIR}/../build.info.txt"

if [ -f "${BUILD_INFO}" ]
then
  while IFS='=' read -r key value
  do
    eval "${key}='${value}'"
  done < "${BUILD_INFO}"
else
  echo "${BUILD_INFO} not found."
  exit 1;
fi

export PROJECT=imgproc
export BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< ${BuildType:0:1})${BuildType:1}"
export APP_VERSION=${BuildVersion}

cd ${SHELL_DIR} \
&& bash clean.linux.sh \
&& gradle assemble \
&& cp -r build/libs/*.jar dist/linux/ \
&& cp -r libs dist/linux/ \
&& cp -r ../${PROJECT}${BUILD_TYPE}-${APP_VERSION}-Build/output/lib dist/linux/jniLibs