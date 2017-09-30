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

import java.util.Map;

/**
 * Simple Gpu Process
 * @author John Kenrinus Lee
 * @version 2017-09-18
 */
public class SimpleGpuProc {
    static {
        System.loadLibrary("glbox");
    }

    private long nativeContextPointer;

    public SimpleGpuProc() {
        nativeCreate();
    }

    public synchronized int[] imageProc(int[] argb, int w, int h,
                                        int type, Map<String, Object> paramsMap) {
        try {
            return nativeImageProc(argb, w, h, type, paramsMap);
        } catch (Throwable thr) {
            handleError(thr);
            return null;
        }
    }

    public synchronized void destroy() {
        try {
            nativeDestroy();
        } catch (Throwable thr) {
            handleError(thr);
        }
    }

    private void handleError(Throwable thr) {
        thr.printStackTrace();
        try {
            Thread.sleep(1000L);
        } catch (InterruptedException e) {
            /* ignored */
        }
        System.exit(-1);
    }

    private native void nativeCreate();
    private native void nativeDestroy();

    private native int[] nativeImageProc(int[] argb, int w, int h,
                                   int type, Map<String, Object> paramsMap);
}
