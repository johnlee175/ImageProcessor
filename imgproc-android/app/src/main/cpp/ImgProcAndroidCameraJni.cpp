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
#include "base/base.h"
#include "ImgProcAndroidCameraJni.hpp"
#include "CCGLRenderCameraBox.hpp"
#include <android/native_window_jni.h>

struct JNIGLRenderCameraBox {
    JNIGLRenderCameraBox(JavaVM *jvm):vm(jvm), method(nullptr), buffer(nullptr),
                                      jVertexShaderSource(nullptr), jFragmentShaderSource(nullptr),
                                      vertexShaderSource(nullptr), fragmentShaderSource(nullptr) { }

    ~JNIGLRenderCameraBox() {
        if (vm) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, nullptr);
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

    void SetCallbackParameters(JNIEnv *env, jmethodID method, jobject buffer) {
        this->method = method;
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

    JavaVM *vm;
    jmethodID method;
    jobject buffer;
    jstring jVertexShaderSource;
    jstring jFragmentShaderSource;
    const char *vertexShaderSource;
    const char *fragmentShaderSource;
};

class MyCallback: public camerabox::CCFrameDataCallback {
public:
    MyCallback(JavaVM *jvm):vm(jvm) { }

    virtual ~MyCallback() {
        if (callback && vm) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, nullptr);
            env->DeleteGlobalRef(callback);
        }
    }

    void SetCallbackParameters(JNIEnv *env, jobject callback) {
        this->callback = env->NewGlobalRef(callback);
    }

    virtual void OnDataCallback(camerabox::CCGLRenderCameraBox *glrcbox) {
        JNIGLRenderCameraBox *jnibox = __static_cast(JNIGLRenderCameraBox *, glrcbox->GetUserTag());
        if (callback && vm && jnibox->method && jnibox->buffer) {
            JNIEnv *env;
            jnibox->vm->AttachCurrentThread(&env, nullptr);
            env->CallVoidMethod(callback, jnibox->method, jnibox->buffer);
        }
    }
private:
    JavaVM *vm;
    jobject callback;
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
    camerabox::CCGLRenderCameraBox *glrcbox = camerabox::CCGLRenderCameraBox::create();
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
        MyCallback *normalCallback = new MyCallback(vm);
        MyCallback *filteredCallback = new MyCallback(vm);
        glrcbox->SetFrameDataCallback(normalCallback, filteredCallback);
        JNIGLRenderCameraBox *jnibox = new JNIGLRenderCameraBox(vm);
        glrcbox->SetUserTag(jnibox);
    }
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    camerabox::CCGLRenderCameraBox *glrcbox = __reinterpret_cast(camerabox::CCGLRenderCameraBox *, ptr);

    JNIGLRenderCameraBox *jnibox = __static_cast(JNIGLRenderCameraBox *, glrcbox->GetUserTag());
    if (!jnibox) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "jnibox is NULL which getting from GetUserTag");
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
    const char *vertexShaderSource = nullptr;
    const char *fragmentShaderSource = nullptr;
    if (vertexSource) {
        jstring jVertexShaderSource = __static_cast(jstring, env->NewGlobalRef(vertexSource));
        vertexShaderSource = jnicchelper_from_utf_string(env, jVertexShaderSource);
        jnibox->SetVertexShaderSource(jVertexShaderSource, vertexShaderSource);
    }
    if (fragmentSource) {
        jstring jFragmentShaderSource = __static_cast(jstring, env->NewGlobalRef(fragmentSource));
        fragmentShaderSource = jnicchelper_from_utf_string(env, jFragmentShaderSource);
        jnibox->SetFragmentShaderSource(jFragmentShaderSource, fragmentShaderSource);
    }
    if (glrcbox->SetShaderSource(vertexShaderSource, fragmentShaderSource) == -1) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                    "setShaderSourceCode native method execute error");
        return;
    }

    jobject normalCallback = jnicchelper_call_method("getNormalFrameRgbaDataCallback",
                                                     "()Lcom/johnsoft/imgproc/camera/CameraView"
                                                             "$OnFrameRgbaDataCallback;", Object);
    jobject filteredCallback = jnicchelper_call_method("getFilteredFrameRgbaDataCallback",
                                                       "()Lcom/johnsoft/imgproc/camera/CameraView"
                                                               "$OnFrameRgbaDataCallback;", Object);
    if (normalCallback) {
        jobject byte_buffer = env->NewDirectByteBuffer(pixels, __static_cast(jlong, pixels_size));
        jmethodID mid = jnicchelper_obj_method_id(normalCallback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
        jnibox->SetCallbackParameters(env, mid, byte_buffer);
    } else if (filteredCallback) {
        jobject byte_buffer = env->NewDirectByteBuffer(pixels, __static_cast(jlong, pixels_size));
        jmethodID mid = jnicchelper_obj_method_id(filteredCallback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
        jnibox->SetCallbackParameters(env, mid, byte_buffer);
    }
    if (normalCallback) {
        MyCallback *myCallback = __static_cast(MyCallback *, glrcbox->GetNormalFrameDataCallback());
        if (!myCallback) {
            jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                        "myCallback is NULL which getting from GetNormalFrameDataCallback");
            return;
        }
        myCallback->SetCallbackParameters(env, normalCallback);
    }
    if (filteredCallback) {
        MyCallback *myCallback = __static_cast(MyCallback *, glrcbox->GetFilteredFrameDataCallback());
        if (!myCallback) {
            jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                        "myCallback is NULL which getting from GetFilteredFrameDataCallback");
            return;
        }
        myCallback->SetCallbackParameters(env, filteredCallback);
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

    MyCallback *myCallback;
    myCallback = __static_cast(MyCallback *, glrcbox->GetNormalFrameDataCallback());
    if (myCallback) {
        delete myCallback;
    }
    glrcbox->SetFrameDataCallback(nullptr, nullptr);
    myCallback = __static_cast(MyCallback *, glrcbox->GetFilteredFrameDataCallback());
    if (myCallback) {
        delete myCallback;
    }
    JNIGLRenderCameraBox *jnibox = __static_cast(JNIGLRenderCameraBox *, glrcbox->GetUserTag());
    if (jnibox) {
        delete jnibox;
    }
    glrcbox->SetUserTag(nullptr);

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
        return nullptr;
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
