#!/usr/bin/env bash

SHELL_DIR="$(cd $(dirname "$0"); pwd)"

gradle clean
rm -rf ${SHELL_DIR}/dist/linux/*.jar
rm -rf ${SHELL_DIR}/dist/linux/libs
rm -rf ${SHELL_DIR}/dist/linux/jniLibs
