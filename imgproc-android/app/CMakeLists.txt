cmake_minimum_required(VERSION 3.4.2)

#TODO Refactor: extract base header file later
include_directories(../../imgproc-c/src/main/cpp/base)

add_library(imgproc_android_camera SHARED
    src/main/cpp/glclient_render_thread.c
    src/main/cpp/CCGlClientRenderThread.cpp
    src/main/cpp/camera_native_view.c
    src/main/cpp/CCCameraNativeView.cpp
    src/main/cpp/imgproc_android_camera_jni.c
    #src/main/cpp/ImgProcAndroidCameraJni.cpp
)

find_library(log-lib log)
find_library(dl-lib dl)
find_library(EGL-lib EGL)
find_library(GLESv2-lib GLESv2)
find_library(android-lib android)

target_link_libraries(imgproc_android_camera
   ${log-lib}
   ${dl-lib}
   ${EGL-lib}
   ${GLESv2-lib}
   ${android-lib}
)
