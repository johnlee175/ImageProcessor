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
 * jni simple helper cc interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_JNI_SIMPLE_HELPER_HPP
#define IMGPROC_ANDROID_JNI_SIMPLE_HELPER_HPP

#include <jni.h>

#define JNI_INSTANCE_PARAM JNIEnv *env, jobject thiz
#define JNI_CLASS_PARAM JNIEnv *env, jclass klass

#define jnicchelper_field_id(field_name, field_sign) \
env->GetFieldID(env->GetObjectClass(thiz), field_name, field_sign)

#define jnicchelper_method_id(method_name, method_sign) \
env->GetMethodID(env->GetObjectClass(thiz), method_name, method_sign)

#define jnicchelper_obj_field_id(obj, field_name, field_sign) \
env->GetFieldID(env->GetObjectClass(obj), field_name, field_sign)

#define jnicchelper_obj_method_id(obj, method_name, method_sign) \
env->GetMethodID(env->GetObjectClass(obj), method_name, method_sign)

/* Example: jnicchelper_get_field("nativeContextPointer", "J", Long); */
#define jnicchelper_get_field(field_name, field_sign, type_cap) \
env->Get##type_cap##Field(thiz, \
env->GetFieldID(env->GetObjectClass(thiz), field_name, field_sign))

/* Example: jnicchelper_call_method("getName", "()Ljava/lang/String;", Object); */
#define jnicchelper_call_method(method_name, method_sign, type_cap) \
env->Call##type_cap##Method(thiz, \
env->GetMethodID(env->GetObjectClass(thiz), method_name, method_sign))

/* Example: jnicchelper_call_method_v("setName", "(Z)V", Void, JNI_FALSE); */
#define jnicchelper_call_method_v(method_name, method_sign, type_cap, ...) \
env->Call##type_cap##Method(thiz, \
env->GetMethodID(env->GetObjectClass(thiz), method_name, method_sign), __VA_ARGS__)

/* Example: jnicchelper_get_obj_field(jobj, "nativeContextPointer", "J", Long); */
#define jnicchelper_get_obj_field(obj, field_name, field_sign, type_cap) \
env->Get##type_cap##Field(obj, \
env->GetFieldID(env->GetObjectClass(obj), field_name, field_sign))

/* Example: jnicchelper_call_obj_method(jobj, "getName", "()Ljava/lang/String;", Object); */
#define jnicchelper_call_obj_method(obj, method_name, method_sign, type_cap) \
env->Call##type_cap##Method(obj, \
env->GetMethodID(env->GetObjectClass(obj), method_name, method_sign))

/* Example: jnicchelper_call_obj_method_v(jobj, "setName", "(Z)V", Void, JNI_FALSE); */
#define jnicchelper_call_obj_method_v(obj, method_name, method_sign, type_cap, ...) \
env->Call##type_cap##Method(obj, \
env->GetMethodID(env->GetObjectClass(obj), method_name, method_sign), __VA_ARGS__)

#define jnicchelper_global_empty_new(name, sign, ...) \
env->NewGlobalRef(env->NewObject(\
env->FindClass(name), env->GetMethodID(\
env->FindClass(name), "<init>", sign), __VA_ARGS__))

#define jnicchelper_get_native_ctx_ptr(env, thiz) \
env->GetLongField(thiz, env->GetFieldID(\
env->GetObjectClass(thiz), "nativeContextPointer", "J"))

#define jnicchelper_set_native_ctx_ptr(env, thiz, ptr) \
env->SetLongField(thiz, env->GetFieldID(\
env->GetObjectClass(thiz), "nativeContextPointer", "J"), ptr)

#define jnicchelper_call_native_ctx_ptr(env, thiz) \
env->CallLongMethod(thiz, env->GetMethodID(\
env->GetObjectClass(thiz), "getNativeContextPointer", "()J"))

#define jnicchelper_from_utf_string(env, str) \
env->GetStringUTFChars(str, JNI_FALSE)

#define jnicchelper_throw_exception(env, name, message) \
env->ThrowNew(env->FindClass(name), message)

#endif //IMGPROC_ANDROID_JNI_SIMPLE_HELPER_HPP
