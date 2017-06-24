#!/usr/bin/env bash

#bash --posix -x clean.linux.sh
bash --posix -x build.linux.sh && bash --posix -x dist/linux/launcher.sh
#bash --posix -x build.linux.sh && bash --posix -x dist/linux/launcher.sh --debug