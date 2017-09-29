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
 * jni entry c implement
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "base/base.h"
#include "imgproc_android_camera_jni.h"
#include "gl_render_camera_box.h"
#include <android/native_window_jni.h>

struct JNIGLRenderCameraBox {
    JavaVM *vm;
    jmethodID method;
    jobject normal_callback;
    jobject filtered_callback;
    jobject byte_buffer;
    jstring j_vertex_shader_source;
    jstring j_fragment_shader_source;
    const char *vertex_shader_source;
    const char *fragment_shader_source;
};

static void normal_callback_func(GLRenderCameraBox *glrcbox) {
    struct JNIGLRenderCameraBox *jni_glrcbox = _static_cast(struct JNIGLRenderCameraBox *)
            glrcbox_get_user_tag(glrcbox);
    if (jni_glrcbox && jni_glrcbox->vm && jni_glrcbox->method && jni_glrcbox->byte_buffer
        && jni_glrcbox->normal_callback) {
        JNIEnv *env;
        (*jni_glrcbox->vm)->AttachCurrentThread(jni_glrcbox->vm, &env, NULL);
        (*env)->CallVoidMethod(env, jni_glrcbox->normal_callback, jni_glrcbox->method,
                               jni_glrcbox->byte_buffer);
    }
}

static void filtered_callback_func(GLRenderCameraBox *glrcbox) {
    struct JNIGLRenderCameraBox *jni_glrcbox = _static_cast(struct JNIGLRenderCameraBox *)
            glrcbox_get_user_tag(glrcbox);
    if (jni_glrcbox && jni_glrcbox->vm && jni_glrcbox->method && jni_glrcbox->byte_buffer
        && jni_glrcbox->filtered_callback) {
        JNIEnv *env;
        (*jni_glrcbox->vm)->AttachCurrentThread(jni_glrcbox->vm, &env, NULL);
        (*env)->CallVoidMethod(env, jni_glrcbox->filtered_callback, jni_glrcbox->method,
                               jni_glrcbox->byte_buffer);
    }
}

/* =================== CameraManager.GLClientRenderNativeThread =================== */

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, swapBuffers)(JNI_INSTANCE_PARAM) {
    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_call_native_ctx_ptr(env, thiz);
    if (glrcbox_swap_buffers(glrcbox) < 0) {
        LOGW("call glrcbox_swap_buffers(GLRenderCameraBox *) failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(jboolean, CameraManager_00024GLClientRenderNativeThread, createEGL)(JNI_INSTANCE_PARAM,
                                                                               jobject surface) {
    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_call_native_ctx_ptr(env, thiz);
    if (glrcbox_set_window(glrcbox, ANativeWindow_fromSurface(env, surface)) < 0) {
        LOGW("call glrcbox_set_window(GLRenderCameraBox *) failed\n");
        return JNI_FALSE;
    }
    if (glrcbox_create_egl(glrcbox) < 0) {
        LOGW("call glrcbox_create_egl(GLRenderCameraBox *) failed\n");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNI_METHOD(void, CameraManager_00024GLClientRenderNativeThread, destroyEGL)(JNI_INSTANCE_PARAM,
                                                                            jobject surface) {
    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_call_native_ctx_ptr(env, thiz);
    if (glrcbox_destroy_egl(glrcbox) < 0) {
        LOGW("call glrcbox_destroy_egl(GLRenderCameraBox *) failed\n");
    }
    EGLNativeWindowType window = glrcbox_get_window(glrcbox);
    if (window) {
        ANativeWindow_release(window);
    }
    glrcbox_destroy_release(glrcbox);
}

/* ============================ CameraNativeView ============================ */

JNI_METHOD(void, CameraNativeView, nativeInitialized)(JNI_INSTANCE_PARAM) {
    GLRenderCameraBox *glrcbox = glrcbox_create_initialize();
    if (!glrcbox) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "nativeInitialized native method execute error");
        return;
    } else {
        jnihelper_set_native_ctx_ptr(env, thiz, _reinterpret_cast(jlong) glrcbox);
        glrcbox_set_frame_data_callback(glrcbox, GL_FALSE, normal_callback_func, filtered_callback_func);
        struct JNIGLRenderCameraBox *jni_glrcbox = _static_cast(struct JNIGLRenderCameraBox *)
                malloc(sizeof(struct JNIGLRenderCameraBox));
        if (!jni_glrcbox) {
            jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                      "nativeInitialized native method execute error");
            return;
        }
        memset(jni_glrcbox, 0, sizeof(struct JNIGLRenderCameraBox));
        glrcbox_set_user_tag(glrcbox, jni_glrcbox);
    }
}

JNI_METHOD(void, CameraNativeView, createShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    JavaVM *vm;
    if ((*env)->GetJavaVM(env, &vm)) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "can't get Java VM");
        return;
    }

    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_get_native_ctx_ptr(env, thiz);

    struct JNIGLRenderCameraBox *jni_glrcbox = _static_cast(struct JNIGLRenderCameraBox *)
            glrcbox_get_user_tag(glrcbox);
    if (!jni_glrcbox) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "jni_glrcbox is NULL which getting from user tag");
        return;
    }
    jni_glrcbox->vm = vm;

    jint fragment_shader_type = jnihelper_call_method("getFragmentShaderType", "()I", Int);
    switch(fragment_shader_type) {
        case -1:
            glrcbox_set_fragment_shader_type(glrcbox, REVERSE_X);
            break;
        case 1:
            glrcbox_set_fragment_shader_type(glrcbox, REVERSE_Y);
            break;
        default:
            glrcbox_set_fragment_shader_type(glrcbox, NORMAL);
            break;
    }

    jint frame_width = jnihelper_call_method("getFrameWidth", "()I", Int);
    jint frame_height = jnihelper_call_method("getFrameHeight", "()I", Int);
    glrcbox_set_frame_size(glrcbox, _static_cast(GLuint) frame_width, _static_cast(GLuint) frame_height);

    GLubyte *pixels;
    size_t pixels_size;
    glrcbox_get_pixels(glrcbox, &pixels, &pixels_size);

    jstring vertexSource = jnihelper_call_method("getVertexShaderSourceCode", "()Ljava/lang/String;", Object);
    jstring fragmentSource = jnihelper_call_method("getFragmentShaderSourceCode", "()Ljava/lang/String;", Object);
    if (vertexSource) {
        jni_glrcbox->j_vertex_shader_source = _static_cast(jstring) (*env)->NewGlobalRef(env, vertexSource);
        jni_glrcbox->vertex_shader_source = jnihelper_from_utf_string(env, vertexSource);
    }
    if (fragmentSource) {
        jni_glrcbox->j_fragment_shader_source = _static_cast(jstring) (*env)->NewGlobalRef(env, fragmentSource);
        jni_glrcbox->fragment_shader_source = jnihelper_from_utf_string(env, fragmentSource);
    }
    glrcbox_set_shader_source(glrcbox, jni_glrcbox->vertex_shader_source,
                                  jni_glrcbox->fragment_shader_source);

    jobject normal_callback = jnihelper_call_method("getNormalFrameRgbaDataCallback", "()Lcom/johnsoft/imgproc/camera/"
            "CameraView$OnFrameRgbaDataCallback;", Object);
    jobject filtered_callback = jnihelper_call_method("getFilteredFrameRgbaDataCallback", "()Lcom/johnsoft/imgproc/camera/"
            "CameraView$OnFrameRgbaDataCallback;", Object);
    if (normal_callback || filtered_callback) {
        jobject byte_buffer = (*env)->NewDirectByteBuffer(env, pixels, _static_cast(jlong) pixels_size);
        jni_glrcbox->byte_buffer = (*env)->NewGlobalRef(env, byte_buffer);
        if (filtered_callback) {
            jni_glrcbox->method = jnihelper_obj_method_id(filtered_callback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
            jni_glrcbox->filtered_callback = (*env)->NewGlobalRef(env, filtered_callback);
        }
        if (normal_callback) {
            jni_glrcbox->method = jnihelper_obj_method_id(normal_callback, "onFrameRgbaData", "(Ljava/nio/ByteBuffer;)V");
            jni_glrcbox->normal_callback = (*env)->NewGlobalRef(env, normal_callback);
        }
    }

    if (glrcbox_create_shader(glrcbox) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "createShaderAndBuffer native method execute error");
        return;
    }
}

JNI_METHOD(void, CameraNativeView, destroyShaderAndBuffer)(JNI_INSTANCE_PARAM) {
    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    struct JNIGLRenderCameraBox *jni_glrcbox = _static_cast(struct JNIGLRenderCameraBox *)
            glrcbox_get_user_tag(glrcbox);
    if (jni_glrcbox) {
        if (jni_glrcbox->normal_callback) {
            (*env)->DeleteGlobalRef(env, jni_glrcbox->normal_callback);
            jni_glrcbox->normal_callback = NULL;
        }
        if (jni_glrcbox->filtered_callback) {
            (*env)->DeleteGlobalRef(env, jni_glrcbox->filtered_callback);
            jni_glrcbox->filtered_callback = NULL;
        }
        if (jni_glrcbox->byte_buffer) {
            (*env)->DeleteGlobalRef(env, jni_glrcbox->byte_buffer);
            jni_glrcbox->byte_buffer = NULL;
        }
        if (jni_glrcbox->j_vertex_shader_source && jni_glrcbox->vertex_shader_source) {
            (*env)->ReleaseStringUTFChars(env, jni_glrcbox->j_vertex_shader_source,
                                          jni_glrcbox->vertex_shader_source);
            (*env)->DeleteGlobalRef(env, jni_glrcbox->j_vertex_shader_source);
            jni_glrcbox->j_vertex_shader_source = NULL;
            jni_glrcbox->vertex_shader_source = NULL;
        }
        if (jni_glrcbox->j_fragment_shader_source && jni_glrcbox->fragment_shader_source) {
            (*env)->ReleaseStringUTFChars(env, jni_glrcbox->j_fragment_shader_source,
                                          jni_glrcbox->fragment_shader_source);
            (*env)->DeleteGlobalRef(env, jni_glrcbox->j_fragment_shader_source);
            jni_glrcbox->j_fragment_shader_source = NULL;
            jni_glrcbox->fragment_shader_source = NULL;
        }
        free(jni_glrcbox);
        glrcbox_set_user_tag(glrcbox, NULL);
    }
    if (glrcbox_destroy_shader(glrcbox) < 0) {
        LOGW("call glrcbox_destroy_shader(GLRenderCameraBox *) failed\n");
    }
}

JNI_METHOD(void, CameraNativeView, drawFrame)(JNI_INSTANCE_PARAM, jint textureId) {
    GLRenderCameraBox *glrcbox = _reinterpret_cast(GLRenderCameraBox *)
            jnihelper_get_native_ctx_ptr(env, thiz);
    if (glrcbox_draw_frame(glrcbox, _static_cast(GLuint) textureId) < 0) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION, "drawFrame native method execute error");
        return;
    }
}

/* ====================== DirectByteBuffers.NativeDirectMemory ====================== */

JNI_METHOD(jobject/* ByteBuffer */, DirectByteBuffers_00024NativeDirectMemory, mallocDirect)(JNI_INSTANCE_PARAM,
                                                                                             jint capacity) {
    void *buffer = malloc(_static_cast(size_t) capacity);
    if (!buffer) {
        jnihelper_throw_exception(env, CAMERA_MANAGE_EXCEPTION,
                                  "mallocDirect native method execute error");
        return NULL;
    }
    memset(buffer, 0, _static_cast(size_t) capacity);
    return (*env)->NewDirectByteBuffer(env, buffer, capacity);
}

JNI_METHOD(void, DirectByteBuffers_00024NativeDirectMemory, freeDirect)(JNI_INSTANCE_PARAM, jobject byteBuffer) {
    void *buffer = (*env)->GetDirectBufferAddress(env, byteBuffer);
    if (buffer) {
        free(buffer);
    }
}
