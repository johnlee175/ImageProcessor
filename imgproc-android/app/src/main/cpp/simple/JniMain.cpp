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
 * simple version jni entry, for SimpleCameraView
 *
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
#include "base/base.h"
#include "../JniSimpleHelper.hpp"
#include "SimpleCameraView.hpp"
#include <android/native_window_jni.h>

#define JNI_METHOD(return_value, class_name, method_name) JNIEXPORT return_value JNICALL \
Java_com_johnsoft_imgproc_camera_simple_##class_name##_##method_name

#define CAMERA_MANAGE_EXCEPTION "com/johnsoft/imgproc/camera/CameraManager$CameraManageException"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

JNI_METHOD(void, SimpleCameraView, nativeInit)(JNI_INSTANCE_PARAM,
                                               jobject callback, jint width, jint height,
                                               jobject surface, jint fragShaderType);
JNI_METHOD(void, SimpleCameraView, nativeRelease)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, createEgl)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, swapBuffers)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, destroyEgl)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, createShader)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, drawFrame)(JNI_INSTANCE_PARAM);
JNI_METHOD(void, SimpleCameraView, destroyShader)(JNI_INSTANCE_PARAM);

#ifdef __cplusplus
}
#endif /* __cplusplus */

class MyCallback: public FrameDataCallback {
public:
    MyCallback(JNIEnv *env, jobject callback, jsize length):vm(NULL), method(0), callback(0), buffer(0) {
        env->GetJavaVM(&this->vm);
        if (callback) {
            jbyteArray bytes = env->NewByteArray(length);
            this->method = jnicchelper_obj_method_id(callback, "onNativeFrameData", "([B)V");
            this->callback = env->NewGlobalRef(callback);
            this->buffer = env->NewGlobalRef(bytes);
        }
    }

    virtual ~MyCallback() {
        if (vm && method) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, NULL);
            if (callback) {
                env->DeleteGlobalRef(callback);
            }
            if (buffer) {
                env->DeleteGlobalRef(buffer);
            }
        }
    }

    virtual void onDataCallback(SimpleCameraView *view) {
        if (vm && method) {
            JNIEnv *env;
            vm->AttachCurrentThread(&env, NULL);
            GLubyte *bytes;
            GLuint s = view->ReadPixels(&bytes);
            jsize len = env->GetArrayLength(static_cast<jbyteArray>(buffer));
            jbyte *fp = env->GetByteArrayElements(static_cast<jbyteArray>(buffer), NULL);
            if (bytes && fp && s == len) {
                memcpy(fp, bytes, s);
            }
            env->ReleaseByteArrayElements(static_cast<jbyteArray>(buffer), fp, 0);
            env->CallVoidMethod(callback, method, buffer);
        }
    }
private:
    JavaVM *vm;
    jmethodID method;
    jobject callback;
    jobject buffer;
};

JNI_METHOD(void, SimpleCameraView, nativeInit)(JNI_INSTANCE_PARAM,
                                               jobject callback, jint width, jint height,
                                               jobject surface, jint fragShaderType) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    MyCallback *myCallback = NULL;
    if (callback != NULL) {
        myCallback = new MyCallback(env, callback, width * height * 4);
    }
    FragmentShaderType fst;
    switch (fragShaderType) {
        case -1:
            fst = FST_REVERSE_X;
            break;
        case 1:
            fst = FST_REVERSE_Y;
            break;
        default:
            fst = FST_NORMAL;
            break;
    }
    SimpleCameraView *view = new SimpleCameraView(myCallback, static_cast<GLuint>(width),
                                                  static_cast<GLuint>(height), window, fst);
    jnicchelper_set_native_ctx_ptr(env, thiz, reinterpret_cast<jlong>(view));
}

JNI_METHOD(void, SimpleCameraView, nativeRelease)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    ANativeWindow *window = static_cast<ANativeWindow *>(view->GetWindow());
    if (window) {
        ANativeWindow_release(window);
    }
    delete view;
    jnicchelper_set_native_ctx_ptr(env, thiz, 0);
}

JNI_METHOD(void, SimpleCameraView, createEgl)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    if (!view->CreateEgl()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call createEgl() failed");
    }
}

JNI_METHOD(void, SimpleCameraView, swapBuffers)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    if (!view->SwapBuffers()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call SwapBuffers() failed");
    }
}

JNI_METHOD(void, SimpleCameraView, destroyEgl)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    view->DestroyEgl();
}

JNI_METHOD(void, SimpleCameraView, createShader)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    if (!view->CreateShader()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call createShader() failed");
    }
}

JNI_METHOD(void, SimpleCameraView, drawFrame)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    if (!view->DrawFrame()) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call drawFrame() failed");
    }
}

JNI_METHOD(void, SimpleCameraView, destroyShader)(JNI_INSTANCE_PARAM) {
    jlong ptr = jnicchelper_get_native_ctx_ptr(env, thiz);
    SimpleCameraView *view = reinterpret_cast<SimpleCameraView *>(ptr);
    view->DestroyShader();
}
