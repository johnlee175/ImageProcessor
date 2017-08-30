cmake_minimum_required(VERSION 3.4.2)

#TODO Refactor: extract base header file later
include_directories(../../imgproc-c/src/main/cpp/base)

add_library(imgproc_android_camera SHARED
    src/main/cpp/gl_render_camera_box.c
    src/main/cpp/CCGLRenderCameraBox.cpp
    src/main/cpp/imgproc_android_camera_jni.c
    #src/main/cpp/ImgProcAndroidCameraJni.cpp
)

add_library(simple_camera_view SHARED
    src/main/cpp/simple/SimpleCameraView.cpp
    src/main/cpp/simple/JniMain.cpp
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

target_link_libraries(simple_camera_view
   ${log-lib}
   ${dl-lib}
   ${EGL-lib}
   ${GLESv2-lib}
   ${android-lib}
)
