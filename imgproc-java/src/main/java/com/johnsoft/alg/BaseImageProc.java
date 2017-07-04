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
package com.johnsoft.alg;

import org.intellij.lang.annotations.MagicConstant;

/**
 * @author John Kenrinus Lee
 * @version 2017-06-05
 */
public class BaseImageProc {
    static {
        System.loadLibrary("imgproc-1.0.0");
    }

    public static final int FILTER_TYPE_GREY = 0;
    public static final int FILTER_TYPE_RED = 1;
    public static final int FILTER_TYPE_GREEN = 2;
    public static final int FILTER_TYPE_BLUE = 3;

    public static final int COMPONENT_PLANE_POSITION_ALPHA = 0;
    public static final int COMPONENT_PLANE_POSITION_RED = 1;
    public static final int COMPONENT_PLANE_POSITION_GREEN = 2;
    public static final int COMPONENT_PLANE_POSITION_BLUE = 3;

    public static final int COMPONENT_PLANE_TYPE_COLOR = 0;
    public static final int COMPONENT_PLANE_TYPE_GREY = 1;

    public static final int INDEX_NO_FILTER = 0;
    public static final int INDEX_REVERSE_FILTER = 1;
    public static final int INDEX_LINEAR_FILTER = 2;
    public static final int INDEX_LOG_FILTER = 3;
    public static final int INDEX_GAMMA_FILTER = 4;
    public static final int INDEX_STRETCH_FILTER = 5;
    public static final int INDEX_BINARY_FILTER = 6;

    public static final int BINARY_FILTER_POLICY_SAME = 0;
    public static final int BINARY_FILTER_POLICY_MIN = 1;
    public static final int BINARY_FILTER_POLICY_MAX = 2;

    public static native synchronized int[] grey(int[] argb, int w, int h);
    public static native synchronized int[] greyColor(int[] argb, int w, int h);

    public static native synchronized int[] greyFilter(int[] argb, int w, int h);
    public static native synchronized int[] colorFilter(int[] argb, int w, int h);

    public static native synchronized int[] greyBitPlaneSlicing(int[] argb, int w, int h, int bitPosition);
    public static native synchronized int[] colorBitPlaneSlicing(int[] argb, int w, int h, int bitPosition);
    public static native synchronized int[] colorComponentPlaneSlicing(int[] argb, int w, int h,
                                                                       @MagicConstant(intValues = {
                                                                               COMPONENT_PLANE_POSITION_ALPHA,
                                                                               COMPONENT_PLANE_POSITION_RED,
                                                                               COMPONENT_PLANE_POSITION_GREEN,
                                                                               COMPONENT_PLANE_POSITION_BLUE
                                                                       })
                                                                               int position,
                                                                       @MagicConstant(intValues = {
                                                                               COMPONENT_PLANE_TYPE_COLOR,
                                                                               COMPONENT_PLANE_TYPE_GREY
                                                                       })
                                                                               int type);

    public static native synchronized int[] combineSimplePlane(int[][] argbs, int w, int h);

    /**
     * @param type  0=grey, 1=red, 2=green, 3=blue
     * @param index 0=no_filter, 1=reverse_filter, 2=linear_filter, 3=log_filter,
     *              4=gamma_filter, 5=stretch_filter, 6=binary_filter
     */
    public static native synchronized boolean filterIndex(
            @MagicConstant(intValues = {FILTER_TYPE_GREY, FILTER_TYPE_RED, FILTER_TYPE_GREEN, FILTER_TYPE_BLUE})
                    int type,
            @MagicConstant(intValues = {INDEX_NO_FILTER, INDEX_REVERSE_FILTER, INDEX_LINEAR_FILTER,
                    INDEX_LOG_FILTER, INDEX_GAMMA_FILTER, INDEX_STRETCH_FILTER, INDEX_BINARY_FILTER})
                    int index);
    /** like c + l * pow({filter-value}, g) */
    public static native synchronized boolean filterParam(float c, float l, float g);

    /**
     * require: level >= 0 && level <= 255
     * policy: 0=same as level, 1=value 0, 2=value 255, others=no change
     */
    public static native synchronized boolean filterParam2(int lowLevel,
                                                           @MagicConstant(intValues = {
                                                                   BINARY_FILTER_POLICY_SAME,
                                                                   BINARY_FILTER_POLICY_MIN,
                                                                   BINARY_FILTER_POLICY_MAX})
                                                                   int lowPolicy,
                                                           int highLevel,
                                                           @MagicConstant(intValues = {
                                                                   BINARY_FILTER_POLICY_SAME,
                                                                   BINARY_FILTER_POLICY_MIN,
                                                                   BINARY_FILTER_POLICY_MAX})
                                                                   int highPolicy);

}
