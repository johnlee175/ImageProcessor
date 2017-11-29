cmake_minimum_required(VERSION 3.8.1)

#begin global settings
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

#set(CMAKE_C_COMPILER /path/to/your/c/compiler)
#set(CMAKE_CXX_COMPILER /path/to/your/c++/compiler)

message("CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("CMAKE_C_COMPILER_ID=${CMAKE_C_COMPILER_ID}")
message("CMAKE_C_COMPILER_VERSION=${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}")
message("")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -std=c99 -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -std=c++99 -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

if (DEFINED ENV{BUILD_DIR})
    message("FOUND BUILD_DIR IN ENV VAR")
    set(CMAKE_BINARY_DIR $ENV{BUILD_DIR}/output)
elseif(CMAKE_BUILD_DIR)
    message("FOUND CMAKE_BUILD_DIR IN CMAKE OPTS")
    set(CMAKE_BINARY_DIR ${CMAKE_BUILD_DIR}/output)
else()
    message("USING DEFAULT CMAKE_BINARY_DIR")
    set(CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR}/output)
endif ()
message("CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32 OR MSVC OR MSVC_IDE OR MSVC60 OR MSVC70 OR MSVC71 OR MSVC80 OR CMAKE_COMPILER_2005 OR MSVC90 OR MSVC10
        OR MINGW OR MSYS OR CYGWIN OR BORLAND OR WATCOM)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

message("CMAKE_SYSTEM=${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
message("CMAKE_HOST_SYSTEM=${CMAKE_HOST_SYSTEM}")
message("CMAKE_HOST_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME}")
message("")

if (DEFINED ENV{APP_VERSION})
    set(GLOBAL_APP_VERSION $ENV{APP_VERSION})
    message("FOUND APP_VERSION IN ENV VAR")
elseif(CMAKE_APP_VERSION)
    set(GLOBAL_APP_VERSION ${CMAKE_APP_VERSION})
    message("FOUND APP_VERSION IN CMAKE OPTS")
else()
    set(GLOBAL_APP_VERSION "1.0.0")
    message("USING DEFAULT APP_VERSION")
endif ()
message("GLOBAL_APP_VERSION=${GLOBAL_APP_VERSION}")
message("")

include_directories(src/main/cpp)
if (UNIX)
    include_directories(/usr/include /usr/local/include)
    link_directories(/usr/lib /usr/local/lib)
endif ()
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)

# Make sure exists JAVA_HOME global environment variable, locate at valid JDK directory, and $JAVA_HOME/bin in $PATH
# If you use Clion, maybe need add JAVA_HOME by(linux): File->Settings->Build,Execution,Deployment->CMake->Environment
find_package(JNI REQUIRED)
if (NOT ${JNI_FOUND})
    message(FATAL_ERROR "JNI NOT FOUND")
endif ()
message("JNI_FOUND=${JNI_FOUND}")
message("JAVA_INCLUDE_PATH=${JAVA_INCLUDE_PATH}")
message("JAVA_INCLUDE_PATH2=${JAVA_INCLUDE_PATH2}")
message("")

find_package(OpenGL REQUIRED)
if (NOT ${OPENGL_FOUND})
    message(FATAL_ERROR "OpenGL NOT FOUND")
endif ()
message("OPENGL_FOUND=${OPENGL_FOUND}")
message("OPENGL_GLU_FOUND=${OPENGL_GLU_FOUND}")
message("OPENGL_INCLUDE_DIR=${OPENGL_INCLUDE_DIR}")
message("OPENGL_LIBRARIES=${OPENGL_LIBRARIES}")
message("OPENGL_gl_LIBRARY=${OPENGL_gl_LIBRARY}")
message("OPENGL_glu_LIBRARY=${OPENGL_glu_LIBRARY}")
message("")

find_package(PkgConfig REQUIRED)
if (NOT ${PkgConfig_FOUND})
    message(FATAL_ERROR "PkgConfig NOT FOUND")
endif ()
message("PkgConfig_FOUND=${PkgConfig_FOUND}\n")

# 1. "cmake --help-module-list` to see the find_package work target;
# 2. "pkg-config --list-all` to see all config;
# 3. add /*/glfw/<version>/lib/pkgconfig/ to $PKG_CONFIG_PATH,
#    or make sure the *.pc file in /usr/lib/pkgconfig/ or /usr/local/lib/pkgconfig/;
pkg_search_module(GLEW REQUIRED glew glew2 libglew)
message("GLEW: FROM PKG_CONFIG_FOUND=${PKG_CONFIG_FOUND}")
pkg_search_module(GLFW REQUIRED glfw glfw3 libglfw)
message("GLFW: FROM PKG_CONFIG_FOUND=${PKG_CONFIG_FOUND}")
pkg_search_module(FREEGLUT REQUIRED freeglut freeglut3 libfreeglut)
if (${GLEW_FOUND} AND ${GLFW_FOUND} AND ${FREEGLUT_FOUND})
    link_directories(${GLEW_LIBDIR} ${GLFW_LIBDIR} ${FREEGLUT_LIBDIR})
endif ()
message("FREEGLUT: FROM PKG_CONFIG_FOUND=${PKG_CONFIG_FOUND}")
message("GLEW_INCLUDEDIR=${GLEW_INCLUDEDIR}")
message("GLFW_INCLUDEDIR=${GLFW_INCLUDEDIR}")
message("FREEGLUT_INCLUDEDIR=${FREEGLUT_INCLUDEDIR}")
message("GLEW_LIBDIR=${GLEW_LIBDIR}")
message("GLFW_LIBDIR=${GLFW_LIBDIR}")
message("FREEGLUT_LIBDIR=${FREEGLUT_LIBDIR}")
message("")
#end global settings

#begin imgproc
add_library(imgproc SHARED
        src/main/cpp/BaseImageProc_jni.c
        src/main/cpp/BaseImageProc.c)
target_include_directories(imgproc PRIVATE ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
target_link_libraries(imgproc 
        # ${dependencies-lib} 
)

list(APPEND lib_targets imgproc)
message("NOTICE: module imgproc will be build")
#end imgproc

#begin glbox
if (${GLEW_FOUND} AND ${GLFW_FOUND} AND ${FREEGLUT_FOUND})
    if(APPLE)
        set(PLAT_SUBDIR macosx)
    elseif(UNIX)
        set(PLAT_SUBDIR linux)
    else()
        set(PLAT_SUBDIR windows)
    endif ()
    # ANDROID and IOS see another project

    file(GLOB_RECURSE GLBOX_SOURCES
            src/main/cpp/glbox/${PLAT_SUBDIR}/*.c
            src/main/cpp/glbox/${PLAT_SUBDIR}/*.h)

    # see imgproc-android/app/CMakeLists.txt
    add_library(glbox SHARED ${GLBOX_SOURCES}
            src/main/cpp/glbox/gpu_image.c
            src/main/cpp/glbox/SimpleGpuProc_jni.c)

    target_include_directories(glbox PRIVATE
            ${GLEW_INCLUDEDIR} ${GLFW_INCLUDEDIR} ${FREEGLUT_INCLUDEDIR}
            ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
    set(GLLIB_DEPENDENCIES ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} ${FREEGLUT_LIBRARIES})
    target_link_libraries(glbox ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLLIB_DEPENDENCIES})

    list(APPEND lib_targets glbox)
    message("NOTICE: module glbox will also be build")
else()
    message(WARNING "module glbox not build because of dependencies")
endif ()
#end glbox

#begin image-io
message("")
check_include_file(png.h HAVE_PNG_H)
if (NOT HAVE_PNG_H)
    find_file(_HAVE_PNG_H_ png.h)
    set(HAVE_PNG_H ${_HAVE_PNG_H_})
endif ()
message("HAVE_PNG_H=${HAVE_PNG_H}")

check_include_file(jpeglib.h HAVE_JPEG_H)
if (NOT HAVE_JPEG_H)
    find_file(_HAVE_JPEG_H_ jpeglib.h)
    set(HAVE_JPEG_H ${_HAVE_JPEG_H_})
endif ()
message("HAVE_JPEG_H=${HAVE_JPEG_H}")

check_include_file(gif_lib.h HAVE_GIF_H)
if (NOT HAVE_GIF_H)
    find_file(_HAVE_GIF_H_ gif_lib.h)
    set(HAVE_GIF_H ${_HAVE_GIF_H_})
endif ()
message("HAVE_GIF_H=${HAVE_GIF_H}")

check_include_files("webp/encode.h;webp/mux.h;webp/decode.h;webp/demux.h" HAVE_WEBP_H)
if (NOT HAVE_WEBP_H)
    find_file(_HAVE_WEBP_H_ webp)
    set(HAVE_WEBP_H ${_HAVE_WEBP_H_})
endif ()
message("HAVE_WEBP_H=${HAVE_WEBP_H}")

# png16
check_library_exists(png "png_image_begin_read_from_file;png_image_write_to_file" "" HAVE_PNG_LIB)
message("HAVE_PNG_LIB=${HAVE_PNG_LIB}")
# jpeg9
check_library_exists(jpeg "jpeg_start_compress;jpeg_start_decompress" "" HAVE_JPEG_LIB)
message("HAVE_JPEG_LIB=${HAVE_JPEG_LIB}")
# gif7
check_library_exists(gif "EGifOpenFileName;DGifOpenFileName" "" HAVE_GIF_LIB)
message("HAVE_GIF_LIB=${HAVE_GIF_LIB}")
# webp040
check_library_exists(webp "WebPEncode" "" HAVE_WEBP_LIB)
message("HAVE_WEBP_LIB=${HAVE_WEBP_LIB}")

if (HAVE_PNG_H AND HAVE_JPEG_H AND HAVE_GIF_H AND HAVE_WEBP_H
        AND HAVE_PNG_LIB AND HAVE_JPEG_LIB AND HAVE_GIF_LIB AND HAVE_WEBP_LIB)
    set(SOURCE_FILES_IMGIO
            src/main/cpp/image_io/octree_palette.c
            src/main/cpp/image_io/bmp.c
            src/main/cpp/image_io/tga.c
            src/main/cpp/image_io/image_io.c
            src/main/cpp/image_io/image_test.c)
    add_executable(image_io_test ${SOURCE_FILES_IMGIO})
    target_link_libraries(image_io_test png jpeg gif webp)
    file(COPY src/main/resources DESTINATION ./output/)
    message("NOTICE: module image-io will also be build")
else()
    message(WARNING "module image-io not build because of dependencies")
endif ()
unset(HAVE_PNG_H)
unset(HAVE_JPEG_H)
unset(HAVE_GIF_H)
unset(HAVE_WEBP_H)
unset(HAVE_PNG_LIB)
unset(HAVE_JPEG_LIB)
unset(HAVE_GIF_LIB)
unset(HAVE_WEBP_LIB)

#end image-io

#begin each item settings
message("\nlib_targets=[${lib_targets}]\n")
foreach(current_target ${lib_targets})
    if (DEFINED ENV{${current_target}_APP_NAME})
        set(${current_target}_APP_NAME $ENV{${current_target}_APP_NAME})
        message("FOUND ${current_target}_APP_NAME IN ENV VAR")
    elseif(CMAKE_${current_target}_APP_NAME)
        set(${current_target}_APP_NAME CMAKE_${current_target}_APP_NAME)
        message("FOUND ${current_target}_APP_NAME IN CMAKE OPTS")
    else()
        set(${current_target}_APP_NAME ${current_target})
        message("USING DEFAULT ${current_target}_APP_NAME")
    endif ()
    if (DEFINED ENV{${current_target}_APP_VERSION})
        set(${current_target}_APP_VERSION $ENV{${current_target}_APP_VERSION})
        message("FOUND ${current_target}_APP_VERSION IN ENV VAR")
    elseif(CMAKE_${current_target}_APP_VERSION)
        set(${current_target}_APP_VERSION CMAKE_${current_target}_APP_VERSION)
        message("FOUND ${current_target}_APP_VERSION IN CMAKE OPTS")
    else()
        set(${current_target}_APP_VERSION ${GLOBAL_APP_VERSION})
        message("USING DEFAULT ${current_target}_APP_VERSION")
    endif ()
    message("settings ${current_target}:
    ${current_target}_APP_NAME=${${current_target}_APP_NAME}
    ${current_target}_APP_VERSION=${${current_target}_APP_VERSION}")
    if(APPLE)
        message("set_target_properties with APPLE style")
        set_target_properties(${current_target} PROPERTIES
                PREFIX "lib" 
                OUTPUT_NAME "${${current_target}_APP_NAME}-${${current_target}_APP_VERSION}" 
                SUFFIX ".jnilib")
    elseif(UNIX)
        message("set_target_properties with UNIX style")
        set_target_properties(${current_target} PROPERTIES
                PREFIX "lib" 
                OUTPUT_NAME "${${current_target}_APP_NAME}-${${current_target}_APP_VERSION}"
                SUFFIX ".so")
    else()
        message("set_target_properties with WINDOWS style")
        set_target_properties(${current_target} PROPERTIES
                PREFIX "" 
                OUTPUT_NAME "${${current_target}_APP_NAME}-${${current_target}_APP_VERSION}"
                SUFFIX ".dll")
        set(CMAKE_GNUtoMS ON)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    endif()
    message("")
endforeach()
#end each item settings
