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

class MyCallback: public camerabox::CCFrameDataCallback {
public:
    MyCallback(JavaVM *jvm):vm(jvm), method(NULL), callback(NULL), buffer(NULL),
                 jVertexShaderSource(NULL), jFragmentShaderSource(NULL),
                 vertexShaderSource(NULL), fragmentShaderSource(NULL) { }

    virtual ~MyCallback() {
        if (vm) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, NULL);
            if (callback) {
                env->DeleteGlobalRef(callback);
            }
            if (buffer) {
                env->DeleteGlobalRef(buffer);
            }
            if (jVertexShaderSource && vertexShaderSource) {
                env->ReleaseStringUTFChars(jVertexShaderSource, vertexShaderSource);
                env->DeleteGlobalRef(jVertexShaderSource);
            }
            if (jFragmentShaderSource && fragmentShaderSource) {
                env->ReleaseStringUTFChars(jFragmentShaderSource, fragmentShaderSource);
                env->DeleteGlobalRef(jFragmentShaderSource);
            }
        }
    }

    virtual void OnDataCallback(camerabox::CCGLRenderCameraBox *glrcbox, GLboolean normal) {
        if (vm && method) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, NULL);
            env->CallVoidMethod(callback, method, buffer, normal ? JNI_TRUE : JNI_FALSE);
        }
    }

    void SetCallbackParameters(JNIEnv *env, jmethodID method, jobject callback, jobject buffer) {
        this->method = method;
        this->callback = env->NewGlobalRef(callback);
        this->buffer = env->NewGlobalRef(buffer);
    }

    void SetVertexShaderSource(jstring jVertexShaderSource, const char *vertexShaderSource) {
        this->jVertexShaderSource = jVertexShaderSource;
        this->vertexShaderSource = vertexShaderSource;
    }

    void SetFragmentShaderSource(jstring jFragmentShaderSource, const char *fragmentShaderSource) {
        this->jFragmentShaderSource = jFragmentShaderSource;
        this->fragmentShaderSource = fragmentShaderSource;
    }

private:
    JavaVM *vm;
    jmethodID method;
    jobject callback;
    jobject buffer;
    jstring jVertexShaderSource;
    jstring jFragmentShaderSource;
    const char *vertexShaderSource;
    const char *fragmentShaderSource;
};

/* =================== CameraManager.GLClientRenderNativeThread =================== */

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, swapBuffers)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_call_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

    if (glrcbox->SwapBuffers() < 0) {
        LOGW("call SwapBuffers() failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, createEGL)(JNI_INSTANCE_PARAM,
                                                                               jobject surface) {
    jlong ptr = jnicchelper_call_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

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
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

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
    camerabox::CCGLRenderCameraBox *glrcbox = new camerabox::CCGLRenderCameraBox;
    if (!glrcbox->IsInitSuccess()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "nativeInitialized native method execute error");
        return;
    } else {
        jnicchelper_set_native_ctx_ptr(env, thiz, __reinterpret_cast(jlong, glrcbox));
        JavaVM *vm;
        if (env->GetJavaVM(&vm)) {
            jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
            return;
        }
        MyCallback *myCallback = new MyCallback(vm);
        glrcbox->SetFrameDataCallback(myCallback);
    }
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

    MyCallback *myCallback = __static_cast(MyCallback *, glrcbox->GetFrameDataCallback());
    if (!myCallback) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "myCallback is NULL which getting from GetFrameDataCallback");
        return;
    }

    jint fragment_shader_type = jnicchelper_call_method("getFragmentShaderType", "()I", Int);
    switch(fragment_shader_type) {
        case -1:
            glrcbox->SetFragmentShaderType(REVERSE_X);
            break;
        case 1:
            glrcbox->SetFragmentShaderType(REVERSE_Y);
            break;
        default:
            glrcbox->SetFragmentShaderType(NORMAL);
            break;
    }

    jint frame_width = jnicchelper_call_method("getFrameWidth", "()I", Int);
    jint frame_height = jnicchelper_call_method("getFrameHeight", "()I", Int);
    glrcbox->SetFrameSize(__static_cast(GLuint, frame_width), __static_cast(GLuint, frame_height));

    GLubyte *pixels;
    size_t pixels_size;
    glrcbox->GetPixels(&pixels, &pixels_size);

    jobject vertexSource = jnicchelper_call_method("getVertexShaderSourceCode",
                                                   "()Ljava/lang/String;", Object);
    jobject fragmentSource = jnicchelper_call_method("getFragmentShaderSourceCode",
                                                     "()Ljava/lang/String;", Object);
    const char *vertexShaderSource = NULL;
    const char *fragmentShaderSource = NULL;
    if (vertexSource) {
        jstring jVertexShaderSource = __static_cast(jstring, env->NewGlobalRef(vertexSource));
        vertexShaderSource = jnicchelper_from_utf_string(env, jVertexShaderSource);
        myCallback->SetVertexShaderSource(jVertexShaderSource, vertexShaderSource);
    }
    if (fragmentSource) {
        jstring jFragmentShaderSource = __static_cast(jstring, env->NewGlobalRef(fragmentSource));
        fragmentShaderSource = jnicchelper_from_utf_string(env, jFragmentShaderSource);
        myCallback->SetFragmentShaderSource(jFragmentShaderSource, fragmentShaderSource);
    }
    if (glrcbox->SetShaderSource(vertexShaderSource, fragmentShaderSource) == -1) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "setShaderSourceCode native method execute error");
        return;
    }

    jobject callback = jnicchelper_call_method("getOnFrameRgbaDataCallback", "()Lcom/johnsoft/imgproc/camera/"
            "CameraView$OnFrameRgbaDataCallback;", Object);
    if (callback) {
        jobject byte_buffer = env->NewDirectByteBuffer(pixels, __static_cast(jlong, pixels_size));
        jmethodID mid = jnicchelper_obj_method_id(callback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;Z)V");
        myCallback->SetCallbackParameters(env, mid, callback, byte_buffer);
    }

    if (glrcbox->CreateShader() < 0) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "createShaderAndBuffer native method execute error");
        return;
    }
}

JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

    MyCallback *myCallback = __static_cast(MyCallback *, glrcbox->GetFrameDataCallback());
    if (myCallback) {
        delete myCallback;
    }

    if (glrcbox->DestroyShader() < 0) {
        LOGW("call DestroyShader() failed\n");
    }
}

JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

    if (glrcbox->DrawFrame(__static_cast(GLuint, textureId)) < 0) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "drawFrame native method execute error");
        return;
    }
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
