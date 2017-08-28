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
 * jni entry c interface
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H
#define IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H

#ifndef __cplusplus
#include "jni_simple_helper.h"
#endif

#define JNI_METHOD_NAME(class_name, method_name) Java_com_johnsoft_imgproc_camera_##class_name##_##method_name

#define JNI_METHOD(return_value, class_name, method_name) JNIEXPORT return_value JNICALL \
Java_com_johnsoft_imgproc_camera_##class_name##_##method_name

#define CAMERA_MANAGE_EXCEPTION "com/johnsoft/imgproc/camera/CameraManager$CameraManageException"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, swapBuffers)(JNI_INSTANCE_PARAM);
JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, createEGL)(JNI_INSTANCE_PARAM,
                                                                               jobject surface);
JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, destroyEGL)(JNI_INSTANCE_PARAM,
                                                                            jobject surface);

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId);
JNI_METHOD(jobject/* CameraView */, CameraNativeView, setShaderSourceCode)(JNI_INSTANCE_PARAM,
                                                                           jstring vertexSource,
                                                                           jstring fragmentSource);

JNI_METHOD(jobject/* ByteBuffer */, DirectByteBuffers_00024NativeDirectMemory, mallocDirect)(JNI_INSTANCE_PARAM,
                                                                                             jint capacity);
JNI_METHOD(void, DirectByteBuffers_00024NativeDirectMemory, freeDirect)(JNI_INSTANCE_PARAM, jobject byteBuffer);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IMGPROC_ANDROID_IMGPROC_ANDROID_CAMERA_JNI_H */
