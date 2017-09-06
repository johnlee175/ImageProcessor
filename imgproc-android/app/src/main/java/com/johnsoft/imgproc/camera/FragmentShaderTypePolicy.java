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
