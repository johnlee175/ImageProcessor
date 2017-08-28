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

import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;

import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;

/**
 * Test Picture in Picture. Test on Google Nexus 6P.
 * @author John Kenrinus Lee
 * @version 2017-08-14
 */
public class PipActivity extends AppCompatActivity implements CameraView.OnFrameRgbaDataCallback {
    protected static final Point testSize = new Point(400, 300);
    protected static long delayMillis = 500L;
    protected static boolean globalFullScreen = true;

    private Handler handler;
    private FrameLayout layout;
    private int backCameraIndex;
    private int frontCameraIndex;
    private volatile boolean hadFullView;
    private volatile boolean capture;
    private CameraView.FrameCallbackThread callbackThread;

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

        final Point screenSize = new Point();
        getWindowManager().getDefaultDisplay().getSize(screenSize);
        backCameraIndex = CameraManager.cameraIndex(false);
        frontCameraIndex = CameraManager.cameraIndex(true);
        CameraManager.singleInstance.flag(backCameraIndex, true, true, false);
        CameraManager.singleInstance.flag(frontCameraIndex, true, true, false);
        CameraManager.singleInstance.open(backCameraIndex, new CameraManager
                .CachedPreviewSizeChooser(screenSize.x, screenSize.y));
        CameraManager.singleInstance.open(frontCameraIndex, new CameraManager
                .CachedPreviewSizeChooser(testSize.x, testSize.y));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        CameraManager.singleInstance.close(backCameraIndex);
        CameraManager.singleInstance.close(frontCameraIndex);
        callbackThread.quit();
    }

    @Override
    protected void onResume() {
        super.onResume();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                CameraManager.singleInstance.startPreview(backCameraIndex);
                CameraManager.singleInstance.startPreview(frontCameraIndex);
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
                CameraManager.singleInstance.stopPreview(backCameraIndex);
                CameraManager.singleInstance.stopPreview(frontCameraIndex);
            }
        }, delayMillis);
    }

    private void fullContentView() {
        final FrameLayout.LayoutParams lpFrameLayout = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.START);
        layout = new FrameLayout(PipActivity.this);
        layout.setLayoutParams(lpFrameLayout);
        layout.setBackgroundColor(Color.BLACK);
        setContentView(layout, lpFrameLayout);
    }

    private void fullCameraViews() {
        if (!hadFullView) {
            hadFullView = true;

            callbackThread = new CameraView.FrameCallbackThread(this,
                    testSize.x * testSize.y * 4, DirectByteBuffers.createJavaDirectMemory());
            callbackThread.start();

            final FrameLayout.LayoutParams lpFront = new FrameLayout.LayoutParams(
                    testSize.x,
                    testSize.y,
                    Gravity.BOTTOM);

            final FrameLayout.LayoutParams lpBack = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    Gravity.CENTER);
            lpBack.leftMargin = lpBack.rightMargin = lpBack.topMargin = lpBack.bottomMargin = 20;

            CameraView cameraView;

            cameraView = new CameraJavaView(PipActivity.this);
            cameraView.markCameraIndex(backCameraIndex).markAsFrontCamera(false);
            cameraView.setLayoutParams(lpBack);
            layout.addView(cameraView, 0, lpBack);

            cameraView = new CameraJavaView(PipActivity.this);
            cameraView.markCameraIndex(frontCameraIndex).markAsFrontCamera(true)
                    .setOnFrameRgbaDataCallback(callbackThread);
            cameraView.setLayoutParams(lpFront);
            layout.addView(cameraView, 1, lpFront);
        }
    }

    @Override
    public void onFrameRgbaData(ByteBuffer rgba) {
        if (capture) {
            capture = false;
            try {
                final String filePath = "/sdcard/piptest" + System.currentTimeMillis()
                        + "_le_" + testSize.x + 'x' + testSize.y + ".rgba";
                final FileChannel channel = new RandomAccessFile(filePath, "rwd").getChannel();
                channel.write(rgba);
                channel.close();
                Log.i("PipActivity", "Capture rgba data in " + filePath);
            } catch (IOException e) {
                Log.w("PipActivity", e);
            }
        }
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
}
