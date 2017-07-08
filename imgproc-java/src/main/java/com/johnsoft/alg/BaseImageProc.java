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

    public static final int TYPE_GREY_COLOR = 0;
    public static final int TYPE_ARGB_COLOR = 5;
    public static final int COMPONENT_RED = 1;
    public static final int COMPONENT_GREEN = 2;
    public static final int COMPONENT_BLUE = 3;
    public static final int COMPONENT_ALPHA = 4;

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

    public static native synchronized int calcGrey(int argb);
    public static native synchronized int[] grey(int[] argb, int w, int h);
    public static native synchronized int[] greyColor(int[] argb, int w, int h);

    public static native synchronized double[] calcGreyFilter(double[] argb);
    public static native synchronized int[] greyFilter(int[] argb, int w, int h);
    public static native synchronized int[] colorFilter(int[] argb, int w, int h);

    public static native synchronized int[] greyBitPlaneSlicing(int[] argb, int w, int h, int bitPosition);
    public static native synchronized int[] colorBitPlaneSlicing(int[] argb, int w, int h, int bitPosition);
    public static native synchronized int[] colorComponentPlaneSlicing(int[] argb, int w, int h,
                                                                       @MagicConstant(intValues = {
                                                                               COMPONENT_ALPHA,
                                                                               COMPONENT_RED,
                                                                               COMPONENT_GREEN,
                                                                               COMPONENT_BLUE
                                                                       })
                                                                               int position,
                                                                       @MagicConstant(intValues = {
                                                                               TYPE_GREY_COLOR,
                                                                               TYPE_ARGB_COLOR
                                                                       })
                                                                               int type);

    public static native synchronized int[] combineSimplePlane(int[][] argbs, int w, int h);
    public static native synchronized int[] combineBitsPlane(int[] argb, int w, int h,
                                                             @MagicConstant(intValues = {
                                                                     TYPE_GREY_COLOR,
                                                                     TYPE_ARGB_COLOR
                                                             }) int type, int mask);

    /**
     * @param type  0=grey, 1=red, 2=green, 3=blue
     * @param index 0=no_filter, 1=reverse_filter, 2=linear_filter, 3=log_filter,
     *              4=gamma_filter, 5=stretch_filter, 6=binary_filter
     */
    public static native synchronized boolean filterIndex(
            @MagicConstant(intValues = {TYPE_GREY_COLOR, COMPONENT_RED, COMPONENT_GREEN, COMPONENT_BLUE})
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

    public static native synchronized int[] getAllColorCounts(int[] argb,
                                                              @MagicConstant(intValues = {
                                                                      TYPE_GREY_COLOR,
                                                                      COMPONENT_ALPHA,
                                                                      COMPONENT_RED,
                                                                      COMPONENT_GREEN,
                                                                      COMPONENT_BLUE
                                                              })
                                                                      int position);
    public static native synchronized int[] simpleHistogramEqualization(int[] argb,
                                                                        @MagicConstant(intValues = {
                                                                                TYPE_GREY_COLOR,
                                                                                COMPONENT_ALPHA,
                                                                                COMPONENT_RED,
                                                                                COMPONENT_GREEN,
                                                                                COMPONENT_BLUE
                                                                        })
                                                                                int position,
                                                                        boolean usingClassic);
}
