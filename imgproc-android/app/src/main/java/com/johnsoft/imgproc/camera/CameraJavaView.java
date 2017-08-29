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
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.support.annotation.Keep;
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
@Keep
public class CameraJavaView extends CameraView {
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

    private int mProgram;
    private int mVertexShader;
    private int mFragmentShader;
    private String mVertexShaderSourceCode = defaultVertexShaderSource;
    private String mFragmentShaderSourceCode = defaultFragmentShaderSource;
    private FloatBuffer mVertexBuffer;
    private FloatBuffer mTextureBuffer;
    private ShortBuffer mDrawListBuffer;
    private ByteBuffer mPixelByteBuffer;

    public CameraJavaView(Context context) {
        super(context);
    }

    public CameraJavaView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CameraJavaView setShaderSourceCode(String vertexSource, String fragmentSource) {
        if (vertexSource != null) {
            this.mVertexShaderSourceCode = vertexSource;
        }
        if (fragmentSource != null) {
            this.mFragmentShaderSourceCode = fragmentSource;
        }
        return this;
    }

    /** not for user call */
    @Override
    public final long getNativeContextPointer() {
        return 0L;
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
        GLES20.glViewport(0, 0, getFrameWidth(), getFrameHeight());
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
            GLES20.glReadPixels(0, 0, getFrameWidth(), getFrameHeight(),
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
        final float[] textureCoords = isFrontCamera() ? frontTextureCoords : backTextureCoords;
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
            bb = ByteBuffer.allocateDirect(4 /* RGBA */ * getFrameWidth() * getFrameHeight());
            bb.order(ByteOrder.nativeOrder());
            mPixelByteBuffer = bb;
            mPixelByteBuffer.position(0);
        }
    }
}
