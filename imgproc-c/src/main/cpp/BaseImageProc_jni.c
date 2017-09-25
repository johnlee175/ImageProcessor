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
#include <jni.h>
#include "BaseImageProc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PROJECT_PACKAGE_NAME com_johnsoft_alg
#define CLASS_NAME BaseImageProc
#define JNI_SYMBOL(package_name, class_name, method_name) Java_##package_name##_##class_name##_##method_name
#define JNI_PREFIX(package_name, class_name, method_name, return_value) JNIEXPORT return_value JNICALL \
JNI_SYMBOL(package_name, class_name, method_name)
#define JNI_METHOD(return_value, method_name) JNI_PREFIX(PROJECT_PACKAGE_NAME, CLASS_NAME, method_name, return_value)

/* interface definitions begin */
JNI_METHOD(jint, calcGrey)(JNIEnv *env, jclass klass, jint argb);
JNI_METHOD(jintArray, grey)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, greyColor)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jdoubleArray, calcGreyFilter)(JNIEnv *env, jclass klass, jdoubleArray argb);
JNI_METHOD(jintArray, greyFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, colorFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, greyBitPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                           jint w, jint h, jint bitPosition);
JNI_METHOD(jintArray, colorBitPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                            jint w, jint h, jint bitPosition);
JNI_METHOD(jintArray, colorComponentPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                            jint w, jint h, jint position, jint type);
JNI_METHOD(jintArray, combineSimplePlane)(JNIEnv *env, jclass klass, jobjectArray argbs, jint w, jint h);
JNI_METHOD(jintArray, combineBitsPlane)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h,
                                        jint type, jint mask);

JNI_METHOD(jboolean, filterIndex)(JNIEnv *env, jclass klass, jint type, jint index);
JNI_METHOD(jboolean, filterParam)(JNIEnv *env, jclass klass, jfloat c, jfloat l, jfloat g);
JNI_METHOD(jboolean, filterParam2)(JNIEnv *env, jclass klass,
                                   jint lowLevel, jint lowPolicy, jint highLevel, jint highPolicy);

JNI_METHOD(jintArray, getAllColorCounts)(JNIEnv *env, jclass klass, jintArray argb, jint position);
JNI_METHOD(jintArray, simpleHistogramEqualization)(JNIEnv *env, jclass klass, jintArray argb,
                                                   jint position, jboolean usingClassic);
/* interface definitions end */

#ifdef __cplusplus
}
#endif

/* private utilities begin */

#define FILTER_POLICY_SAME_LEVEL 0
#define FILTER_POLICY_MIN 1
#define FILTER_POLICY_MAX 2

typedef struct tagFilterInfo {
    int32_t index;
    float arg_c;
    float arg_l;
    float arg_g;
    int32_t low_value;
    int32_t low_policy;
    int32_t high_value;
    int32_t high_policy;
} FilterInfo;

FilterInfo grey_filter_info = { 0, 0, 1, 1, -1, 0, -1, 0 };
FilterInfo red_filter_info = { 0, 0, 1, 1, -1, 0, -1, 0 };
FilterInfo green_filter_info = { 0, 0, 1, 1, -1, 0, -1, 0 };
FilterInfo blue_filter_info = { 0, 0, 1, 1, -1, 0, -1, 0 };

FilterInfo *curr_filter_info_ptr;

void grey_filter_reset() {
    curr_filter_info_ptr = &grey_filter_info;
}

void red_filter_reset() {
    curr_filter_info_ptr = &red_filter_info;
}

void green_filter_reset() {
    curr_filter_info_ptr = &green_filter_info;
}

void blue_filter_reset() {
    curr_filter_info_ptr = &blue_filter_info;
}

void color_filter_reset() {
    static int8_t resetting_color_type = 0;
    switch (resetting_color_type) {
        case 0:
            red_filter_reset();
            break;
        case 1:
            green_filter_reset();
            break;
        case 2:
            blue_filter_reset();
            break;
        default:
            break;
    }
    if (resetting_color_type < 2) {
        resetting_color_type++;
    } else{
        resetting_color_type = 0;
    }
}

int32_t no_filter(int32_t component) {
    return component;
}

int32_t reverse_filter(int32_t component) {
    return (int32_t) (255 - component);
}

int32_t linear_filter(int32_t component) {
    return (int32_t) (curr_filter_info_ptr->arg_c + curr_filter_info_ptr->arg_l * component);
}

int32_t log_filter(int32_t component) {
    const double arg_c = curr_filter_info_ptr->arg_c;
    const double arg_l = curr_filter_info_ptr->arg_l;
    const double arg_g = curr_filter_info_ptr->arg_g;
    double normal = component / 255.0;
    double result = arg_c + arg_l * (log(normal * arg_g + 1.0) / (log(arg_g + 1.0) + 0.001));
    return (int32_t) (result * 255.0);
}

int32_t gamma_filter(int32_t component) {
    const float arg_c = curr_filter_info_ptr->arg_c;
    const float arg_l = curr_filter_info_ptr->arg_l;
    const float arg_g = curr_filter_info_ptr->arg_g;
    float normal = component / 255.0F;
    float result = arg_c + arg_l * (float) pow(normal, arg_g);
    return (int32_t) (result * 255.0F);
}

int32_t stretch_filter(int32_t component) {
    double normal = component / 255.0;
    double m = 0.5;
    double temp = m / (normal + 0.05);
    double e1 = log(1.0 / 0.05 - 1.0) / log(m / 0.05);
    double e2 = log(1.0 / 0.95 - 1.0) / log(m / 0.95);
    double e = ceil(fmin(e1, e2));
    double result = 1.0 / (1.0 + pow(temp, e));
    return (int32_t) (result * 255.0);
}

int32_t binary_filter(int32_t component) {
    const int32_t high_value = curr_filter_info_ptr->high_value;
    const int32_t high_policy = curr_filter_info_ptr->high_policy;
    const int32_t low_value = curr_filter_info_ptr->low_value;
    const int32_t low_policy = curr_filter_info_ptr->low_policy;
    if (high_value >= 0 && component >= high_value) {
        switch (high_policy) {
            case FILTER_POLICY_SAME_LEVEL:
                return high_value;
            case FILTER_POLICY_MAX:
                return 255;
            case FILTER_POLICY_MIN:
                return 0;
            default:
                return component;
        }
    } else if (low_value >= 0 && component <= low_value) {
        switch (low_policy) {
            case FILTER_POLICY_SAME_LEVEL:
                return low_value;
            case FILTER_POLICY_MAX:
                return 255;
            case FILTER_POLICY_MIN:
                return 0;
            default:
                return component;
        }
    } else {
        return component;
    }
}

component_filter grey_color_filters[] = {
        no_filter,
        reverse_filter,
        linear_filter,
        log_filter,
        gamma_filter,
        stretch_filter,
        binary_filter,
};

/* private utilities end */

/* interface implements begin */
JNI_METHOD(jint, calcGrey)(JNIEnv *env, jclass klass, jint argb) {
    return color_to_grey(argb);
}

JNI_METHOD(jintArray, grey)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_raw_grey((int32_t *) result_ptr, (int32_t *) argb_ptr, size);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, greyColor)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_grey_color((int32_t *) result_ptr, (int32_t *) argb_ptr, size);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jdoubleArray, calcGreyFilter)(JNIEnv *env, jclass klass, jdoubleArray argb) {
    jsize size = (*env)->GetArrayLength(env, argb);
    jdoubleArray result = (*env)->NewDoubleArray(env, size);

    jdouble *result_ptr = (*env)->GetDoubleArrayElements(env, result, NULL);
    jdouble *argb_ptr = (*env)->GetDoubleArrayElements(env, argb, NULL);

    calc_grey_normal_filter((double *) result_ptr, (double *) argb_ptr, size, grey_filter_reset,
                           grey_color_filters[grey_filter_info.index]);

    (*env)->ReleaseDoubleArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseDoubleArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, greyFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_grey_color_filter((int32_t *) result_ptr, (int32_t *) argb_ptr, size, grey_filter_reset,
                     grey_color_filters[grey_filter_info.index]);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, colorFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_rgb_color_filter((int32_t *) result_ptr, (int32_t *) argb_ptr, size, color_filter_reset,
                     grey_color_filters[red_filter_info.index],
                     grey_color_filters[green_filter_info.index],
                     grey_color_filters[blue_filter_info.index]);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, greyBitPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                           jint w, jint h, jint bitPosition) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_grey_bit_plane((int32_t *) result_ptr, (int32_t *) argb_ptr, size, bitPosition);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, colorBitPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                            jint w, jint h, jint bitPosition) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_rgb_bit_plane((int32_t *) result_ptr, (int32_t *) argb_ptr, size, bitPosition);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, colorComponentPlaneSlicing)(JNIEnv *env, jclass klass, jintArray argb,
                                                  jint w, jint h, jint position, jint type) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_rgb_component_plane((int32_t *) result_ptr, (int32_t *) argb_ptr, size, position, type);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jintArray, combineSimplePlane)(JNIEnv *env, jclass klass, jobjectArray argbs, jint w, jint h) {
    jsize size = (jsize) w * h;

    jintArray result = (*env)->NewIntArray(env, size);
    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);

    jsize length = (*env)->GetArrayLength(env, argbs);
    if (length > 0) {
        jintArray source = (jintArray) (*env)->GetObjectArrayElement(env, argbs, 0);
        jint *source_ptr = (*env)->GetIntArrayElements(env, source, NULL);
        memcpy(result_ptr, source_ptr, size);
        (*env)->ReleaseIntArrayElements(env, source, source_ptr, 0);

        if (length > 1) {
            for (jsize i = 1; i < length; ++i) {
                jintArray target = (jintArray) (*env)->GetObjectArrayElement(env, argbs,i);
                jint *target_ptr = (*env)->GetIntArrayElements(env, target, NULL);
                calc_combine_simple_plane((int32_t *) result_ptr, (int32_t *) target_ptr, size);
                (*env)->ReleaseIntArrayElements(env, target, target_ptr, 0);
            }
        }
    }

    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);
    return result;
}

JNI_METHOD(jintArray, combineBitsPlane)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h,
                                        jint type, jint mask) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_combine_bits_plane((int32_t *) result_ptr, (int32_t *) argb_ptr, size, type, mask);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);

    return result;
}

JNI_METHOD(jboolean, filterIndex)(JNIEnv *env, jclass klass, jint type, jint index) {
    switch (type) {
        case TYPE_GREY_COLOR:
            grey_filter_reset();
            break;
        case COMPONENT_RED:
            red_filter_reset();
            break;
        case COMPONENT_GREEN:
            green_filter_reset();
            break;
        case COMPONENT_BLUE:
            blue_filter_reset();
            break;
        default:
            break;
    }
    curr_filter_info_ptr->index = index;
    return JNI_TRUE;
}

JNI_METHOD(jboolean, filterParam)(JNIEnv *env, jclass klass, jfloat c, jfloat l, jfloat g) {
    curr_filter_info_ptr->arg_c = c;
    curr_filter_info_ptr->arg_l = l;
    curr_filter_info_ptr->arg_g = g;
    return JNI_TRUE;
}

JNI_METHOD(jboolean, filterParam2)(JNIEnv *env, jclass klass,
                                   jint lowLevel, jint lowPolicy, jint highLevel, jint highPolicy) {
    curr_filter_info_ptr->low_value = lowLevel;
    curr_filter_info_ptr->low_policy = lowPolicy;
    curr_filter_info_ptr->high_value = highLevel;
    curr_filter_info_ptr->high_policy = highPolicy;
    return JNI_TRUE;
}

JNI_METHOD(jintArray, getAllColorCounts)(JNIEnv *env, jclass klass, jintArray argb, jint position) {
    jsize size = (*env)->GetArrayLength(env, argb);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    int32_t *data_ptr;
    int32_t length;
    map_component_color_count(&data_ptr, &length, argb_ptr, size, position);

    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);

    jintArray result = (*env)->NewIntArray(env, length);
    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    memcpy(result_ptr, data_ptr, length * sizeof(int32_t));
    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);
    free(data_ptr);

    return result;
}

JNI_METHOD(jintArray, simpleHistogramEqualization)(JNIEnv *env, jclass klass, jintArray argb,
                                                   jint position, jboolean usingClassic) {
    jsize size = (*env)->GetArrayLength(env, argb);
    jint *argb_ptr = (*env)->GetIntArrayElements(env, argb, NULL);

    int32_t *map_ptr;
    int32_t *data_ptr;
    int32_t length;
    map_component_color_count(&data_ptr, &length, argb_ptr, size, position);
    if (usingClassic == JNI_FALSE) {
        histogram_equalization_photoshop(&map_ptr, data_ptr, length, size);
    } else{
        histogram_equalization_classic(&map_ptr, data_ptr, length, size);
    }
    free(data_ptr);
    map_component_equalization(&data_ptr, map_ptr, length, argb_ptr, size, position);
    free(map_ptr);
    (*env)->ReleaseIntArrayElements(env, argb, argb_ptr, 0);

    jintArray result = (*env)->NewIntArray(env, size);
    jint *result_ptr = (*env)->GetIntArrayElements(env, result, NULL);
    memcpy(result_ptr, data_ptr, size * sizeof(int32_t));

    (*env)->ReleaseIntArrayElements(env, result, result_ptr, 0);
    free(data_ptr);

    return result;
}
/* interface implements end */