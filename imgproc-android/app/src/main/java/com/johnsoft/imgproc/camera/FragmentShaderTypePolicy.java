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

import com.johnsoft.imgproc.camera.simple.SimpleCameraView;

import android.os.Build;

/**
 * FragmentShaderType Policy, for {@link CameraView#markFragmentShaderType(int)}
 * @author John Kenrinus Lee
 * @version 2017-09-05
 */
public class FragmentShaderTypePolicy {
    private static FragmentShaderTypePolicy defaultPolicy;

    public static FragmentShaderTypePolicy getDefault() {
        if (defaultPolicy == null) {
            synchronized(FragmentShaderTypePolicy.class) {
                if (defaultPolicy == null) {
                    defaultPolicy = new FragmentShaderTypePolicy();
                }
            }
        }
        return defaultPolicy;
    }

    public static void setDefault(FragmentShaderTypePolicy policy) {
        defaultPolicy = policy;
    }

    public CameraView apply(CameraView cameraView, boolean isFrontCamera) {
        if (isFrontCamera) {
            if ("Nexus 6P".equals(Build.MODEL)) {
                return cameraView.markFragmentShaderType(CameraView.FRAGMENT_SHADER_TYPE_REVERSE_Y);
            } else {
                return cameraView.markFragmentShaderType(CameraView.FRAGMENT_SHADER_TYPE_REVERSE_X);
            }
        } else {
            return cameraView.markFragmentShaderType(CameraView.FRAGMENT_SHADER_TYPE_NORMAL);
        }
    }

    public void apply(SimpleCameraView cameraView, boolean isFrontCamera) {
        if (isFrontCamera) {
            if ("Nexus 6P".equals(Build.MODEL)) {
                cameraView.setFragShaderType(CameraView.FRAGMENT_SHADER_TYPE_REVERSE_Y);
            } else {
                cameraView.setFragShaderType(CameraView.FRAGMENT_SHADER_TYPE_REVERSE_X);
            }
        } else {
            cameraView.setFragShaderType(CameraView.FRAGMENT_SHADER_TYPE_NORMAL);
        }
    }
}
