#!/usr/bin/env bash

cd "$(dirname "$0")"

# you can java-debug with 'jdb -attach 127.0.0.1:51230'
# you can java-debug with Intellij IDEA Remote Debug Configurations, Socket, Attach, 51230 port
if [ "$1" == "debug" -o "$1" == "-debug" -o "$1" == "--debug" ]; then
    DEBUG_PARAM='-agentlib:jdwp=transport=dt_socket,address=127.0.0.1:51230,suspend=y,server=y'
else
    DEBUG_PARAM=''
fi

java \
${DEBUG_PARAM} \
-Xmx2048M -Xms1024M \
-Dfile.encoding=UTF-8 \
-Djava.library.path=jniLibs \
-classpath imgproc-1.0.0.jar:libs/* \
-Dsun.java2d.opengl=true \
-splash:splash.png \
com.johnsoft.MainLauncher


# you can jni-debug with follow steps:
# 1. break point on java code call (java-debug) before jni call;
# 2. execute command ps or jps to location the current process pid;
# 3. 'gdb/lldb -p pid' or Attach to local process with Clion;
# 4. break point on jni code call;


# jdb command usages: [you can type help]
# use <path-to-src/main/java>
# stop at <class-full-name:line>
# run/cont
# next/step
# list/threads/locals/dump <object name>/print <expr>


# lldb (on macosx) command usages: [you can type help]
# settings set target.source-map /build_src /source
# breakpoint set --file main.c --line 10
# run/continue
# next/step
# list/thread list/bt/frame variable/p <expr>


# gdb (on linux/unix) command usages: [you can type help]
# dir <path-to-src>
# break main.c:10
# run/continue
# next/step
# list/info threads/bt/info locals/p <expr>

