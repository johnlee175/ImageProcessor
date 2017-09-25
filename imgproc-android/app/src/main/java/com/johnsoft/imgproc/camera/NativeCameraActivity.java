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
 * Native Demo. Test on Google Nexus 6P.
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
public class NativeCameraActivity extends AppCompatActivity {
    protected static int NH = 960;
    protected static int NW = 1280;
    protected static long delayMillis = 500L;
    protected static boolean globalFullScreen = true;

    private Handler handler;
    private int cameraIndex;
    private FrameLayout layout;
    private volatile boolean hadFullView;
    private volatile boolean normalCapture;
    private volatile boolean filteredCapture;
    private CameraView.FrameCallbackThread normalCallbackThread;
    private CameraView.FrameCallbackThread filteredCallbackThread;

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
                .DefaultPreviewSizeChooser(NW, NH);
        cameraIndex = CameraManager.cameraIndex(false);
        CameraManager.singleInstance.flag(cameraIndex, true, true, false);
        CameraManager.singleInstance.open(cameraIndex, previewSizeChooser, null);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        CameraManager.singleInstance.close(cameraIndex);
        if (normalCallbackThread != null) {
            normalCallbackThread.quit();
        }
        if (filteredCallbackThread != null) {
            filteredCallbackThread.quit();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                CameraManager.singleInstance.startPreview(cameraIndex);
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
                CameraManager.singleInstance.stopPreview(cameraIndex);
            }
        }, delayMillis);
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
            normalCapture = true;
            filteredCapture = true;
            return true;
        }
        return super.onKeyUp(keyCode, event);
    }

    private void fullContentView() {
        final FrameLayout.LayoutParams lpFrameLayout = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
                Gravity.CENTER);
        layout = new FrameLayout(NativeCameraActivity.this);
        layout.setLayoutParams(lpFrameLayout);
        layout.setBackgroundColor(Color.BLACK);
        setContentView(layout, lpFrameLayout);
    }

    private void fullCameraViews() {
        if (!hadFullView) {
            hadFullView = true;
            normalCallbackThread = new CameraView.FrameCallbackThread(
                new CameraView.OnFrameRgbaDataCallback() {
                    @Override
                    public void onFrameRgbaData(ByteBuffer rgba) {
                        if (normalCapture) {
                            normalCapture = false;
                            try {
                                final String filePath = "/sdcard/nctest-normal" + System.currentTimeMillis()
                                        + "_le_" + NW + 'x' + NH + ".rgba";
                                final FileChannel channel = new RandomAccessFile(filePath, "rwd")
                                        .getChannel();
                                channel.write(rgba);
                                channel.close();
                                Log.i("NativeCameraActivity", "Capture rgba data in " + filePath);
                            } catch (IOException e) {
                                Log.w("NativeCameraActivity", e);
                            }
                        }
                    }
                }, NW * NH * 4, DirectByteBuffers.createNativeDirectMemory());
            normalCallbackThread.start();
            filteredCallbackThread = new CameraView.FrameCallbackThread(
                new CameraView.OnFrameRgbaDataCallback() {
                    @Override
                    public void onFrameRgbaData(ByteBuffer rgba) {
                        if (filteredCapture) {
                            filteredCapture = false;
                            try {
                                final String filePath = "/sdcard/nctest-filtered" + System.currentTimeMillis()
                                        + "_le_" + NW + 'x' + NH + ".rgba";
                                final FileChannel channel = new RandomAccessFile(filePath, "rwd")
                                        .getChannel();
                                channel.write(rgba);
                                channel.close();
                                Log.i("NativeCameraActivity", "Capture rgba data in " + filePath);
                            } catch (IOException e) {
                                Log.w("NativeCameraActivity", e);
                            }
                        }
                    }
                }, NW * NH * 4, DirectByteBuffers.createNativeDirectMemory());
            filteredCallbackThread.start();
            Point size = CameraManager.singleInstance.getFrameSize(cameraIndex);
            if (size == null) {
                Log.w("CameraActivity", "camera frame size return null!");
                size = new Point(0, 0);
            }
            final FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                    size.x, size.y);
            lp.gravity = Gravity.CENTER;
            final CameraView cameraView = new CameraNativeView(NativeCameraActivity.this);
            cameraView.markCameraIndex(cameraIndex);
            FragmentShaderTypePolicy.getDefault().apply(cameraView, false);
            cameraView.setShaderSourceCode(null, CameraActivity.fragShaderCode2);
            cameraView.setNormalFrameRgbaDataCallback(normalCallbackThread);
            cameraView.setFilteredFrameRgbaDataCallback(filteredCallbackThread);
            cameraView.setLayoutParams(lp);
            layout.addView(cameraView, 0, lp);
        }
    }
}
