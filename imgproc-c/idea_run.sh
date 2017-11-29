#!/usr/bin/env bash

# make this script as Clion Bash configuration:
# Run->Edit Configurations->Add Bash Target->Set Name and Script->add JAVA_HOME to Environment Variables
# $1 like image_io_test
bash --posix -x build.linux.sh && ../ImageProcessorDebug-1.0.0-Build/output/bin/$1
