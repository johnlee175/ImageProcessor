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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.EGL14;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.SparseArray;

/**
 * Camera Manager
 * @author John Kenrinus Lee
 * @version 2017-08-14
 */
public enum CameraManager {
    singleInstance;

    public static class CameraManageException extends RuntimeException {
        public CameraManageException(String message) {
            super(message);
        }
        public CameraManageException(Throwable throwable) {
            super(throwable);
        }
        public CameraManageException(String message, Throwable throwable) {
            super(message, throwable);
        }
    }

    public interface PreviewSizeChooser {
        Camera.Size chooseSize(List<Camera.Size> sizes);
    }

    public static class DefaultPreviewSizeChooser implements PreviewSizeChooser {
        private int widthHint;
        private int heightHint;

        public DefaultPreviewSizeChooser(int widthHint, int heightHint) {
            this.widthHint = widthHint;
            this.heightHint = heightHint;
        }

        @Override
        public Camera.Size chooseSize(List<Camera.Size> sizes) {
            final double baseRatio = widthHint / (double) (widthHint + heightHint);
            final List<Double> list = new ArrayList<>();
            double localSize, localRatio;
            for (final Camera.Size size : sizes) {
                final int sizeWidth = size.width;
                final int sizeHeight = size.height;
                localSize = Math.sqrt(Math.pow(widthHint - sizeWidth, 2) + Math.pow(heightHint - sizeHeight, 2)) + 1;
                localRatio = Math.abs(baseRatio - sizeWidth / (double) (sizeWidth + sizeHeight)) + 1;
                list.add(localSize * localRatio * localRatio);
            }
            double minValue = Double.MAX_VALUE, localValue;
            int index = 0;
            final int size = list.size();
            for (int i = 0; i < size; ++i) {
                localValue = list.get(i);
                if (localValue < minValue) {
                    minValue = localValue;
                    index = i;
                }
            }
            return sizes.get(index);
        }
    }

    public static class CachedPreviewSizeChooser extends DefaultPreviewSizeChooser {
        private Camera.Size size;
        private boolean cached;

        public CachedPreviewSizeChooser(int widthHint, int heightHint) {
            super(widthHint, heightHint);
        }

        public int getFrameWidth() {
            if (size == null) {
                return 0;
            }
            return size.width;
        }

        public int getFrameHeight() {
            if (size == null) {
                return 0;
            }
            return size.height;
        }

        @Override
        public Camera.Size chooseSize(List<Camera.Size> sizes) {
            if (cached) {
                return size;
            } else {
                return size = super.chooseSize(sizes);
            }
        }
    }

    public interface GLTextureViewClient {
        SurfaceTexture getSurfaceTexture();
        void createShaderAndBuffer();
        void drawFrame(int textureId);
        void destroyShaderAndBuffer();
        void onError(Throwable thr);
        boolean isPaused();
    }

    public interface SurfaceTextureHolder {
        void drawClientFrame(GLTextureViewClient client);
    }

    public static class GLClientRenderThread extends Thread {
        private final GLTextureViewClient client;
        private final SurfaceTextureHolder surfaceTextureHolder;
        private volatile boolean loop;
        private EGL10 mEgl;
        private EGLDisplay mEglDisplay;
        private EGLConfig mEglConfig;
        private EGLContext mEglContext;
        private EGLSurface mEglSurface;

        public GLClientRenderThread(GLTextureViewClient client, SurfaceTextureHolder surfaceTextureHolder) {
            this.client = client;
            this.surfaceTextureHolder = surfaceTextureHolder;
            this.loop = true;
        }

        public void quit() {
            this.loop = false;
            this.interrupt();
            try {
                this.join(1000L);
            } catch (InterruptedException e) {
                /* ignored */
            }
        }

        @Override
        public synchronized void run() {
            try {
                initEGL(client.getSurfaceTexture());
                client.createShaderAndBuffer();
                while (loop) {
                    try {
                        while (client.isPaused()) {
                            Thread.sleep(1000L);
                        }
                        if (surfaceTextureHolder != null) {
                            surfaceTextureHolder.drawClientFrame(client);
                        }
                        if (!mEgl.eglSwapBuffers(mEglDisplay, mEglSurface)) {
                            throw new CameraManageException("Can not swap buffers");
                        }
                        wait();
                    } catch (InterruptedException e) {
                        break;
                    }
                }
            } catch (Throwable thr) {
                client.onError(thr);
            } finally {
                client.destroyShaderAndBuffer();
                releaseEGL(client.getSurfaceTexture());
            }
        }

        public synchronized void doNotify() {
            notifyAll();
        }

        protected void initEGL(SurfaceTexture surfaceTexture) {
            mEgl = (EGL10) EGLContext.getEGL();
            if ((mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY)) == EGL10.EGL_NO_DISPLAY) {
                throw new CameraManageException("eglGetDisplay failed:" + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
            if (!mEgl.eglInitialize(mEglDisplay, new int[2])) {
                throw new CameraManageException("eglInitialize failed:" + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }

            final int configsCount[] = new int[1];
            final EGLConfig configs[] = new EGLConfig[1];
            final int configSpec[] = new int[] {
                    EGL10.EGL_COLOR_BUFFER_TYPE, EGL10.EGL_RGB_BUFFER,
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_ALPHA_SIZE, 8,
                    EGL10.EGL_LUMINANCE_SIZE, 0,
                    EGL10.EGL_DEPTH_SIZE, 0,
                    EGL10.EGL_STENCIL_SIZE, 0,
                    EGL10.EGL_ALPHA_MASK_SIZE, 0,
                    EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
                    EGL10.EGL_RENDERABLE_TYPE, EGL14.EGL_OPENGL_ES2_BIT,
                    EGL10.EGL_NONE
            };
            mEgl.eglChooseConfig(mEglDisplay, configSpec, configs, 1, configsCount);
            if (configsCount[0] <= 0) {
                throw new CameraManageException("eglChooseConfig failed:" + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
            mEglConfig = configs[0];
            int contextSpec[] = new int[] {
                    EGL14.EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL10.EGL_NONE
            };
            if ((mEglContext = mEgl.eglCreateContext(mEglDisplay, mEglConfig,
                    EGL10.EGL_NO_CONTEXT, contextSpec)) == EGL10.EGL_NO_CONTEXT) {
                throw new CameraManageException("eglCreateContext failed: " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
            if ((mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, mEglConfig, surfaceTexture,
                    null)) == EGL10.EGL_NO_SURFACE) {
                throw new CameraManageException("eglCreateWindowSurface failed"+GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
            if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
                throw new CameraManageException("eglMakeCurrent failed:"+GLUtils.getEGLErrorString(mEgl.eglGetError()));
            }
        }

        protected void releaseEGL(SurfaceTexture surfaceTexture) {
            mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
            mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
            mEgl.eglDestroyContext(mEglDisplay, mEglContext);
            mEgl.eglTerminate(mEglDisplay);
            mEglDisplay = EGL10.EGL_NO_DISPLAY;
            mEglContext = EGL10.EGL_NO_CONTEXT;
            mEglSurface = EGL10.EGL_NO_SURFACE;
            mEgl = null;
        }
    }

    public static class GLRenderClientManager implements SurfaceTexture.OnFrameAvailableListener {
        private final Map<GLTextureViewClient, GLClientRenderThread> clientThreadMap = new HashMap<>();
        private SurfaceTextureHolder surfaceTextureHolder;

        public void setSurfaceTextureHolder(SurfaceTextureHolder surfaceTextureHolder) {
            this.surfaceTextureHolder = surfaceTextureHolder;
        }

        public synchronized void addGLTextureViewClient(GLTextureViewClient client) {
            GLClientRenderThread thread = new GLClientRenderThread(client, surfaceTextureHolder);
            thread.start();
            clientThreadMap.put(client, thread);
        }

        public synchronized void removeGLTextureViewClient(GLTextureViewClient client) {
            clientThreadMap.remove(client).quit();
        }

        public synchronized void clean() {
            ArrayList<GLTextureViewClient> temp = new ArrayList<>();
            for (GLTextureViewClient client : clientThreadMap.keySet()) {
                temp.add(client);
            }
            for (GLTextureViewClient client : temp) {
                clientThreadMap.remove(client).quit();
            }
        }

        @Override
        public synchronized void onFrameAvailable(SurfaceTexture surfaceTexture) {
            for (GLTextureViewClient client : clientThreadMap.keySet()) {
                clientThreadMap.get(client).doNotify();
            }
        }
    }

    public static class CameraInterface implements SurfaceTextureHolder {
        private static final ScheduledExecutorService singleScheduledExecutor = Executors.newSingleThreadScheduledExecutor();

        private final int cameraIndex;
        private final Point frameSize;
        private final Camera camera;
        private final GLRenderClientManager clientManager;
        private final SurfaceTexture cameraPreviewSurfaceTexture;
        private final int cameraPreviewTextureId;
        private ScheduledFuture<?> scheduledFuture;

        public CameraInterface(int cameraIndex, Point frameSize, Camera camera,
                               GLRenderClientManager clientManager) {
            this.cameraIndex = cameraIndex;
            this.frameSize = frameSize;
            this.camera = camera;
            this.clientManager = clientManager;
            int id;
            try {
                final int textures[] = new int[1];
                GLES20.glGenTextures(1, textures, 0);
                id = textures[0];
            } catch (Throwable thr) {
                id = cameraIndex;
            }
            this.cameraPreviewTextureId = id;
            this.cameraPreviewSurfaceTexture = new SurfaceTexture(cameraPreviewTextureId);
        }

        public void startPreview() throws Exception {
            clientManager.setSurfaceTextureHolder(this);
            camera.setPreviewTexture(cameraPreviewSurfaceTexture);
            camera.startPreview();
            cameraPreviewSurfaceTexture.setOnFrameAvailableListener(clientManager);
            try {
                cameraPreviewSurfaceTexture.detachFromGLContext();
            } catch (Throwable thr) {
                /* ignored */
            }
            scheduledFuture = singleScheduledExecutor.scheduleWithFixedDelay(new Runnable() {
                @Override
                public void run() {
                    camera.autoFocus(null);
                }
            }, 4000L, 2000L, TimeUnit.MILLISECONDS);
        }

        public void stopPreview() throws Exception {
            if (scheduledFuture != null) {
                scheduledFuture.cancel(true);
            }
            scheduledFuture = null;
            camera.stopPreview();
            cameraPreviewSurfaceTexture.setOnFrameAvailableListener(null);
        }

        public void close() throws Exception {
            cameraPreviewSurfaceTexture.release();
            camera.release();
            clientManager.clean();
        }

        public void drawClientFrame(GLTextureViewClient client) {
            try {
                synchronized(cameraPreviewSurfaceTexture) {
                    cameraPreviewSurfaceTexture.attachToGLContext(cameraPreviewTextureId);
                    cameraPreviewSurfaceTexture.updateTexImage();
                    client.drawFrame(cameraPreviewTextureId);
                    cameraPreviewSurfaceTexture.detachFromGLContext();
                }
            } catch (Throwable e) {
                throw new CameraManageException(e);
            }
        }

        public int getCameraIndex() {
            return cameraIndex;
        }

        public Point getFrameSize() {
            return frameSize;
        }
    }

    public static class ConditionController {
        public final boolean useStateCheck;
        public final boolean useReferenceCount;
        public volatile int state;
        public volatile int openCount;
        public volatile int previewCount;

        public ConditionController(boolean useStateCheck, boolean useReferenceCount) {
            this.useStateCheck = useStateCheck;
            this.useReferenceCount = useReferenceCount;
        }

        public boolean openBegin() {
            if (useReferenceCount) {
                ++openCount;
                System.out.println("+open count: " + openCount);
            }
            if (useStateCheck) {
                return state == STATE_CLOSED;
            }
            return true;
        }

        public void openEnd() {
            if (useStateCheck) {
                state = STATE_OPEN;
            }
        }

        public boolean startPreviewBegin() {
            if (useReferenceCount) {
                ++previewCount;
                System.out.println("+preview count: " + previewCount);
            }
            if (useStateCheck) {
                return state == STATE_OPEN;
            }
            return true;
        }

        public void startPreviewEnd() {
            if (useStateCheck) {
                state = STATE_PREVIEW;
            }
        }

        public boolean stopPreviewBegin() {
            if (useReferenceCount && (previewCount == 0 || --previewCount != 0)) {
                System.out.println("-preview count: " + previewCount);
                return false;
            }
            if (useStateCheck) {
                return state == STATE_PREVIEW;
            }
            return true;
        }

        public void stopPreviewEnd() {
            if (useStateCheck) {
                state = STATE_OPEN;
            }
        }

        public boolean closeBegin() {
            if (useReferenceCount && (openCount == 0 || --openCount != 0)) {
                System.out.println("-open count: " + openCount);
                return false;
            }
            if (useStateCheck) {
                return state == STATE_OPEN || state == STATE_PREVIEW;
            }
            return true;
        }

        public void closeEnd() {
            if (useStateCheck) {
                state = STATE_CLOSED;
            }
        }
    }

    public static int cameraIndex(boolean front) {
        int cameraIndex = -1;
        final int cameraFacingIndex = front ? Camera.CameraInfo.CAMERA_FACING_FRONT : Camera.CameraInfo.CAMERA_FACING_BACK;
        final Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
        for (int camIdx = 0; camIdx < Camera.getNumberOfCameras(); ++camIdx) {
            Camera.getCameraInfo(camIdx, cameraInfo);
            if (cameraInfo.facing == cameraFacingIndex) {
                cameraIndex = camIdx;
                break;
            }
        }
        return cameraIndex;
    }

    public static final int STATE_UNKNOWN = -1;
    public static final int STATE_CLOSED = 0;
    public static final int STATE_OPEN = 1;
    public static final int STATE_PREVIEW = 2;

    private final SparseArray<CameraInterface> map = new SparseArray<>();
    private final SparseArray<ConditionController> flagMap = new SparseArray<>();

    public synchronized void flag(int cameraIndex, boolean useStateCheck, boolean useReferenceCount,
                                  boolean forceRecover) {
        if (forceRecover) {
            flagMap.remove(cameraIndex);
            flagMap.put(cameraIndex, new ConditionController(useStateCheck, useReferenceCount));
        } else {
            final ConditionController newController = new ConditionController(useStateCheck, useReferenceCount);
            final ConditionController oldController = flagMap.get(cameraIndex);
            if (oldController != null) {
                newController.state = oldController.state;
                newController.openCount = oldController.openCount;
                newController.previewCount = oldController.previewCount;
            }
            flagMap.put(cameraIndex, newController);
        }
    }

    public synchronized void open(int cameraIndex, PreviewSizeChooser chooser) {
        try {
            final ConditionController controller = flagMap.get(cameraIndex);
            if (controller != null && !controller.openBegin()) {
                return;
            }
            if (map.get(cameraIndex) == null) {
                final Camera currentCamera = Camera.open(cameraIndex);
                final Camera.Parameters params = currentCamera.getParameters();
                final Camera.Size frameSize = chooser.chooseSize(params.getSupportedPreviewSizes());
                params.setPreviewSize(frameSize.width, frameSize.height);
                final List<String> focusModes = params.getSupportedFocusModes();
                if(focusModes.contains("continuous-video")){
                    params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
                }
                currentCamera.setParameters(params);
                map.put(cameraIndex, new CameraInterface(cameraIndex, new Point(frameSize.width, frameSize.height),
                        currentCamera, new GLRenderClientManager()));
            }
            if (controller != null) {
                controller.openEnd();
            }
        } catch (Throwable e) {
            throw new CameraManageException("Camera #" + cameraIndex + " failed to open!",  e);
        }
    }

    public synchronized void startPreview(int cameraIndex) {
        try {
            final ConditionController controller = flagMap.get(cameraIndex);
            if (controller != null && !controller.startPreviewBegin()) {
                return;
            }
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null) {
                cameraInterface.startPreview();
                if (controller != null) {
                    controller.startPreviewEnd();
                }
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized void stopPreview(int cameraIndex) {
        try {
            final ConditionController controller = flagMap.get(cameraIndex);
            if (controller != null && !controller.stopPreviewBegin()) {
                return;
            }
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null) {
                cameraInterface.stopPreview();
                if (controller != null) {
                    controller.stopPreviewEnd();
                }
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized void close(int cameraIndex) {
        try {
            final ConditionController controller = flagMap.get(cameraIndex);
            if (controller != null && !controller.closeBegin()) {
                return;
            }
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null) {
                map.remove(cameraIndex);
                cameraInterface.close();
                if (controller != null) {
                    controller.closeEnd();
                }
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized void attachGLTextureViewClient(int cameraIndex, GLTextureViewClient client) {
        try {
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null && cameraInterface.clientManager != null) {
                cameraInterface.clientManager.addGLTextureViewClient(client);
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized void dettachGLTextureViewClient(int cameraIndex, GLTextureViewClient client) {
        try {
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null && cameraInterface.clientManager != null) {
                cameraInterface.clientManager.removeGLTextureViewClient(client);
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized Point getFrameSize(int cameraIndex) {
        try {
            final CameraInterface cameraInterface = map.get(cameraIndex);
            if (cameraInterface != null) {
                return cameraInterface.getFrameSize();
            } else {
                return null;
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }

    public synchronized int getState(int cameraIndex) {
        try {
            final ConditionController controller = flagMap.get(cameraIndex);
            if (controller != null) {
                if (controller.useStateCheck) {
                    return controller.state;
                } else {
                    return STATE_UNKNOWN;
                }
            } else {
                return STATE_UNKNOWN;
            }
        } catch (Throwable e) {
            throw new CameraManageException(e);
        }
    }
}
