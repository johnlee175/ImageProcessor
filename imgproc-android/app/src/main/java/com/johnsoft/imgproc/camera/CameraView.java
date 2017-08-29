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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.support.annotation.Keep;
import android.util.AttributeSet;
import android.view.TextureView;

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
@Keep
public abstract class CameraView extends TextureView
        implements TextureView.SurfaceTextureListener, CameraManager.GLTextureViewClient {
    private boolean isPaused;
    private boolean isFrontCamera;
    private int mCameraIndex;

    private int mFrameWidth;
    private int mFrameHeight;
    private SurfaceTexture mSurface;

    protected OnFrameRgbaDataCallback mFrameRgbaDataCallback;

    public CameraView(Context context) {
        super(context);
        setSurfaceTextureListener(this);
    }

    public CameraView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSurfaceTextureListener(this);
    }

    @Override
    public boolean isPaused() {
        return isPaused;
    }

    @Override
    public void setPaused(boolean isPaused) {
        this.isPaused = isPaused;
    }

    public CameraView markCameraIndex(int cameraIndex) {
        if (mSurface == null) {
            this.mCameraIndex = cameraIndex;
        } else {
            System.out.println("#markCameraIndex should be call before surface attach");
        }
        return this;
    }

    public CameraView markAsFrontCamera(boolean isFrontCamera) {
        if (mSurface == null) {
            this.isFrontCamera = isFrontCamera;
        } else {
            System.out.println("#markAsFrontCamera should be call before surface attach");
        }
        return this;
    }

    public CameraView setOnFrameRgbaDataCallback(OnFrameRgbaDataCallback callback) {
        if (callback == null) {
            this.mFrameRgbaDataCallback = null;
            return this;
        } else {
            if (mSurface == null) {
                this.mFrameRgbaDataCallback = callback;
            } else {
                System.out.println("#setOnFrameRgbaDataCallback should be call before surface attach");
            }
            return this;
        }
    }

    public abstract CameraView setShaderSourceCode(String vertexSource, String fragmentSource);

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, final int width, final int height) {
        this.mSurface = surface;
        this.mFrameWidth = width;
        this.mFrameHeight = height;
        CameraManager.singleInstance.attachGLTextureViewClient(mCameraIndex, this);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        this.mSurface = surface;
        this.mFrameWidth = width;
        this.mFrameHeight = height;
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        CameraManager.singleInstance.dettachGLTextureViewClient(mCameraIndex, this);
        this.mSurface = null;
        this.mFrameWidth = 0;
        this.mFrameHeight = 0;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        /* this method will be called when per frame extracted */
    }

    @Override
    public void onError(Throwable thr) {
        thr.printStackTrace();
    }

    /* @Override
    public SurfaceTexture getSurfaceTexture() {
        return mSurface;
    } */

    protected boolean isFrontCamera() {
        return isFrontCamera;
    }

    protected int getCameraIndex() {
        return mCameraIndex;
    }

    protected int getFrameWidth() {
        return mFrameWidth;
    }

    public int getFrameHeight() {
        return mFrameHeight;
    }

    public interface OnFrameRgbaDataCallback {
        /** this method should be processed quickly to make sure extract frame smoothly */
        void onFrameRgbaData(ByteBuffer rgba);
    }

    public static class FrameCallbackThread extends Thread
            implements CameraView.OnFrameRgbaDataCallback, LoopThread {
        private final OnFrameRgbaDataCallback callback;
        private final DirectByteBuffers.DirectMemory directMemory;
        private final ByteBuffer buffer;
        private volatile boolean loop;
        private volatile boolean paused;
        private volatile boolean busy;

        public FrameCallbackThread(OnFrameRgbaDataCallback callback, int capacity,
                                   DirectByteBuffers.DirectMemory directMemory) {
            this.callback = callback;
            this.directMemory = directMemory;
            this.buffer = directMemory.mallocDirect(capacity);
            this.buffer.order(ByteOrder.nativeOrder());
            this.buffer.clear();
            this.loop = true;
            this.paused = false;
            this.busy = false;
        }

        @Override
        public boolean isLoop() {
            return loop;
        }

        @Override
        public void quit() {
            this.loop = false;
            interrupt();
            try {
                join(1000L);
            } catch (InterruptedException e) {
                /* ignored */
            }
        }

        @Override
        public boolean isPaused() {
            return paused;
        }

        @Override
        public void setPaused(boolean paused) {
            this.paused = paused;
            sendNotification(null);
        }

        @Override
        public void run() {
            try {
                while (loop) {
                    while (isPaused()) {
                        synchronized(this) {
                            wait(1000L);
                        }
                    }
                    while (!busy) {
                        synchronized(this) {
                            wait(1000L);
                        }
                    }
                    buffer.flip();
                    try {
                        callback.onFrameRgbaData(buffer);
                    } catch (Throwable thr) {
                        thr.printStackTrace();
                    }
                    buffer.clear();
                    busy = false;
                }
            } catch (InterruptedException e) {
                onError(e);
            } finally {
                directMemory.freeDirect(buffer);
                System.out.println("FrameCallbackThread quit!");
            }
        }

        @Override
        public void onError(Throwable thr) {
            thr.printStackTrace();
        }

        @Override
        public synchronized void sendNotification(String message) {
            notifyAll();
        }

        @Override
        public void onFrameRgbaData(ByteBuffer rgba) {
            if (!busy) {
                buffer.put(rgba);
                rgba.rewind();
                busy = true;
                sendNotification(null);
            }
        }
    }
}
