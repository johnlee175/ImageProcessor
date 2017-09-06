package com.johnsoft.imgproc.camera;

import java.lang.annotation.Documented;
import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Like java.lang.annotation.Native, but can using at JDK < 1.8 and Android SDK api < 26,
 * also can annotation at method or class which called from JNI or NDK.
 *
 * @author John Kenrinus Lee
 * @version 2017-09-05
 */
@Documented
@Target({ElementType.FIELD, ElementType.METHOD, ElementType.CONSTRUCTOR, ElementType.TYPE})
@Retention(RetentionPolicy.SOURCE)
public @interface Native {
}
