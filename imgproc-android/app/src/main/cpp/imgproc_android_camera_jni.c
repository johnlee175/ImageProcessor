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
 * jni entry implement
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "imgproc_android_camera_jni.h"
#include "jni_simple_helper.h"
#include "camera_native_view.h"
#include "glclient_render_thread.h"

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
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jboolean result = JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, isPaused)(env,
                               jni_glcrt->thiz);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
        return result == JNI_FALSE ? false : true;
    }
    return false;
}

static void set_paused(void *user_tag_data, bool paused) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jboolean result = _static_cast(jboolean) (paused ? JNI_TRUE : JNI_FALSE);
        JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, setPaused)(env, jni_glcrt->thiz,
                               result);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
    }
}

static void create_shader_and_buffer(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnihelper_call_obj_method(client, "createShaderAndBuffer", "()V", Void);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
    }
}

static void destroy_shader_and_buffer(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnihelper_call_obj_method(client, "destroyShaderAndBuffer", "()V", Void);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
    }
}

static void draw_client_frame(void *user_tag_data) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jobject thiz = jni_glcrt->thiz;
        jobject surfaceTextureHolder = jnihelper_get_field("surfaceTextureHolder",
                                                           "Lcom/johnsoft/imgproc/camera/CameraManager$SurfaceTextureHolder;",
                                                           Object);
        jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jnihelper_call_obj_method_v(surfaceTextureHolder, "drawClientFrame",
                                  "(com/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;)V",
                                  Void, client);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
    }
}

static void glclient_thread_on_error(void *user_tag_data, const char *message) {
    struct JNIGLClientRenderThreadStruct *jni_glcrt = _static_cast(struct JNIGLClientRenderThreadStruct *)
            user_tag_data;
    if (jni_glcrt->vm) {
        JNIEnv *env;
        (*jni_glcrt->vm)->AttachCurrentThread(jni_glcrt->vm, &env, NULL);
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, message);
        (*jni_glcrt->vm)->DetachCurrentThread(jni_glcrt->vm);
    }
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, nativeInitialized)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if ((*env)->GetJavaVM(env, &vm) == JNI_FALSE) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }
    char *buf = _static_cast(char *) malloc(sizeof("GLClientRenderNativeThread") + 5);
    sprintf(buf, "GLClientRenderNativeThread%d", glcrt_thread_count++);
    BoolProperty *boolProperty = NULL;
    Drawer *drawer = NULL;
    GLClientRenderThreadStruct *glcrt = glcrt_create_initialize(buf, &boolProperty, &drawer);
    if (!glcrt || !boolProperty || !drawer) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "native init failed");
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
        jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                             Object);
        jobject surfaceTexture = jnihelper_call_obj_method(client, "getSurfaceTexture",
                                                   "()Landroid/graphics/SurfaceTexture;", Object);
        jobject window = jnihelper_global_empty_new("Landroid/view/Surface;",
                                                    "(Landroid/graphics/SurfaceTexture;)V",
                                                    surfaceTexture);
        glcrt_set_window(glcrt, ANativeWindow_fromSurface(env, window));
        jnihelper_set_native_ctx_ptr(env, thiz, _reinterpret_cast(jlong) glcrt);
    }
}

JNI_METHOD(jstring, CameraManager_00024GLClientRenderNativeThread, getName)(JNI_INSTANCE_PARAM) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    const char *name;
    if (glcrt_get_name(glcrt, &name) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call glcrt_get_name failed");
    }
    return (*env)->NewStringUTF(env, name);
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isLoop)(JNI_INSTANCE_PARAM) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    bool loop = false;
    if (glcrt_get_loop(glcrt, &loop) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call glcrt_get_loop failed");
    }
    return _static_cast(jboolean) loop;
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, start)(JNI_INSTANCE_PARAM) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (glcrt_start(glcrt) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call glcrt_start failed");
    }
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, quit)(JNI_INSTANCE_PARAM) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (glcrt_quit(glcrt) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "call glcrt_quit failed");
    }
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, isPaused)(JNI_INSTANCE_PARAM) {
    jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                         Object);
    return jnihelper_call_obj_method(client, "isPaused", "()Z", Boolean);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, setPaused)(JNI_INSTANCE_PARAM, jboolean paused) {
    jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                                         Object);
    jnihelper_call_obj_method_v(client, "setPaused", "(Z)V", Void, paused);
    JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, sendNotification)(env, thiz, NULL);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onError)(JNI_INSTANCE_PARAM, jthrowable thr) {
    jobject client = jnihelper_get_field("client", "Lcom/johnsoft/imgproc/camera/CameraManager$GLTextureViewClient;",
                        Object);
    jnihelper_call_obj_method_v(client, "onError", "(Ljava/lang/Throwable;)V", Void, thr);
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, sendNotification)(JNI_INSTANCE_PARAM, jstring message) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (message) {
        const char *cmsg = (*env)->GetStringUTFChars(env, message, JNI_FALSE);
        if (glcrt_handle_message(glcrt, cmsg) < 0) {
            LOGI("call glcrt_handle_message failed");
        }
    } else {
        if (glcrt_handle_message(glcrt, NULL) < 0) {
            LOGI("call glcrt_handle_message failed");
        }
    }
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, onFrameAvailable)(JNI_INSTANCE_PARAM, jobject surfaceTexture) {
    GLClientRenderThreadStruct *glcrt = _reinterpret_cast(GLClientRenderThreadStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (glcrt_set_available(glcrt, true) < 0) {
        LOGI("call glcrt_set_available failed");
    }
    JNI_METHOD_NAME(CameraManager_00024GLClientRenderNativeThread, sendNotification)(env, thiz, NULL);
}

/* ============================ CameraNativeView ============================ */

struct JNICameraNativeViewStruct {
    JavaVM *vm;
    jmethodID method;
    jobject callback;
    jobject bytebuffer;
};

static void callback_func(CameraNativeViewStruct *cnv) {
    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            cnv_get_user_tag(cnv);
    if (jni_cnv->vm) {
        JNIEnv *env;
        (*jni_cnv->vm)->AttachCurrentThread(jni_cnv->vm, &env, NULL);
        (*env)->CallVoidMethod(env, jni_cnv->callback, jni_cnv->method, jni_cnv->bytebuffer);
        (*jni_cnv->vm)->DetachCurrentThread(jni_cnv->vm);
    }
}

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM) {
    CameraNativeViewStruct *cnv = cnv_create_initialize();
    if (!cnv) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "native init failed");
    } else {
        jnihelper_set_native_ctx_ptr(env, thiz, _reinterpret_cast(jlong) cnv);
        cnv_set_frame_data_callback(cnv, callback_func);
    }
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if ((*env)->GetJavaVM(env, &vm) == JNI_FALSE) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }

    CameraNativeViewStruct *cnv = _reinterpret_cast(CameraNativeViewStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);

    jboolean is_front_camera = jnihelper_call_method("isFrontCamera", "()B", Boolean);
    cnv_set_front_camera(cnv, is_front_camera);

    jint frame_width = jnihelper_call_method("getFrameWidth", "()I", Int);
    jint frame_height = jnihelper_call_method("getFrameHeight", "()I", Int);
    cnv_set_frame_size(cnv, _static_cast(GLuint) frame_width, _static_cast(GLuint) frame_height);

    GLubyte *pixels;
    size_t pixels_size;
    cnv_get_pixels(cnv, &pixels, &pixels_size);
    jobject bytebuffer = (*env)->NewDirectByteBuffer(env, pixels, _static_cast(jlong) pixels_size);

    jobject callback = jnihelper_get_field("mFrameRgbaDataCallback", "Lcom/johnsoft/imgproc/camera/"
                                           "CameraView$OnFrameRgbaDataCallback;", Object);

    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            malloc(sizeof(struct JNICameraNativeViewStruct));
    memset(jni_cnv, 0, sizeof(struct JNICameraNativeViewStruct));
    jni_cnv->vm = vm;
    jni_cnv->method = jnihelper_method_id("onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
    jni_cnv->callback = (*env)->NewWeakGlobalRef(env, callback);
    jni_cnv->bytebuffer = (*env)->NewGlobalRef(env, bytebuffer);
    cnv_set_user_tag(cnv, jni_cnv);

    cnv_create_shader(cnv);
}

JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    CameraNativeViewStruct *cnv = _reinterpret_cast(CameraNativeViewStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    struct JNICameraNativeViewStruct *jni_cnv = _static_cast(struct JNICameraNativeViewStruct *)
            cnv_get_user_tag(cnv);
    if (jni_cnv) {
        (*env)->DeleteWeakGlobalRef(env, jni_cnv->callback);
        (*env)->DeleteGlobalRef(env, jni_cnv->bytebuffer);
        free(jni_cnv);
    }
    if (cnv_destroy_shader(cnv) < 0) {
        LOGW("cnv_destroy_shader(CameraNativeViewStruct) call failed");
    }
    cnv_destroy_release(cnv);
    jnihelper_set_native_ctx_ptr(env, thiz, 0);
}

JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId) {
    CameraNativeViewStruct *cnv = _reinterpret_cast(CameraNativeViewStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (cnv_draw_frame(cnv, _static_cast(GLuint) textureId) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "drawFrame native method execute error");
    }
}

JNI_METHOD(jobject/* CameraView */, CameraNativeView, setShaderSourceCode)(JNI_INSTANCE_PARAM,
                                                                           jstring vertexSource,
                                                                           jstring fragmentSource) {
    CameraNativeViewStruct *cnv = _reinterpret_cast(CameraNativeViewStruct *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (cnv_set_shader_source(cnv, jnihelper_from_utf_string(env, vertexSource),
                          jnihelper_from_utf_string(env, fragmentSource)) == -1) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "setShaderSourceCode native method execute error");
    }
    return thiz;
}

/* ====================== CameraNativeView.FrameCallbackThread ====================== */

JNI_METHOD(jobject/* ByteBuffer */, CameraNativeView_00024FrameCallbackThread, mallocDirect)(JNI_CLASS_PARAM, jint capacity) {
    void *buffer = malloc(_static_cast(size_t) capacity);
    memset(buffer, 0, _static_cast(size_t) capacity);
    jobject byteBuffer = (*env)->NewDirectByteBuffer(env, buffer, capacity);
    return (*env)->NewGlobalRef(env, byteBuffer);
}

JNI_METHOD(void, CameraNativeView_00024FrameCallbackThread, freeDirect)(JNI_CLASS_PARAM, jobject byteBuffer) {
    void *buffer = (*env)->GetDirectBufferAddress(env, byteBuffer);
    (*env)->DeleteGlobalRef(env, byteBuffer);
    if (buffer) {
        free(buffer);
    }
}
