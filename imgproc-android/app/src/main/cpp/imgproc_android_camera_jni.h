/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache license, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the license for the specific language governing permissions and
 * limitations under the license.
 */
/**
 * jni entry interface
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H
#define IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H

#include <jni.h>

#define JNI_METHOD_NAME(class_name, method_name) Java_com_johnsoft_imgproc_camera_##class_name##_##method_name

#define JNI_METHOD(return_value, class_name, method_name) JNIEXPORT return_value JNICALL \
Java_com_johnsoft_imgproc_camera_##class_name##_##method_name

#define JNI_INSTANCE_PARAM JNIEnv *env, jobject thiz
#define JNI_CLASS_PARAM JNIEnv *env, jclass klass

#define CAMERA_MANAGE_EXCEPTION "Lcom/johnsoft/imgproc/camera/CameraManager$CameraManageException;"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, nativeInitialized)(JNI_INSTANCE_PARAM);
JNI_METHOD(jstring, CameraManager_00024GLClientRenderNativeThread, getName)(JNI_INSTANCE_PARAM);
JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isLoop)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, start)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, quit)(JNI_INSTANCE_PARAM);
JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isPaused)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, setPaused)(JNI_INSTANCE_PARAM, jboolean paused);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onError)(JNI_INSTANCE_PARAM, jthrowable thr);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, sendNotification)(JNI_INSTANCE_PARAM, jstring message);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onFrameAvailable)(JNI_INSTANCE_PARAM, jobject surfaceTexture);

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId);
JNI_METHOD(jobject/* CameraView */, CameraNativeView, setShaderSourceCode)(JNI_INSTANCE_PARAM,
                                                                           jstring vertexSource,
                                                                           jstring fragmentSource);

JNI_METHOD(jobject/* ByteBuffer */, CameraNativeView_00024FrameCallbackThread, mallocDirect)(JNI_CLASS_PARAM, jint capacity);
JNI_METHOD(void, CameraNativeView_00024FrameCallbackThread, freeDirect)(JNI_CLASS_PARAM, jobject byteBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H */
