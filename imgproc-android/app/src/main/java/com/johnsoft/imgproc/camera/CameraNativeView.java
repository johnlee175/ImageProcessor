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
package com.johnsoft.imgproc.camera;

import android.content.Context;
import android.util.AttributeSet;

/**
 * Need following line to AndroidManifest.xml: {@code
 * <uses-feature android:name="android.hardware.camera" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.autofocus" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.front" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.front.autofocus" android:required="false"/>
 * <uses-feature android:glEsVersion="0x00020000" android:required="true" />
 * <uses-permission android:name="android.permission.CAMERA"/>
 * <supports-gl-texture android:name="GL_OES_compressed_ETC1_RGB8_texture" />
 * }
 *
 * @author John Kenrinus Lee
 * @version 2017-08-14
 */
public class CameraNativeView extends CameraView {
    static {
        System.loadLibrary("imgproc_android_camera");
    }

    private long nativeContextPointer;

    public CameraNativeView(Context context) {
        super(context);
        nativeInitialized();
    }

    public CameraNativeView(Context context, AttributeSet attrs) {
        super(context, attrs);
        nativeInitialized();
    }

    private native void nativeInitialized();

    @Override
    public native CameraView setShaderSourceCode(String vertexSource, String fragmentSource);

    /** not for user call */
    @Override
    public native void createShaderAndBuffer();

    /** not for user call */
    @Override
    public native void drawFrame(int textureId);

    /** not for user call */
    @Override
    public native void destroyShaderAndBuffer();

    /** not for user call */
    @Override
    public final long getNativeContextPointer() {
        return nativeContextPointer;
    }
}
