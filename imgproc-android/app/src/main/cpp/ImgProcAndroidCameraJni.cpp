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
 * jni entry cc implement
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "base.h"
#include "ImgProcAndroidCameraJni.hpp"
#include "CCGLRenderCameraBox.hpp"
#include <android/native_window_jni.h>

struct JNICCGLRenderCameraBox {
    JavaVM *vm;
    jmethodID method;
    jobject callback;
    jobject bytebuffer;
    jstring jvertexShaderSource;
    jstring jfragmentShaderSource;
    const char *vertexShaderSource;
    const char *fragmentShaderSource;
};

static void callback_func(CCGLRenderCameraBox *glrcbox) {
    void *data = glrcbox->GetUserTag();
    struct JNICCGLRenderCameraBox *jni_glrcbox = __static_cast(struct JNICCGLRenderCameraBox *, data);

    if (jni_glrcbox && jni_glrcbox->vm && jni_glrcbox->method) {
        JNIEnv *env;
        jni_glrcbox->vm->AttachCurrentThread(&env, NULL);
        env->CallVoidMethod(jni_glrcbox->callback, jni_glrcbox->method, jni_glrcbox->bytebuffer);
    }
}

/* =================== CameraManager.GLClientRenderNativeThread =================== */

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, swapBuffers)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_call_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    if (glrcbox->SwapBuffers() < 0) {
        LOGW("call SwapBuffers() failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, createEGL)(JNI_INSTANCE_PARAM,
                                                                               jobject surface) {
    jlong ptr = jnicchelper_call_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    if (glrcbox->SetWindow(ANativeWindow_fromSurface(env, surface)) < 0) {
        LOGW("call SetWindow() failed\n");
        return JNI_FALSE;
    }
    if (glrcbox->CreateEGL() < 0) {
        LOGW("call CreateEGL() failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, destroyEGL)(JNI_INSTANCE_PARAM,
                                                                            jobject surface) {
    jlong ptr = jnicchelper_call_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    if (glrcbox->DestroyEGL() < 0) {
        LOGW("call DestroyEGL() failed\n");
    }
    EGLNativeWindowType window = glrcbox->GetWindow();
    if (window) {
        ANativeWindow_release(window);
    }
    delete glrcbox;
}

/* ============================ CameraNativeView ============================ */

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM) {
    CCGLRenderCameraBox *glrcbox = new CCGLRenderCameraBox;
    if (!glrcbox->IsInitSuccess()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "nativeInitialized native method execute error");
        return;
    } else {
        jnicchelper_set_native_ctx_ptr(env, thiz, __reinterpret_cast(jlong, glrcbox));
        glrcbox->SetFrameDataCallback(callback_func);
        void *data = malloc(sizeof(struct JNICCGLRenderCameraBox));
        if (!data) {
            jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                      "nativeInitialized native method execute error");
            return;
        }
        struct JNICCGLRenderCameraBox *jni_glrcbox = __static_cast(struct JNICCGLRenderCameraBox *,
                                                                   data);
        memset(jni_glrcbox, 0, sizeof(struct JNICCGLRenderCameraBox));
        glrcbox->SetUserTag(jni_glrcbox);
    }
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if (env->GetJavaVM(&vm)) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }

    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    void *data = glrcbox->GetUserTag();
    struct JNICCGLRenderCameraBox *jni_glrcbox = __static_cast(struct JNICCGLRenderCameraBox *, data);
    if (jni_glrcbox) {
        jni_glrcbox->vm = vm;
    }

    jboolean is_front_camera = jnicchelper_call_method("isFrontCamera", "()Z", Boolean);
    glrcbox->SetFrontCamera(__static_cast(GLboolean, is_front_camera));

    jint frame_width = jnicchelper_call_method("getFrameWidth", "()I", Int);
    jint frame_height = jnicchelper_call_method("getFrameHeight", "()I", Int);
    glrcbox->SetFrameSize(__static_cast(GLuint, frame_width), __static_cast(GLuint, frame_height));

    GLubyte *pixels;
    size_t pixels_size;
    glrcbox->GetPixels(&pixels, &pixels_size);

    jobject callback = jnicchelper_get_field("mFrameRgbaDataCallback", "Lcom/johnsoft/imgproc/camera/"
            "CameraView$OnFrameRgbaDataCallback;", Object);
    if (callback && jni_glrcbox) {
        jobject byte_buffer = env->NewDirectByteBuffer(pixels, __static_cast(jlong, pixels_size));
        jni_glrcbox->method = jnicchelper_obj_method_id(callback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
        jni_glrcbox->callback = env->NewGlobalRef(callback);
        jni_glrcbox->bytebuffer = env->NewGlobalRef(byte_buffer);
    }

    if (glrcbox->CreateShader() < 0) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "createShaderAndBuffer native method execute error");
        return;
    }
}

JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    void *data = glrcbox->GetUserTag();
    struct JNICCGLRenderCameraBox *jni_glrcbox = __static_cast(struct JNICCGLRenderCameraBox *, data);

    if (jni_glrcbox) {
        if (jni_glrcbox->callback) {
            env->DeleteGlobalRef(jni_glrcbox->callback);
        }
        if (jni_glrcbox->bytebuffer) {
            env->DeleteGlobalRef(jni_glrcbox->bytebuffer);
        }
        if (jni_glrcbox->jvertexShaderSource && jni_glrcbox->vertexShaderSource) {
            env->ReleaseStringUTFChars(jni_glrcbox->jvertexShaderSource,
                                       jni_glrcbox->vertexShaderSource);
            env->DeleteGlobalRef(jni_glrcbox->jvertexShaderSource);
        }
        if (jni_glrcbox->jfragmentShaderSource && jni_glrcbox->fragmentShaderSource) {
            env->ReleaseStringUTFChars(jni_glrcbox->jfragmentShaderSource,
                                       jni_glrcbox->fragmentShaderSource);
            env->DeleteGlobalRef(jni_glrcbox->jfragmentShaderSource);
        }

        free(jni_glrcbox);
    }
    if (glrcbox->DestroyShader() < 0) {
        LOGW("call DestroyShader() failed\n");
    }
}

JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    if (glrcbox->DrawFrame(__static_cast(GLuint, textureId)) < 0) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "drawFrame native method execute error");
        return;
    }
}

JNI_METHOD(jobject/* CameraView */, CameraNativeView, setShaderSourceCode)(JNI_INSTANCE_PARAM,
                                                                           jstring vertexSource,
                                                                           jstring fragmentSource) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    CCGLRenderCameraBox *glrcbox = __reinterpret_cast(CCGLRenderCameraBox *, ptr);

    void *data = glrcbox->GetUserTag();
    struct JNICCGLRenderCameraBox *jni_glrcbox = __static_cast(struct JNICCGLRenderCameraBox *, data);

    if (jni_glrcbox) {
        if (vertexSource) {
            jni_glrcbox->jvertexShaderSource = __static_cast(jstring, env->NewGlobalRef(vertexSource));
            jni_glrcbox->vertexShaderSource = jnicchelper_from_utf_string(env, vertexSource);
        }
        if (fragmentSource) {
            jni_glrcbox->jfragmentShaderSource = __static_cast(jstring, env->NewGlobalRef(fragmentSource));
            jni_glrcbox->fragmentShaderSource = jnicchelper_from_utf_string(env, fragmentSource);
        }
        if (glrcbox->SetShaderSource(jni_glrcbox->vertexShaderSource,
                                     jni_glrcbox->fragmentShaderSource) == -1) {
            jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                        "setShaderSourceCode native method execute error");
            return NULL;
        }
    }
    return thiz;
}

/* ====================== DirectByteBuffers.NativeDirectMemory ====================== */

JNI_METHOD(jobject/* ByteBuffer */, DirectByteBuffers_00024NativeDirectMemory, mallocDirect)(JNI_INSTANCE_PARAM,
                                                                                             jint capacity) {
    void *buffer = malloc(__static_cast(size_t, capacity));
    if (!buffer) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "mallocDirect native method execute error");
        return NULL;
    }
    memset(buffer, 0, __static_cast(size_t, capacity));
    return env->NewDirectByteBuffer(buffer, capacity);
}

JNI_METHOD(void, DirectByteBuffers_00024NativeDirectMemory, freeDirect)(JNI_INSTANCE_PARAM,
                                                                        jobject byteBuffer) {
    void *buffer = env->GetDirectBufferAddress(byteBuffer);
    if (buffer) {
        free(buffer);
    }
}
