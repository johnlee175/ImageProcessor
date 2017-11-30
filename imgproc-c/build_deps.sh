#!/usr/bin/env bash

cd ../c-deps
CURR_DIR=`pwd`
echo "we are in `pwd`"
rm -rf dist && mkdir dist

cd ${CURR_DIR} && rm -rf libpng-1.4.22 && tar zxf libpng-1.4.22.tar.gz \
 && cd libpng-1.4.22 && mkdir cmake-build && cd cmake-build \
 && cmake -DCMAKE_INSTALL_PREFIX="${CURR_DIR}/dist" -G "Unix Makefiles" .. && make -j2 && make install \
 && cd ${CURR_DIR} && rm -rf libpng-1.4.22
if [ $? -ne 0 ]; then
    echo "Build libpng-1.4.22 failed"; exit;
fi

cd ${CURR_DIR} && rm -rf jpeg-9b && tar zxf jpeg-9b.tar.gz \
 && cd jpeg-9b && ./configure --prefix="${CURR_DIR}/dist" && make -j2 && make install \
 && cd ${CURR_DIR} && rm -rf jpeg-9b
if [ $? -ne 0 ]; then
    echo "Build jpeg-9b failed"; exit;
fi

cd ${CURR_DIR} && rm -rf giflib-5.1.4 && tar zxf giflib-5.1.4.tar.gz \
 && cd giflib-5.1.4 && ./configure --prefix="${CURR_DIR}/dist" && make -j2 && make install \
 && cd ${CURR_DIR} && rm -rf giflib-5.1.4
if [ $? -ne 0 ]; then
    echo "Build giflib-5.1.4 failed"; exit;
fi

#TODO we need install it
cd ${CURR_DIR} && rm -rf libwebp-0.6.0 && tar zxf libwebp-0.6.0.tar.gz \
 && cd libwebp-0.6.0 && mkdir cmake-build && cd cmake-build \
 && cmake -DCMAKE_INSTALL_PREFIX="${CURR_DIR}/dist"  -G "Unix Makefiles" .. && make -j2 \
 && cd ${CURR_DIR} && rm -rf libwebp-0.6.0
if [ $? -ne 0 ]; then
    echo "Build libwebp-0.6.0 failed"; exit;
fi

cd ${CURR_DIR} && rm -rf glew-2.1.0 && tar zxf glew-2.1.0.tar.gz \
 && cd glew-2.1.0 && mkdir cmake-build && cd cmake-build \
 && cmake -DCMAKE_INSTALL_PREFIX="${CURR_DIR}/dist"  -G "Unix Makefiles" ../build/cmake/ \
 && make -j2 && make install  \
 && cd ${CURR_DIR} && rm -rf glew-2.1.0
if [ $? -ne 0 ]; then
    echo "Build glew-2.1.0 failed"; exit;
fi

cd ${CURR_DIR} && rm -rf glfw-3.2.1 && tar zxf glfw-3.2.1.tar.gz \
 && cd glfw-3.2.1 && mkdir cmake-build && cd cmake-build \
 && cmake -DCMAKE_INSTALL_PREFIX="${CURR_DIR}/dist" -G "Unix Makefiles" .. && make -j2 && make install \
 && cd ${CURR_DIR} && rm -rf glfw-3.2.1
if [ $? -ne 0 ]; then
    echo "Build glfw-3.2.1 failed"; exit;
fi

echo "All build is successful!"
