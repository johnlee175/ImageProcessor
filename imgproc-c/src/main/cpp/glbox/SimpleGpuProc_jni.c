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
 * @author John Kenrinus Lee
 * @version 2017-09-25
 */
#include "gpu_image.h"
#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROJECT_PACKAGE_NAME com_johnsoft_alg
#define CLASS_NAME SimpleGpuProc
#define JNI_SYMBOL(package_name, class_name, method_name) Java_##package_name##_##class_name##_##method_name
#define JNI_PREFIX(package_name, class_name, method_name, return_value) JNIEXPORT return_value JNICALL \
JNI_SYMBOL(package_name, class_name, method_name)
#define JNI_METHOD(return_value, method_name) JNI_PREFIX(PROJECT_PACKAGE_NAME, CLASS_NAME, method_name, return_value)
#define THROW_EXCEPTION(msg) (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/IllegalStateException"), msg);

/* interface definitions begin */

JNI_METHOD(void, nativeCreate)(JNIEnv *env, jobject thiz);

JNI_METHOD(void, nativeDestroy)(JNIEnv *env, jobject thiz);

JNI_METHOD(jintArray, nativeImageProc)(JNIEnv *env, jobject thiz, jintArray argb, jint w, jint h,
                                 jint type, jobject /* Map<String, Object> */ paramsMap);

/* interface definitions end */

#ifdef __cplusplus
}
#endif

static void copy_argb_ints_to_rgba_bytes(int32_t *from, uint8_t *to, int32_t from_size) {
    for (int32_t i = 0; i < from_size; ++i) {
        *to++ = _static_cast(uint8_t) ((from[i] >> 16) & 0xFF); // r
        *to++ = _static_cast(uint8_t) ((from[i] >> 8) & 0xFF); // g
        *to++ = _static_cast(uint8_t) ((from[i] >> 0) & 0xFF); // b
        *to++ = _static_cast(uint8_t) ((from[i] >> 24) & 0xFF); // a
    }
}

static void copy_rgba_bytes_to_argb_ints(uint8_t *from, int32_t *to, size_t from_size) {
    for (size_t i = 0; i < from_size; i += 4) {
        *to++ = _static_cast(int32_t) ((from[i + 3] << 24) + (from[i] << 16)
           + (from[i + 1] << 8) + (from[i + 2] << 0));
    }
}

GContext *gk_jni_context;

JNI_METHOD(void, nativeCreate)(JNIEnv *env, jobject thiz) {
    if (gk_jni_context == NULL) {
        gk_jni_context = glbox2_create_context(NULL);
    }
}

JNI_METHOD(void, nativeDestroy)(JNIEnv *env, jobject thiz) {
    if (gk_jni_context != NULL) {
        glbox2_destroy_context(gk_jni_context, NULL, NULL, 0);
        gk_jni_context = NULL;
    }
}

/* TODO reuse GProcessFlags object */
/* TODO memory may increasing while each call this method via OpenGL or OpenGLES */
JNI_METHOD(jintArray, nativeImageProc)(JNIEnv *env, jobject thiz, jintArray argb, jint w, jint h,
                                 jint type, jobject /* Map<String, Object> */ params_map) {
    const uint32_t channels = 4;

    jsize size = _static_cast(jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    jclass map_clazz = (*env)->GetObjectClass(env, params_map);
    jmethodID map_get_method = (*env)->GetMethodID(env, map_clazz, "get",
                                                   "(Ljava/lang/Object;)Ljava/lang/Object;");
    jstring key = (*env)->NewStringUTF(env, "fragment_shader_source");
    jobject frag_shader = (*env)->CallObjectMethod(env, params_map, map_get_method, key);
    jclass string_clazz = (*env)->FindClass(env, "java/lang/String");
    const char *shader_source = NULL;
    if (frag_shader != NULL && (*env)->IsInstanceOf(env, frag_shader, string_clazz)) {
        jstring frag_shader_string = _static_cast(jstring) frag_shader;
        shader_source = (*env)->GetStringUTFChars(env, frag_shader_string, NULL);
    }
    if (shader_source == NULL) {
        shader_source = default_fragment_shader_source;
    }

    DEFINE_HEAP_ARRAY_POINTER(uint8_t, origin_bytes, size * channels, {
        THROW_EXCEPTION("malloc origin_bytes[uint8_t] failed");
        return NULL; // abort from java
    });

    copy_argb_ints_to_rgba_bytes(argb_ptr, origin_bytes, size);

    DEFINE_HEAP_TYPE_POINTER(GImage, origin, {
        THROW_EXCEPTION("malloc origin[GImage] failed");
        return NULL; // abort from java
    });
    origin->width = _static_cast(uint32_t) w;
    origin->height = _static_cast(uint32_t) h;
    origin->channels = channels;
    origin->image = origin_bytes;

    DEFINE_HEAP_ARRAY_POINTER(uint8_t, target_bytes, size * channels, {
        THROW_EXCEPTION("malloc target_bytes[uint8_t] failed");
        return NULL; // abort from java
    });

    DEFINE_HEAP_TYPE_POINTER(GImage, target, {
        THROW_EXCEPTION("malloc target[GImage] failed");
        return NULL; // abort from java
    });
    target->width = _static_cast(uint32_t) w;
    target->height = _static_cast(uint32_t) h;
    target->channels = channels;
    target->image = target_bytes;

    DEFINE_HEAP_TYPE_POINTER(GProcessFlags, process_flags, {
        THROW_EXCEPTION("malloc process_flags[GProcessFlags] failed");
        return NULL; // abort from java
    });
    process_flags->fbo_by_texture = true;
    process_flags->reuse_fbo = true;
    process_flags->reuse_vao = true;
    process_flags->reuse_texture = true;
    process_flags->reuse_program = true;

    if (glbox2_image_process(gk_jni_context, target, origin, shader_source,
                             process_flags) < 0) {
        THROW_EXCEPTION("call glbox2_image_process failed");
        return NULL; // abort from java
    }

    copy_rgba_bytes_to_argb_ints(target->image, result_ptr, size * sizeof(jint));

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    FREE_POINTER(origin_bytes);
    FREE_POINTER(origin);
    FREE_POINTER(target_bytes);
    FREE_POINTER(target);
    FREE_POINTER(process_flags->private_data);
    FREE_POINTER(process_flags);

    return result;
}