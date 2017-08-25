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
#include "ImgProcAndroidCameraJni.hpp"
#include "JniSimpleHelper.hpp"
#include "CCCameraNativeView.hpp"
#include "CCGlClientRenderThread.hpp"

#include <android/native_window_jni.h>

/* =================== CameraManager.GLClientRenderNativeThread =================== */

struct JNIGLClientRenderThreadStruct {
    JavaVM *vm;
    jobject thiz;
};

static int glcrt_thread_count = 0;

static bool is_paused(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jboolean result = JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, isPaused)(env,
                                                                                              jni_glcrt->thiz);
        jni_glcrt->vm->DetachCurrentThread();
        return result != JNI_FALSE;
    }
    return false;
}

static void set_paused(void *user_tag_data, bool paused) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jboolean result = _static_cast(jboolean) (paused ? JNI_TRUE : JNI_FALSE);
        JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, setPaused)(env, jni_glcrt->thiz,
                                                                             result);
        jni_glcrt->vm->DetachCurrentThread();
    }
}

static void create_shader_and_buffer(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnicchelper_call_obj_method(client, "createShaderAndBuffer", "()V", Void);
        jni_glcrt->vm->DetachCurrentThread();
    }
}

static void destroy_shader_and_buffer(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnicchelper_call_obj_method(client, "destroyShaderAndBuffer", "()V", Void);
        jni_glcrt->vm->DetachCurrentThread();
    }
}

static void draw_client_frame(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject surfaceTextureHolder = jnicchelper_get_field("surfaceTextureHolder",
                                                           "Lcom/johnsoft/imgproc/camera/CameraManager$SurfaceTextureHolder;",
                                                           Object);
        jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnicchelper_call_obj_method_v(surfaceTextureHolder, "drawClientFrame",
                                    "(com/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;)V",
                                    Void, client);
        jni_glcrt->vm->DetachCurrentThread();
    }
}

static void glclient_thread_on_error(void *user_tag_data, const char *message) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        jni_glcrt->vm->AttachCurrentThread(&env, NULL);
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, message);
        jni_glcrt->vm->DetachCurrentThread();
    }
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, nativeInitialized)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if (env->GetJavaVM(&vm) == JNI_FALSE) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }
    char *buf = _static_cast(char *) malloc(sizeof("GLClientRenderNativeThread") + 5);
    sprintf(buf, "GLClientRenderNativeThread%d", glcrt_thread_count++);
    BoolProperty *boolProperty = NULL;
    Drawer *drawer = NULL;
    CCGlClientRenderThread *glcrt = new CCGlClientRenderThread(buf, &boolProperty, &drawer);
    if (!boolProperty || !drawer) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "native init failed");
    } else {
        struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
                malloc(sizeof(struct JNIGLClientRenderThreadStruct));
        memset(jni_glcrt, 0, sizeof(struct JNIGLClientRenderThreadStruct));
        jni_glcrt->vm = vm;
        jni_glcrt->thiz = thiz;
        boolProperty->user_tag_data = jni_glcrt;
        boolProperty->get_prop = is_paused;
        boolProperty->set_prop = set_paused;
        drawer->user_tag_data = jni_glcrt;
        drawer->create = create_shader_and_buffer;
        drawer->destroy = destroy_shader_and_buffer;
        drawer->draw = draw_client_frame;
        drawer->on_error = glclient_thread_on_error;
        jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jobject surfaceTexture = jnicchelper_call_obj_method(client, "getSurfaceTexture",
                                                           "()Landroid/graphics/SurfaceTexture;", Object);
        jobject window = jnicchelper_global_empty_new("Landroid/view/Surface;",
                                                    "(Landroid/graphics/SurfaceTexture;)V",
                                                    surfaceTexture);
        glcrt->setWindow(ANativeWindow_fromSurface(env, window));
        jnicchelper_set_native_ctx_ptr(env, thiz, _reinterpret_cast(jlong) glcrt);
    }
}

JNI_METHOD(jstring, CameraManager_00024GLClientRenderNativeThread, getName)(JNI_INSTANCE_PARAM) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    return env->NewStringUTF(glcrt->getName());
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isLoop)(JNI_INSTANCE_PARAM) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    return _static_cast(jboolean) glcrt->isLoop();
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, start)(JNI_INSTANCE_PARAM) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    glcrt->start();
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, quit)(JNI_INSTANCE_PARAM) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    glcrt->quit();
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isPaused)(JNI_INSTANCE_PARAM) {
    jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                         Object);
    return jnicchelper_call_obj_method(client, "isPaused", "()Z", Boolean);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, setPaused)(JNI_INSTANCE_PARAM, jboolean paused) {
    jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                         Object);
    jnicchelper_call_obj_method_v(client, "setPaused", "(Z)V", Void, paused);
    JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, sendNotification)(env, thiz, NULL);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onError)(JNI_INSTANCE_PARAM, jthrowable thr) {
    jobject client = jnicchelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                         Object);
    jnicchelper_call_obj_method_v(client, "onError", "(Ljava/lang/Throwable;)V", Void, thr);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, sendNotification)(JNI_INSTANCE_PARAM, jstring message) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    if (message) {
        const char *cmsg = env->GetStringUTFChars(message, JNI_FALSE);
        glcrt->handleMessage(cmsg);
    } else {
        glcrt->handleMessage(NULL);
    }
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onFrameAvailable)(JNI_INSTANCE_PARAM, jobject surfaceTexture) {
    CCGlClientRenderThread *glcrt = _reinterpret_cast(CCGlClientRenderThread *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    glcrt->setAvailable(true);
    JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, sendNotification)(env, thiz, NULL);
}

/* ============================ CameraNativeView ============================ */

struct JNICameraNativeViewStruct {
    JavaVM *vm;
    jmethodID method;
    jobject callback;
    jobject bytebuffer;
};

static void callback(CameraNativeViewStruct *cnv) {
    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            cnv_get_user_tag(cnv);
    if (jni_cnv->vm) {
        JNIEnv *env;
        jni_cnv->vm->AttachCurrentThread(&env, NULL);
        env->CallVoidMethod(jni_cnv->callback, jni_cnv->method, jni_cnv->bytebuffer);
        jni_cnv->vm->DetachCurrentThread();
    }
}

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM) {
    CCCameraNativeView *cnv = new CCCameraNativeView;
    jnicchelper_set_native_ctx_ptr(env, thiz, _reinterpret_cast(jlong) cnv);
    cnv->setFrameDataCallback(callback);
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if (env->GetJavaVM(&vm) == JNI_FALSE) {
        jnicchelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }

    CCCameraNativeView *cnv = _reinterpret_cast(CCCameraNativeView *)
            jnicchelper_get_native_ctx_ptr(env, thiz);

    jboolean is_front_camera = jnicchelper_call_method("isFrontCamera", "()B", Boolean);
    cnv->setFrontCamera(is_front_camera);

    jint frame_width = jnicchelper_call_method("getFrameWidth", "()I", Int);
    jint frame_height = jnicchelper_call_method("getFrameHeight", "()I", Int);
    cnv->setFrameSize(_static_cast(GLuint) frame_width, _static_cast(GLuint) frame_height);

    GLubyte *pixels;
    size_t pixels_size;
    cnv->readPixels(&pixels, &pixels_size);
    jobject bytebuffer = env->NewDirectByteBuffer(pixels, _static_cast(jlong) pixels_size);

    jobject callback = jnicchelper_get_field("mFrameRgbaDataCallback", "Lcom/johnsoft/imgproc/camera/"
            "CameraView$OnFrameRgbaDataCallback;", Object);

    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            malloc(sizeof(struct JNICameraNativeViewStruct));
    memset(jni_cnv, 0, sizeof(struct JNICameraNativeViewStruct));
    jni_cnv->vm = vm;
    jni_cnv->method = jnicchelper_method_id("onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
    jni_cnv->callback = env->NewWeakGlobalRef(callback);
    jni_cnv->bytebuffer = env->NewGlobalRef(bytebuffer);
    cnv->setUserTag(jni_cnv);
    cnv->createShader();
}

JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    CCCameraNativeView *cnv = _reinterpret_cast(CCCameraNativeView *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            cnv->getUserTag();
    if (jni_cnv) {
        env->DeleteWeakGlobalRef(jni_cnv->callback);
        env->DeleteGlobalRef(jni_cnv->bytebuffer);
        free(jni_cnv);
    }
    cnv->destroyShader();
    delete cnv;
    jnicchelper_set_native_ctx_ptr(env, thiz, 0);
}

JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId) {
    CCCameraNativeView *cnv = _reinterpret_cast(CCCameraNativeView *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    cnv->drawFrame(__static_cast(GLuint, textureId));
}

JNI_METHOD(jobject/* CameraView */, CameraNativeView, setShaderSourceCode)(JNI_INSTANCE_PARAM,
                                                                           jstring vertexSource,
                                                                           jstring fragmentSource) {
    CCCameraNativeView *cnv = _reinterpret_cast(CCCameraNativeView *)
            jnicchelper_get_native_ctx_ptr(env, thiz);
    cnv->setShaderSource(jnicchelper_from_utf_string(env, vertexSource),
                         jnicchelper_from_utf_string(env, fragmentSource));
    return thiz;
}

/* ====================== CameraNativeView.FrameCallbackThread ====================== */

JNI_METHOD(jobject/* ByteBuffer */, CameraNativeView_00024FrameCallbackThread, mallocDirect)(JNI_CLASS_PARAM, jint capacity) {
    void *buffer = malloc(__static_cast(size_t, capacity));
    memset(buffer, 0, __static_cast(size_t, capacity));
    jobject byteBuffer = env->NewDirectByteBuffer(buffer, capacity);
    return env->NewGlobalRef(byteBuffer);
}

JNI_METHOD(void, CameraNativeView_00024FrameCallbackThread, freeDirect)(JNI_CLASS_PARAM, jobject byteBuffer) {
    void *buffer = env->GetDirectBufferAddress(byteBuffer);
    env->DeleteGlobalRef(byteBuffer);
    if (buffer) {
        free(buffer);
    }
}
