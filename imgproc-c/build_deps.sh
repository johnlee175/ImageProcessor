#!/usr/bin/env bash

cd ../c-deps
CURR_DIR=`pwd`
echo "we are in `pwd`"
INSTALL_DIR="${CURR_DIR}/dist"
rm -rf dist && mkdir dist

function build_with_cmake
{
local _lib_name="$1"
local _cmake_other_opt="$2"
local _cmake_rela_addr=".."
if [ ! -z "$3" ]; then
    _cmake_rela_addr="$3"
fi
local _make_thread="2"
if [ ! -z "$4" ]; then
    _make_thread="$4"
fi
local _make_install="make install"
if [ ! -z "$5" ]; then
    _make_install="$5"
fi

cd ${CURR_DIR} && rm -rf ${_lib_name} && tar zxf ${_lib_name}.tar.gz \
 && cd ${_lib_name} && mkdir cmake-build && cd cmake-build
if [ $? -ne 0 ]; then
    echo "Build ${_lib_name} failed"; exit;
fi

cmake -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF -G "Unix Makefiles" \
 ${_cmake_other_opt} ${_cmake_rela_addr} && make -j${_make_thread} && ${_make_install}
if [ $? -ne 0 ]; then
    echo "Build ${_lib_name} failed"; exit;
fi

cmake -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=ON -G "Unix Makefiles" \
 ${_cmake_other_opt} ${_cmake_rela_addr} && make -j${_make_thread} && ${_make_install}
if [ $? -ne 0 ]; then
    echo "Build ${_lib_name} failed"; exit;
fi

cd ${CURR_DIR} && rm -rf ${_lib_name}
if [ $? -ne 0 ]; then
    echo "Build ${_lib_name} failed"; exit;
fi
}

function build_with_autotools
{
local _lib_name="$1"
local _config_other_opt="$2"
local _make_thread="2"
if [ ! -z "$3" ]; then
    _make_thread="$3"
fi
local _make_install="make install"
if [ ! -z "$4" ]; then
    _make_install="$4"
fi

cd ${CURR_DIR} && rm -rf ${_lib_name} && tar zxf ${_lib_name}.tar.gz \
 && cd ${_lib_name} && ./configure --prefix="${INSTALL_DIR}" ${_config_other_opt} \
 && make -j${_make_thread} && ${_make_install} \
 && cd ${CURR_DIR} && rm -rf ${_lib_name}

if [ $? -ne 0 ]; then
    echo "Build ${_lib_name} failed"; exit;
fi
}

build_with_cmake "libpng-1.4.22"

build_with_autotools "jpeg-9b"

build_with_autotools "giflib-5.1.4"

WEBP_OPTS="--enable-libwebpmux --enable-libwebpdemux --enable-libwebpdecoder --enable-libwebpextras --disable-gl"
build_with_autotools "libwebp-0.6.0" "${WEBP_OPTS}"

build_with_cmake "glew-2.1.0" "" "../build/cmake/"

build_with_cmake "glfw-3.2.1"

echo "All build is successful!"
