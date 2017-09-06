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
    private static final float[] textureCoords = {
            0.0f, 0.0f, // top left
            0.0f, 1.0f, // bottom left
            1.0f, 1.0f, // bottom right
            1.0f, 0.0f, // top right
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
    /* if change shader source code, search replace_target in this file */
    private static final String defaultFragmentShaderSource = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  vec2 vTexture = vTextureCoord;\n"
            + "  gl_FragColor = texture2D(sTexture, vTexture);\n"
            + "}";
    private static final String defaultFragmentShaderSourceX = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  vec2 vTexture = vTextureCoord;\n"
            + "  vTexture.x = 1.0 - vTexture.x;\n"
            + "  gl_FragColor = texture2D(sTexture, vTexture);\n"
            + "}";
    private static final String defaultFragmentShaderSourceY = ""
            + "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "varying vec2 vTextureCoord;\n"
            + "uniform samplerExternalOES sTexture;\n"
            + "void main() {\n"
            + "  vec2 vTexture = vTextureCoord;\n"
            + "  vTexture.y = 1.0 - vTexture.y;\n"
            + "  gl_FragColor = texture2D(sTexture, vTexture);\n"
            + "}";

    public static void upsideDown(ByteBuffer pixelByteBuffer, int width, int height, int channels) {
        if (pixelByteBuffer != null) {
            final ByteBuffer pixels = pixelByteBuffer.duplicate();
            final ByteBuffer temp = pixels.duplicate();
            final int row_size = width * channels;
            final byte[] temp_row_pixels = new byte[row_size];
            int i = 0, j = height - 1;
            while(i < j) {
                pixels.position(j * row_size);
                pixels.get(temp_row_pixels, 0, row_size);

                temp.position(i * row_size);
                temp.limit(temp.position() + row_size);
                pixels.position(j * row_size);
                pixels.put(temp);

                pixels.position(i * row_size);
                pixels.put(temp_row_pixels, 0, row_size);

                ++i;
                --j;
            }
        }
    }

    private int mNormalProgram;
    private int mFilterProgram;
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

    /** not for user call */
    @Override
    public final long getNativeContextPointer() {
        return 0L;
    }

    /** not for user call */
    @Override
    public void createShaderAndBuffer() {
        prepareShader(true);
        if (getFragmentShaderSourceCode() != null) {
            prepareShader(false);
        }
        prepareBuffer();
    }

    /** not for user call */
    @Override
    public void destroyShaderAndBuffer() {
        if (mNormalProgram != 0) {
            GLES20.glDeleteProgram(mNormalProgram);
        }
        if (mFilterProgram != 0) {
            GLES20.glDeleteProgram(mFilterProgram);
        }
        mNormalProgram = 0;
        mFilterProgram = 0;
        /* Notice the direct byte buffer not be freed! */
    }

    /** not for user call */
    @Override
    public void drawFrame(int textureId) {
        if (mNormalProgram != 0) {
            realDrawFrame(true, textureId);
        }
        if (mFilterProgram != 0) {
            realDrawFrame(false, textureId);
        }
    }

    private void realDrawFrame(boolean normal, int textureId) {
        GLES20.glViewport(0, 0, getFrameWidth(), getFrameHeight());
        GLES20.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

        final int program;
        if (normal) {
            program = mNormalProgram;
        } else {
            program = mFilterProgram;
        }
        GLES20.glUseProgram(program);

        int positionHandler = GLES20.glGetAttribLocation(program, "aPosition");
        int textureCoordHandler = GLES20.glGetAttribLocation(program, "aTextureCoord");
        int textureHandler = GLES20.glGetUniformLocation(program, "sTexture");

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

        final OnFrameRgbaDataCallback callback = getOnFrameRgbaDataCallback();
        if (callback != null) {
            final int w = getFrameWidth(), h = getFrameHeight();
            GLES20.glReadPixels(0, 0, w, h, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, mPixelByteBuffer);
            upsideDown(mPixelByteBuffer, w, h, 4 /* RGBA */);
            callback.onFrameRgbaData(mPixelByteBuffer, normal);
            mPixelByteBuffer.clear();
        }
    }

    private void prepareShader(boolean normal) {
        System.out.println("prepareShader(normal=" + normal + ")");
        final int vertexShader = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        if (vertexShader == 0) {
            throw new IllegalStateException("vertex shader create failed:"
                    + GLES20.glGetShaderInfoLog(vertexShader));
        }
        final int fragmentShader = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        if (fragmentShader == 0) {
            throw new IllegalStateException("fragment shader create failed:"
                    + GLES20.glGetShaderInfoLog(vertexShader));
        }
        final String vertexShaderSource;
        final String fragmentShaderSource;
        if (normal) {
            vertexShaderSource = defaultVertexShaderSource;
            switch (getFragmentShaderType()) {
                case FRAGMENT_SHADER_TYPE_REVERSE_X:
                    fragmentShaderSource = defaultFragmentShaderSourceX;
                    break;
                case FRAGMENT_SHADER_TYPE_REVERSE_Y:
                    fragmentShaderSource = defaultFragmentShaderSourceY;
                    break;
                default:
                    fragmentShaderSource = defaultFragmentShaderSource;
                    break;
            }
        } else {
            vertexShaderSource = defaultVertexShaderSource;
            final String replace_target = "  gl_FragColor = texture2D(sTexture, vTexture);\n";
            switch (getFragmentShaderType()) {
                case FRAGMENT_SHADER_TYPE_REVERSE_X: {
                    fragmentShaderSource = defaultFragmentShaderSourceX
                            .replace(replace_target, getFragmentShaderSourceCode());
                }
                    break;
                case FRAGMENT_SHADER_TYPE_REVERSE_Y: {
                    fragmentShaderSource = defaultFragmentShaderSourceY
                            .replace(replace_target, getFragmentShaderSourceCode());
                }
                    break;
                default: {
                    fragmentShaderSource = defaultFragmentShaderSource
                            .replace(replace_target, getFragmentShaderSourceCode());
                }
                    break;
            }
        }
        GLES20.glShaderSource(vertexShader, vertexShaderSource);
        GLES20.glShaderSource(fragmentShader, fragmentShaderSource);
        final int[] status = new int[1];
        GLES20.glCompileShader(vertexShader);
        GLES20.glGetShaderiv(vertexShader, GLES20.GL_COMPILE_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("vertex shader compile failed:"
                    + GLES20.glGetShaderInfoLog(vertexShader));
        }
        GLES20.glCompileShader(fragmentShader);
        GLES20.glGetShaderiv(fragmentShader, GLES20.GL_COMPILE_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("fragment shader compile failed:"
                    + GLES20.glGetShaderInfoLog(fragmentShader));
        }
        final int program = GLES20.glCreateProgram();
        if (program == 0) {
            throw new IllegalStateException("program create failed:"
                    + GLES20.glGetProgramInfoLog(vertexShader));
        }
        GLES20.glAttachShader(program, vertexShader);
        GLES20.glAttachShader(program, fragmentShader);
        GLES20.glLinkProgram(program);
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, status, 0);
        if (status[0] != GLES20.GL_TRUE) {
            throw new IllegalStateException("program link failed:"
                    + GLES20.glGetProgramInfoLog(vertexShader));
        }
        GLES20.glDeleteShader(vertexShader);
        GLES20.glDeleteShader(fragmentShader);
        if (normal) {
            mNormalProgram = program;
        } else {
            mFilterProgram = program;
        }
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
        if (getOnFrameRgbaDataCallback() != null) {
            bb = ByteBuffer.allocateDirect(4 /* RGBA */ * getFrameWidth() * getFrameHeight());
            bb.order(ByteOrder.nativeOrder());
            mPixelByteBuffer = bb;
            mPixelByteBuffer.position(0);
        }
    }
}
