cmake_minimum_required(VERSION 3.8.1)


set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)


message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")

set(CMAKE_BINARY_DIR $ENV{BUILD_DIR}/output/)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32 OR MSVC OR MSVC_IDE OR MSVC60 OR MSVC70 OR MSVC71 OR MSVC80 OR CMAKE_COMPILER_2005 OR MSVC90 OR MSVC10
          OR MINGW OR MSYS OR CYGWIN OR BORLAND OR WATCOM)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()


#set(CMAKE_C_COMPILER /path/to/your/c/compiler)
#set(CMAKE_CXX_COMPILER /path/to/your/c++/compiler)

message("CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("CMAKE_C_COMPILER_ID=${CMAKE_C_COMPILER_ID}")
message("CMAKE_C_COMPILER_VERSION=${CMAKE_C_COMPILER_VERSION}")
message("CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_CXX_COMPILER_VERSION=${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_BUILD_TYPE $ENV{BUILD_TYPE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -std=c11 -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wno-unused-parameter -std=c++11 -fvisibility=hidden")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")


find_package(JNI REQUIRED)
include_directories(${JAVA_INCLUDE_PATH})
include_directories(${JAVA_INCLUDE_PATH2})


add_library(
        # Sets the name of the library.
        sourceFiles
        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        # Associated headers in the same location as their source
        # file are automatically included.
        src/main/cpp/main.c
        src/main/cpp/BaseImageProc.c)


message("CMAKE_SYSTEM=${CMAKE_SYSTEM}")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
message("CMAKE_HOST_SYSTEM=${CMAKE_HOST_SYSTEM}")
message("CMAKE_HOST_SYSTEM_NAME=${CMAKE_HOST_SYSTEM_NAME}")

if(APPLE)
    message("APPLE")
    set_target_properties(sourceFiles PROPERTIES
            PREFIX "lib" OUTPUT_NAME "$ENV{PROJECT}-$ENV{APP_VERSION}" SUFFIX ".jnilib")
elseif(UNIX)
    message("UNIX")
    set_target_properties(sourceFiles PROPERTIES
            PREFIX "lib" OUTPUT_NAME "$ENV{PROJECT}-$ENV{APP_VERSION}" SUFFIX ".so")
else()
    message("WINDOWS")
    set_target_properties(sourceFiles PROPERTIES
            PREFIX "" OUTPUT_NAME "$ENV{PROJECT}-$ENV{APP_VERSION}" SUFFIX ".dll")
    set(CMAKE_GNUtoMS ON)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
endif()


target_link_libraries(
        sourceFiles
        # ${dependencies-lib}
)
