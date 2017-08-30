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

import com.johnsoft.imgproc.camera.CameraManager;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.SurfaceTexture;
import android.hardware.SensorManager;
import android.support.annotation.IntDef;
import android.support.annotation.Keep;
import android.util.AttributeSet;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.TextureView;

/**
 * Simple Camera View
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
@Keep
public class SimpleCameraView extends TextureView implements TextureView.SurfaceTextureListener {
    static {
        System.loadLibrary("simple_camera_view");
    }

    @Keep
    public interface FrameDataCallback {
        void onCreate(int width, int height);
        void onNativeFrameData(byte[] bytes);
        void onDestroy();
    }

    public static final int FRAG_SHADER_TYPE_NORMAL = 0;
    public static final int FRAG_SHADER_TYPE_REVERSE_X = -1;
    public static final int FRAG_SHADER_TYPE_REVERSE_Y = 1;

    @IntDef({FRAG_SHADER_TYPE_NORMAL, FRAG_SHADER_TYPE_REVERSE_X, FRAG_SHADER_TYPE_REVERSE_Y})
    public @interface FragShaderTypeAnno {
    }

    private boolean isInit;
    private long nativeContextPointer;
    private FrameDataCallback nativeCallback;
    private Surface nativeSurface;
    private RenderThread renderThread;
    private OrientationEventListener orientationEventListener;
    @FragShaderTypeAnno
    private int fragShaderType = FRAG_SHADER_TYPE_NORMAL;
    private int orientationType = Configuration.ORIENTATION_UNDEFINED;

    public SimpleCameraView(Context context) {
        super(context);
        setSurfaceTextureListener(this);
    }

    public SimpleCameraView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSurfaceTextureListener(this);
    }

    public void setFrameDataCallback(FrameDataCallback callback) {
        this.nativeCallback = callback;
    }

    /**
     * {@link #FRAG_SHADER_TYPE_NORMAL}, {@link #FRAG_SHADER_TYPE_REVERSE_X}, {@link #FRAG_SHADER_TYPE_REVERSE_Y} */
    public void setFragShaderType(@FragShaderTypeAnno int fragShaderType) {
        this.fragShaderType = fragShaderType;
    }

    public void resume() {
        if (isInit) {
            SimpleCameraManager.singleInstance.startPreview(renderThread);
        }
        if (orientationEventListener == null) {
            orientationEventListener = new OrientationEventListener(getContext(), SensorManager.SENSOR_DELAY_NORMAL) {
                @Override
                public void onOrientationChanged(int orientation) {
                    if (orientation == ORIENTATION_UNKNOWN) {
                        return;
                    }
                    final int gap = 30;
                    if ((orientation > 0 && orientation < gap)
                            || (orientation > (360 - gap) && orientation < 360)
                            || (orientation > (180 - gap) && orientation < (180 + gap))) {
                        if (orientationType != Configuration.ORIENTATION_PORTRAIT) {
                            orientationType = Configuration.ORIENTATION_PORTRAIT;
                            System.out.println("SOMMNSON portrait");
                        }
                    } else if ((orientation > (90 - gap) && orientation < (90 + gap))
                            || (orientation > (270 - gap) && orientation < (270 + gap))) {
                        if (orientationType != Configuration.ORIENTATION_LANDSCAPE) {
                            orientationType = Configuration.ORIENTATION_LANDSCAPE;
                            System.out.println("SOMMNSON landscape");
                        }
                    }
                }
            };
        }
        if (orientationEventListener.canDetectOrientation()) {
            System.out.println("SOMMNSON Can detect orientation");
            orientationEventListener.enable();
        } else {
            System.out.println("SOMMNSON Cannot detect orientation");
            orientationEventListener.disable();
        }
    }

    public void pause() {
        if (isInit) {
            SimpleCameraManager.singleInstance.stopPreview();
        }
        if (orientationEventListener != null) {
            orientationEventListener.disable();
        }
    }

    private native void nativeInit(FrameDataCallback callback, int width, int height,
                                   Surface surface, int fragShaderType);
    private native void nativeRelease();

    private native void createEgl();
    private native void swapBuffers();
    private native void destroyEgl();

    private native void createShader();
    private native void drawFrame();
    private native void destroyShader();

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surfaceTexture, int width, int height) {
        System.out.println("#onSurfaceTextureAvailable");
        if (!isInit) {
            nativeSurface = new Surface(surfaceTexture);
            nativeInit(nativeCallback, width, height, nativeSurface, fragShaderType);
            final CameraManager.DefaultPreviewSizeChooser chooser = new CameraManager
                    .DefaultPreviewSizeChooser(width, height);
            SimpleCameraManager.singleInstance.open(chooser);
            renderThread = new RenderThread(this);
            renderThread.start();
            SimpleCameraManager.singleInstance.startPreview(renderThread);
            if (nativeCallback != null) {
                try {
                    nativeCallback.onCreate(width, height);
                } catch (Throwable thr) {
                    thr.printStackTrace();
                }
            }
            isInit = true;
        }
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        System.out.println("#onSurfaceTextureDestroyed");
        if (isInit) {
            if (nativeCallback != null) {
                try {
                    nativeCallback.onDestroy();
                } catch (Throwable thr) {
                    thr.printStackTrace();
                }
            }
            renderThread.quit();
            SimpleCameraManager.singleInstance.close();
            nativeSurface.release();
            nativeRelease();
            isInit = false;
        }
        return true;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surfaceTexture, int width, int height) {
        System.out.println("#onSurfaceTextureSizeChanged");
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
    }

    @Keep
    public static class RenderThread extends Thread implements SurfaceTexture.OnFrameAvailableListener {
        private final SimpleCameraView client;
        private volatile boolean loop;
        private volatile boolean available;

        public RenderThread(SimpleCameraView client) {
            this.client = client;
            this.loop = true;
            this.available = false;
        }

        public void quit() {
            this.loop = false;
            this.interrupt();
        }

        @Override
        public void run() {
            try {
                client.createEgl();
                client.createShader();
                final SurfaceTexture texture = SimpleCameraManager.singleInstance.getSurfaceTexture();
                while (loop) {
                    try {
                        texture.attachToGLContext(0);
                        texture.updateTexImage();
                        client.drawFrame();
                        texture.detachFromGLContext();
                        client.swapBuffers();
                        synchronized (this) {
                            while (!available) {
                                wait(1000L);
                            }
                        }
                        available = false;
                    } catch (InterruptedException e) {
                        break;
                    }
                }
            } catch (Throwable thr) {
                thr.printStackTrace();
            } finally {
                client.destroyShader();
                client.destroyEgl();
            }
        }

        @Override
        public void onFrameAvailable(SurfaceTexture surfaceTexture) {
            available = true;
            synchronized (this) {
                notifyAll();
            }
        }
    }

    @Keep
    public static abstract class TransferThread extends Thread
            implements FrameDataCallback {
        private final byte[] buffer;
        private volatile boolean loop;
        private volatile boolean busy;

        public TransferThread(int capacity) {
            this.buffer = new byte[capacity];
            this.loop = true;
            this.busy = false;
        }

        public void quit() {
            this.loop = false;
            interrupt();
        }

        @Override
        public void run() {
            try {
                while (loop) {
                    while (!busy) {
                        synchronized (this) {
                            wait(1000L);
                        }
                    }
                    try {
                        onFrameData(buffer);
                    } catch (Throwable thr) {
                        thr.printStackTrace();
                    }
                    busy = false;
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                System.out.println("FrameCallbackThread quit!");
            }
        }

        @Override
        public void onNativeFrameData(byte[] rgba) {
            if (!busy) {
                System.arraycopy(rgba, 0, buffer, 0, rgba.length);
                busy = true;
                synchronized (this) {
                    notifyAll();
                }
            }
        }

        public abstract void onFrameData(byte[] rgba);
    }
}