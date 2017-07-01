#include <jni.h>
#include "BaseImageProc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CLASS_NAME BaseImageProc
#define JNI_SYMBOL(package_name, class_name, method_name) Java_##package_name##_##class_name##_##method_name
#define JNI_PREFIX(package_name, class_name, method_name, return_value) JNIEXPORT return_value JNICALL \
JNI_SYMBOL(package_name, class_name, method_name)
#define JNI_METHOD(return_value, method_name) JNI_PREFIX(PROJECT_PACKAGE_NAME, CLASS_NAME, method_name, return_value)

/* interface definitions begin */
JNI_METHOD(jintArray, grey)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, greyColor)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, greyFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);
JNI_METHOD(jintArray, colorFilter)(JNIEnv *env, jclass klass, jintArray argb, jint w, jint h);

JNI_METHOD(jboolean, filterIndex)(JNIEnv *env, jclass klass, jint type, jint index);
JNI_METHOD(jboolean, filterParam)(JNIEnv *env, jclass klass, jfloat c, jfloat l, jfloat g);
JNI_METHOD(jboolean, filterParam2)(JNIEnv *env, jclass klass,
                                   jint lowLevel, jint lowPolicy, jint highLevel, jint highPolicy);
/* interface definitions end */

#ifdef __cplusplus
}
#endif

/* private utilities begin */

#define FILTER_TYPE_GREY 0
#define FILTER_TYPE_RED 1
#define FILTER_TYPE_GREEN 2
#define FILTER_TYPE_BLUE 3

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

JNI_METHOD(jboolean, filterIndex)(JNIEnv *env, jclass klass, jint type, jint index) {
    switch (type) {
        case FILTER_TYPE_GREY:
            grey_filter_reset();
            break;
        case FILTER_TYPE_RED:
            red_filter_reset();
            break;
        case FILTER_TYPE_GREEN:
            green_filter_reset();
            break;
        case FILTER_TYPE_BLUE:
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

/* interface implements end */