#include <jni.h>
#include "BaseImageProc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* interface definitions */
JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_grey(JNIEnv *, jclass, jintArray, jint, jint);
JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_greyColor(JNIEnv *, jclass, jintArray, jint, jint);
JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_greyFilter(JNIEnv *, jclass, jintArray, jint, jint);
JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_colorFilter(JNIEnv *, jclass, jintArray, jint, jint);

#ifdef __cplusplus
}
#endif

/* interface implements */
JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_grey(JNIEnv *env, jclass klass,
                                                                     jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *pResult = (*env)->GetIntArrayElements(env, result, NULL);
    jint *pArgb = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_grey((int32_t *)pResult, (int32_t *)pArgb, size);


    (*env)->ReleaseIntArrayElements(env, argb, pArgb, 0);
    (*env)->ReleaseIntArrayElements(env, result, pResult, 0);

    return result;
}

JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_greyColor(JNIEnv *env, jclass klass,
                                                                          jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *pResult = (*env)->GetIntArrayElements(env, result, NULL);
    jint *pArgb = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_greyColor((int32_t *)pResult, (int32_t *)pArgb, size);


    (*env)->ReleaseIntArrayElements(env, argb, pArgb, 0);
    (*env)->ReleaseIntArrayElements(env, result, pResult, 0);

    return result;
}


int gamma_filter(int component) {
    float normal = component / 255.0F;
    float c = 6.0F;
    float y = 4.0F;
    float result = c * powf(normal, y);
    return (int) (result * 255.0F);
}

JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_greyFilter(JNIEnv *env, jclass klass,
                                                                          jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *pResult = (*env)->GetIntArrayElements(env, result, NULL);
    jint *pArgb = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_greyFilter((int32_t *)pResult, (int32_t *)pArgb, size, gamma_filter);

    (*env)->ReleaseIntArrayElements(env, argb, pArgb, 0);
    (*env)->ReleaseIntArrayElements(env, result, pResult, 0);

    return result;
}

JNIEXPORT jintArray JNICALL Java_com_johnsoft_alg_BaseImageProc_colorFilter(JNIEnv *env, jclass klass,
                                                                           jintArray argb, jint w, jint h) {
    jsize size = (jsize) w * h;
    jintArray result = (*env)->NewIntArray(env, size);

    jint *pResult = (*env)->GetIntArrayElements(env, result, NULL);
    jint *pArgb = (*env)->GetIntArrayElements(env, argb, NULL);

    calc_colorFilter((int32_t *)pResult, (int32_t *)pArgb, size, gamma_filter);

    (*env)->ReleaseIntArrayElements(env, argb, pArgb, 0);
    (*env)->ReleaseIntArrayElements(env, result, pResult, 0);

    return result;
}
