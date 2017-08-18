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
import java.nio.FloatBuffer;
import java.nio.ShortBuffer;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.AttributeSet;
import android.view.TextureView;

/**
 * Need following line to AndroidManifest.xml:
 * <uses-feature android:name="android.hardware.camera" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.autofocus" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.front" android:required="false"/>
 * <uses-feature android:name="android.hardware.camera.front.autofocus" android:required="false"/>
 * <uses-feature android:glEsVersion="0x00020000" android:required="true" />
 * <uses-permission android:name="android.permission.CAMERA"/>
 * <supports-gl-texture android:name="GL_OES_compressed_ETC1_RGB8_texture" />
 *
 * @author John Kenrinus Lee
 * @version 2017-08-14
 */
public class CameraView extends TextureView
        implements TextureView.SurfaceTextureListener, CameraManager.GLTextureViewClient {
    private static final float[] vertexCoords = {
            -1.0f, 1.0f, 0.0f, // top left
            -1.0f, -1.0f, 0.0f, // bottom left
            1.0f, -1.0f, 0.0f, // bottom right
            1.0f, 1.0f, 0.0f, // top right
    };

    // texture coordinate in android OpenGL es:
    // is not from bottom left to top right, just from top left to bottom right!
    private static final float[] backTextureCoords = { // rotate 0 degree
            0.0f, 0.0f, // top left
            0.0f, 1.0f, // bottom left
            1.0f, 1.0f, // bottom right
            1.0f, 0.0f, // top right
    };

    // texture coordinate in android OpenGL es:
    // is not from bottom left to top right, just from top left to bottom right!
    private static final float[] frontTextureCoords = { // rotate 180 degree
            1.0f, 1.0f, // bottom right
            1.0f, 0.0f, // top right
            0.0f, 0.0f, // top left
            0.0f, 1.0f, // bottom left
    };

    private static final short[] drawOrder = { 0, 1, 2, 0, 2, 3 };

    private static final int VERTEX_COMPONENT = 3;
    private static final int TEXTURE_COMPONENT = 2;

    private static final String defaultVertexShaderSource = ""
            + "attribute vec4 aPosition;\n"
            + "attribute vec2 aTextureCoord;\n"
            + "varying vec2 vTextureCoord;\n"
            + "void main() {\n"
            + "  gl_Position = aPosition;\n"
            + "  vTextureCoord = aTextureCoord;\n"
            + "}";
    private static final String defaultFragmentShaderSource = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
            + "}";

    private boolean mIsFrontCamera;
    private int mCameraIndex;
    private int mFrameWidth;
    private int mFrameHeight;
    private SurfaceTexture mSurface;

    private int mProgram;
    private int mVertexShader;
    private int mFragmentShader;
    private String mVertexShaderSourceCode = defaultVertexShaderSource;
    private String mFragmentShaderSourceCode = defaultFragmentShaderSource;
    private FloatBuffer mVertexBuffer;
    private FloatBuffer mTextureBuffer;
    private ShortBuffer mDrawListBuffer;
    private ByteBuffer mPixelByteBuffer;
    private OnFrameRgbaDataCallback mFrameRgbaDataCallback;

    private boolean paused;

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
        return paused;
    }

    public CameraView setPaused(boolean paused) {
        this.paused = paused;
        return this;
    }

    public CameraView markAsFrontCamera(boolean isFrontCamera) {
        this.mIsFrontCamera = isFrontCamera;
        return this;
    }

    public CameraView setCameraIndex(int cameraIndex) {
        this.mCameraIndex = cameraIndex;
        return this;
    }

    public CameraView setShaderSourceCode(String vertexSource, String fragmentSource) {
        if (vertexSource != null) {
            this.mVertexShaderSourceCode = vertexSource;
        }
        if (fragmentSource != null) {
            this.mFragmentShaderSourceCode = fragmentSource;
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
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        /* this method will be called when per frame extracted */
    }

    /* @Override
    public SurfaceTexture getSurfaceTexture() {
        return mSurface;
    } */

    @Override
    public void onError(Throwable thr) {
        thr.printStackTrace();
    }

    /** not for user call */
    @Override
    public void createShaderAndBuffer() {
        compileShader();
        prepareBuffer();
    }

    /** not for user call */
    @Override
    public void destroyShaderAndBuffer() {
        GLES20.glDeleteShader(mVertexShader);
        GLES20.glDeleteShader(mFragmentShader);
        GLES20.glDeleteProgram(mProgram);
        /* Notice the direct byte buffer not be freed! */
    }

    /** not for user call */
    @Override
    public void drawFrame(int textureId) {
        GLES20.glViewport(0, 0, mFrameWidth, mFrameHeight);
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        GLES20.glUseProgram(mProgram);

        int positionHandler = GLES20.glGetAttribLocation(mProgram, "aPosition");
        int textureCoordHandler = GLES20.glGetAttribLocation(mProgram, "aTextureCoord");
        int textureHandler = GLES20.glGetUniformLocation(mProgram, "sTexture");

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);

        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        GLES20.glEnableVertexAttribArray(positionHandler);
        GLES20.glVertexAttribPointer(positionHandler, VERTEX_COMPONENT,
                GLES20.GL_FLOAT, false, VERTEX_COMPONENT * Float.SIZE / Byte.SIZE, mVertexBuffer);

        GLES20.glEnableVertexAttribArray(textureCoordHandler);
        GLES20.glVertexAttribPointer(textureCoordHandler, TEXTURE_COMPONENT,
                GLES20.GL_FLOAT, false, TEXTURE_COMPONENT * Float.SIZE / Byte.SIZE, mTextureBuffer);

        GLES20.glUniform1i(textureHandler, 0);

        GLES20.glDrawElements(GLES20.GL_TRIANGLES, drawOrder.length, GLES20.GL_UNSIGNED_SHORT, mDrawListBuffer);

        GLES20.glDisableVertexAttribArray(positionHandler);
        GLES20.glDisableVertexAttribArray(textureCoordHandler);

        if (mFrameRgbaDataCallback != null) {
            GLES20.glReadPixels(0, 0, mFrameWidth, mFrameHeight,
                    GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, mPixelByteBuffer);
            mFrameRgbaDataCallback.onFrameRgbaData(mPixelByteBuffer);
            mPixelByteBuffer.clear();
        }
    }

    private void compileShader() {
        if (mVertexShaderSourceCode == null || mFragmentShaderSourceCode == null) {
            throw new IllegalArgumentException("please call CameraView#setShaderSourceCode(String,String) first");
        }
        mVertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        mFragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        GLES20.glShaderSource(mVertexShader, mVertexShaderSourceCode);
        GLES20.glShaderSource(mFragmentShader, mFragmentShaderSourceCode);
        final int[] compileStatus = new int[1];
        GLES20.glCompileShader(mVertexShader);
        GLES20.glGetShaderiv(mVertexShader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);
        if (compileStatus[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("vertex shader compile failed:"
                    + GLES20.glGetShaderInfoLog(mVertexShader));
        }
        GLES20.glCompileShader(mFragmentShader);
        GLES20.glGetShaderiv(mFragmentShader, GLES20.GL_COMPILE_STATUS, compileStatus, 0);
        if (compileStatus[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("fragment shader compile failed:"
                    + GLES20.glGetShaderInfoLog(mFragmentShader));
        }
        mProgram = GLES20.glCreateProgram();
        GLES20.glAttachShader(mProgram, mVertexShader);
        GLES20.glAttachShader(mProgram, mFragmentShader);
        GLES20.glLinkProgram(mProgram);
    }

    private void prepareBuffer() {
        /* Vertex buffer */
        ByteBuffer bb;
        bb = ByteBuffer.allocateDirect(Float.SIZE / Byte.SIZE * vertexCoords.length);
        bb.order(ByteOrder.nativeOrder());
        mVertexBuffer = bb.asFloatBuffer();
        mVertexBuffer.put(vertexCoords);
        mVertexBuffer.position(0);

		/* Texture buffer */
        final float[] textureCoords = mIsFrontCamera ? frontTextureCoords : backTextureCoords;
        bb = ByteBuffer.allocateDirect(Float.SIZE / Byte.SIZE * textureCoords.length);
        bb.order(ByteOrder.nativeOrder());
        mTextureBuffer = bb.asFloatBuffer();
        mTextureBuffer.put(textureCoords);
        mTextureBuffer.position(0);

		/* Draw list buffer */
        bb = ByteBuffer.allocateDirect(Short.SIZE / Byte.SIZE * drawOrder.length);
        bb.order(ByteOrder.nativeOrder());
        mDrawListBuffer = bb.asShortBuffer();
        mDrawListBuffer.put(drawOrder);
        mDrawListBuffer.position(0);

        /* Pixel buffer */
        if (mFrameRgbaDataCallback != null) {
            bb = ByteBuffer.allocateDirect(4 /* RGBA */ * mFrameWidth * mFrameHeight);
            bb.order(ByteOrder.nativeOrder());
            mPixelByteBuffer = bb;
            mPixelByteBuffer.position(0);
        }
    }

    public interface OnFrameRgbaDataCallback {
        /** this method should be processed quickly to make sure extract frame smoothly */
        void onFrameRgbaData(ByteBuffer rgba);
    }

    public static class FrameCallbackThread extends Thread implements OnFrameRgbaDataCallback {
        private final OnFrameRgbaDataCallback callback;
        private final ByteBuffer buffer;
        private volatile boolean loop;
        private volatile boolean busy;

        public FrameCallbackThread(OnFrameRgbaDataCallback callback, int capacity) {
            this.callback = callback;
            this.buffer = ByteBuffer.allocateDirect(capacity);
            this.buffer.order(ByteOrder.nativeOrder());
            this.buffer.clear();
            this.loop = true;
            this.busy = false;
        }

        public void quit() {
            this.loop = false;
            this.interrupt();
        }

        @Override
        public void run() {
            try {
                while (loop) {
                    while (!busy) {
                        synchronized(this) {
                            wait();
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
                e.printStackTrace();
            }
        }

        @Override
        public void onFrameRgbaData(ByteBuffer rgba) {
            if (!busy) {
                buffer.put(rgba);
                rgba.rewind();
                busy = true;
                synchronized(this) {
                    notifyAll();
                }
            }
        }
    }
}
