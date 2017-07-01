#!/usr/bin/env bash

#bash --posix -x clean.linux.sh
#you can set program argument with '--debug'
bash --posix -x build.linux.sh && bash --posix -x dist/linux/launcher.sh $1