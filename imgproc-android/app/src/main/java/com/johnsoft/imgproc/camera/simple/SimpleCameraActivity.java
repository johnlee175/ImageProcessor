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
package com.johnsoft.imgproc.camera.simple;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import com.johnsoft.imgproc.camera.FragmentShaderTypePolicy;

import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.Toast;

/**
 * Simple Camera Activity. Test on Google Nexus 6P.
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
public class SimpleCameraActivity extends AppCompatActivity {
    protected static boolean globalFullScreen = true;
    protected static int sWidth = 1280;
    protected static int sHeight = 960;
    private SimpleCameraView cameraView;
    private SimpleCameraView.TransferThread callbackThread;
    private boolean capture;

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
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        callbackThread.quit();
    }

    @Override
    protected void onResume() {
        super.onResume();
        cameraView.resume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        cameraView.pause();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) {
            capture = true;
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    void capturePicture(byte[] rgba, int w, int h) {
        if (capture) {
            capture = false;
            try {
                final String filePath = "/sdcard/simple" + System.currentTimeMillis()
                        + "_le_" + w + 'x' + h + ".rgba";
                BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(filePath));
                bos.write(rgba);
                bos.close();
                Log.i("SimpleCameraActivity", "Capture rgba data in " + filePath);
            } catch (IOException e) {
                Log.w("SimpleCameraActivity", e);
            }
        }
    }

    private void fullContentView() {
        final FrameLayout.LayoutParams lpCameraView = new FrameLayout.LayoutParams(
                sWidth, sHeight, Gravity.CENTER);
        final FrameLayout.LayoutParams lpFrameLayout = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);

        cameraView = new SimpleCameraView(this);
        cameraView.setLayoutParams(lpCameraView);
        final FrameLayout frameLayout = new FrameLayout(this);
        frameLayout.setLayoutParams(lpFrameLayout);

        frameLayout.setBackgroundColor(Color.BLACK);

        frameLayout.addView(cameraView, lpCameraView);
        setContentView(frameLayout, lpFrameLayout);

        FragmentShaderTypePolicy.getDefault().apply(cameraView,
                SimpleCameraManager.FACE == Camera.CameraInfo.CAMERA_FACING_FRONT);

        callbackThread = new SimpleCameraView.TransferThread(sWidth * sHeight * 4) {
            @Override
            public void onFrameData(byte[] rgba) {
                capturePicture(rgba, sWidth, sHeight);
            }
            @Override
            public void onCreate(int width, int height) {
            }
            @Override
            public void onDestroy() {
            }
        };
        callbackThread.start();
        cameraView.setFrameDataCallback(callbackThread);

        cameraView.setOnTopTrackListener(Configuration.ORIENTATION_LANDSCAPE,
            new SimpleCameraView.OnTopTrackListener() {
                @Override
                public void onTopChanged(boolean correct) {
                    final String tip;
                    if (!correct) {
                        tip = "Top side not on top";
                    } else {
                        tip = "Top side back to top";
                    }
                    Toast.makeText(SimpleCameraActivity.this, tip, Toast.LENGTH_SHORT).show();
                }
        });
    }
}
