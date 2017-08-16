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

import static com.johnsoft.imgproc.camera.CameraManager.singleInstance;

import java.util.Random;

import android.content.pm.ActivityInfo;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.HorizontalScrollView;
import android.widget.LinearLayout;
import android.widget.ScrollView;

/**
 * Demo. Test on Google Nexus 6P.
 * @author John Kenrinus Lee
 * @version 2017-08-14
 */
public class CameraActivity extends AppCompatActivity {
    private static final String fragShaderCode1 = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  vec2 uv = vTextureCoord;\n"
            + "  if (vTextureCoord.y>0.5){\n"
            + "    uv.y = 1.0 - vTextureCoord.y;\n"
            + "  }\n"
            + "  gl_FragColor = texture2D(sTexture, uv);\n"
            + "}";
    private static final String fragShaderCode2 = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  vec4 color = texture2D(sTexture, vTextureCoord);\n"
            + "  gl_FragColor = vec4(1.0-color.r, 1.0-color.g, 1.0-color.b, 1.0);\n"
            + "}";
    private static final String fragShaderCode3 = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "const vec3 monoMultiplier = vec3(0.299, 0.587, 0.114);\n"
            + "const vec3 sepiaToneFactor = vec3(1.2, 1.0, 0.8);\n"
            + "void main() {\n"
            + "  vec4 color = texture2D(sTexture, vTextureCoord);\n"
            + "  float monoColor = dot(color.rgb,monoMultiplier);\n"
            + "  gl_FragColor = vec4(clamp(vec3(monoColor, monoColor, monoColor)*sepiaToneFactor, 0.0, 1.0), 1.0);\n"
            + "}";
    private static final String fragShaderCode4 = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "const float sampleDist = 1.0;\n"
            + "const float sampleStrength = 2.2; \n"
            + "void main() {\n"
            + "  float samples[10];\n"
            + "  samples[0] = -0.08; samples[1] = -0.05;\n"
            + "  samples[2] = -0.03; samples[3] = -0.02;\n"
            + "  samples[4] = -0.01; samples[5] = 0.01;\n"
            + "  samples[6] = 0.02;  samples[7] = 0.03;\n"
            + "  samples[8] = 0.05;  samples[9] = 0.08;\n"
            + "  vec2 dir = 0.5 - vTextureCoord; \n"
            + "  float dist = sqrt(dir.x*dir.x + dir.y*dir.y); \n"
            + "  dir = dir / dist; \n"
            + "  vec4 color = texture2D(sTexture, vTextureCoord); \n"
            + "  vec4 sum = color;\n"
            + "  for (int i = 0; i < 10; ++i) {\n"
            + "    sum += texture2D(sTexture, vTextureCoord + dir*samples[i]*sampleDist);\n"
            + "  }\n"
            + "  sum *= 1.0 / 11.0;\n"
            + "  float t = dist * sampleStrength;\n"
            + "  t = clamp(t, 0.0, 1.0);\n"
            + "  gl_FragColor = mix(color, sum, t);\n"
            + "} ";

    private static final Random rand = new Random();
    protected static long delayMillis = 500L;
    protected static boolean globalFullScreen = true;

    private Handler handler;
    private LinearLayout backLayout;
    private LinearLayout frontLayout;
    private int backCameraIndex;
    private int frontCameraIndex;
    private boolean hadFullView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        final Window window = getWindow();
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        window.addFlags(WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED);
        if (globalFullScreen) {
            window.requestFeature(Window.FEATURE_NO_TITLE);
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_FULLSCREEN);
            window.setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                    WindowManager.LayoutParams.FLAG_FULLSCREEN);
            final android.app.ActionBar actionBar = getActionBar();
            if (actionBar != null) {
                actionBar.hide();
            }
            final android.support.v7.app.ActionBar supportActionBar = getSupportActionBar();
            if (supportActionBar != null) {
                supportActionBar.hide();
            }
        }

        fullContentView();

        handler = new Handler();
        hadFullView = false;

        final CameraManager.DefaultPreviewSizeChooser previewSizeChooser = new CameraManager
                .DefaultPreviewSizeChooser(1280, 960);
        backCameraIndex = CameraManager.cameraIndex(false);
        frontCameraIndex = CameraManager.cameraIndex(true);
        CameraManager.singleInstance.flag(backCameraIndex, true, true, false);
        CameraManager.singleInstance.flag(frontCameraIndex, true, true, false);
        singleInstance.open(backCameraIndex, previewSizeChooser);
        singleInstance.open(frontCameraIndex, previewSizeChooser);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        singleInstance.close(backCameraIndex);
        singleInstance.close(frontCameraIndex);
    }

    @Override
    protected void onResume() {
        super.onResume();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                singleInstance.startPreview(backCameraIndex);
                singleInstance.startPreview(frontCameraIndex);
                fullCameraViews();
            }
        }, delayMillis);
    }

    @Override
    protected void onPause() {
        super.onPause();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                singleInstance.stopPreview(backCameraIndex);
                singleInstance.stopPreview(frontCameraIndex);
            }
        }, delayMillis);
    }

    private void fullContentView() {
        final LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT);
        lp.gravity = Gravity.CENTER;
        backLayout = new LinearLayout(this);
        backLayout.setLayoutParams(lp);
        backLayout.setOrientation(LinearLayout.HORIZONTAL);
        frontLayout = new LinearLayout(this);
        frontLayout.setLayoutParams(lp);
        frontLayout.setOrientation(LinearLayout.HORIZONTAL);

        final FrameLayout.LayoutParams lpHScrollView = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.START);
        final HorizontalScrollView backScrollView = new HorizontalScrollView(this);
        backScrollView.setLayoutParams(lpHScrollView);
        final HorizontalScrollView frontScrollView = new HorizontalScrollView(this);
        frontScrollView.setLayoutParams(lpHScrollView);

        final FrameLayout.LayoutParams lpLinearLayout = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.START);
        final LinearLayout linearLayout = new LinearLayout(this);
        linearLayout.setLayoutParams(lpLinearLayout);
        linearLayout.setOrientation(LinearLayout.VERTICAL);

        final FrameLayout.LayoutParams lpScrollView = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);
        final ScrollView scrollView = new ScrollView(this);
        scrollView.setLayoutParams(lpScrollView);

        backScrollView.addView(backLayout, lp);
        frontScrollView.addView(frontLayout, lp);
        linearLayout.addView(backScrollView, lpHScrollView);
        linearLayout.addView(frontScrollView, lpHScrollView);
        scrollView.addView(linearLayout, lpLinearLayout);
        setContentView(scrollView, lpScrollView);
    }

    private void fullCameraViews() {
        if (!hadFullView) {
            hadFullView = true;

            Point backSize = CameraManager.singleInstance.getFrameSize(backCameraIndex);
            if (backSize == null) {
                Log.w("CameraActivity", "back camera frame size return null!");
                backSize = new Point(0, 0);
            }
            Point frontSize = CameraManager.singleInstance.getFrameSize(frontCameraIndex);
            if (frontSize == null) {
                Log.w("CameraActivity", "front camera frame size return null!");
                frontSize = new Point(0, 0);
            }
            final LinearLayout.LayoutParams lpBack = new LinearLayout.LayoutParams(
                    backSize.x, backSize.y);
            final LinearLayout.LayoutParams lpFront = new LinearLayout.LayoutParams(
                    frontSize.x, frontSize.y);
            CameraView cameraView;

            cameraView = new CameraView(CameraActivity.this);
            cameraView.setCameraIndex(backCameraIndex).markAsFrontCamera(false);
            cameraView.setLayoutParams(lpBack);
            backLayout.addView(cameraView, 0, lpBack);

            cameraView = new CameraView(CameraActivity.this);
            cameraView.setShaderSourceCode(null, rand.nextBoolean() ? fragShaderCode3 : fragShaderCode4);
            cameraView.setCameraIndex(backCameraIndex).markAsFrontCamera(false);
            cameraView.setLayoutParams(lpBack);
            backLayout.addView(cameraView, 1, lpBack);

            cameraView = new CameraView(CameraActivity.this);
            cameraView.setCameraIndex(frontCameraIndex).markAsFrontCamera(true);
            cameraView.setLayoutParams(lpFront);
            frontLayout.addView(cameraView, 0, lpFront);

            cameraView = new CameraView(CameraActivity.this);
            cameraView.setShaderSourceCode(null, rand.nextBoolean() ? fragShaderCode1 : fragShaderCode2);
            cameraView.setCameraIndex(frontCameraIndex).markAsFrontCamera(true);
            cameraView.setLayoutParams(lpFront);
            frontLayout.addView(cameraView, 1, lpFront);
        }
    }
}
