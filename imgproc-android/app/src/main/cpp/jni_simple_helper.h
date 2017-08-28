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
 * jni simple helper c interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_JNI_SIMPLE_HELPER_H
#define IMGPROC_ANDROID_JNI_SIMPLE_HELPER_H

#include <jni.h>

#define JNI_INSTANCE_PARAM JNIEnv *env, jobject thiz
#define JNI_CLASS_PARAM JNIEnv *env, jclass klass

#define jnihelper_field_id(field_name, field_sign) \
(*env)->GetFieldID(env, (*env)->GetObjectClass(env, thiz), field_name, field_sign)

#define jnihelper_method_id(method_name, method_sign) \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, thiz), method_name, method_sign)

#define jnihelper_obj_field_id(obj, field_name, field_sign) \
(*env)->GetFieldID(env, (*env)->GetObjectClass(env, obj), field_name, field_sign)

#define jnihelper_obj_method_id(obj, method_name, method_sign) \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, obj), method_name, method_sign)

/* Example: jnihelper_get_field("nativeContextPointer", "J", Long); */
#define jnihelper_get_field(field_name, field_sign, type_cap) \
(*env)->Get##type_cap##Field(env, thiz, \
(*env)->GetFieldID(env, (*env)->GetObjectClass(env, thiz), field_name, field_sign))

/* Example: jnihelper_call_method("getName", "Ljava/lang/String;()", Object); */
#define jnihelper_call_method(method_name, method_sign, type_cap) \
(*env)->Call##type_cap##Method(env, thiz, \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, thiz), method_name, method_sign))

/* Example: jnihelper_call_method_v("setName", "V(Z)", Void, JNI_FALSE); */
#define jnihelper_call_method_v(method_name, method_sign, type_cap, ...) \
(*env)->Call##type_cap##Method(env, thiz, \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, thiz), method_name, method_sign), __VA_ARGS__)

/* Example: jnihelper_get_obj_field(jobj, "nativeContextPointer", "J", Long); */
#define jnihelper_get_obj_field(obj, field_name, field_sign, type_cap) \
(*env)->Get##type_cap##Field(env, obj, \
(*env)->GetFieldID(env, (*env)->GetObjectClass(env, obj), field_name, field_sign))

/* Example: jnihelper_call_obj_method(jobj, "getName", "Ljava/lang/String;()", Object); */
#define jnihelper_call_obj_method(obj, method_name, method_sign, type_cap) \
(*env)->Call##type_cap##Method(env, obj, \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, obj), method_name, method_sign))

/* Example: jnihelper_call_obj_method_v(jobj, "setName", "V(Z)", Void, JNI_FALSE); */
#define jnihelper_call_obj_method_v(obj, method_name, method_sign, type_cap, ...) \
(*env)->Call##type_cap##Method(env, obj, \
(*env)->GetMethodID(env, (*env)->GetObjectClass(env, obj), method_name, method_sign), __VA_ARGS__)

#define jnihelper_global_empty_new(name, sign, ...) \
(*env)->NewGlobalRef(env, (*env)->NewObject(env, \
(*env)->FindClass(env, name), (*env)->GetMethodID(env, \
(*env)->FindClass(env, name), "<init>", sign), __VA_ARGS__))

#define jnihelper_get_native_ctx_ptr(env, thiz) \
(*env)->GetLongField(env, thiz, (*env)->GetFieldID(env, \
(*env)->GetObjectClass(env, thiz), "nativeContextPointer", "J"))

#define jnihelper_set_native_ctx_ptr(env, thiz, ptr) \
(*env)->SetLongField(env, thiz, (*env)->GetFieldID(env, \
(*env)->GetObjectClass(env, thiz), "nativeContextPointer", "J"), ptr)

#define jnihelper_call_native_ctx_ptr(env, thiz) \
(*env)->CallLongMethod(env, thiz, (*env)->GetMethodID(env, \
(*env)->GetObjectClass(env, thiz), "getNativeContextPointer", "()J"))

#define jnihelper_from_utf_string(env, str) \
(*env)->GetStringUTFChars(env, str, JNI_FALSE)

#define jnihelper_throw_exception(env, name, message) \
(*env)->ThrowNew(env, (*env)->FindClass(env, name), message)

#endif /* IMGPROC_ANDROID_JNI_SIMPLE_HELPER_H */
