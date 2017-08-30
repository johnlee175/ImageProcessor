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

import java.util.List;

import com.johnsoft.imgproc.camera.CameraManager;

import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.support.annotation.Keep;

/**
 * Simple Camera Manager
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
@Keep
public enum SimpleCameraManager {
    singleInstance;

    public static final int STATE_CLOSED = 0;
    public static final int STATE_OPEN = 1;
    public static final int STATE_PREVIEW = 2;

    public static final int FACE = Camera.CameraInfo.CAMERA_FACING_FRONT;

    private Camera camera;
    private Camera.Size frameSize;
    private SurfaceTexture surfaceTexture;
    private int count;
    private int state;

    public synchronized void open(CameraManager.PreviewSizeChooser chooser) {
        try {
            ++count;
            if (state == STATE_CLOSED) {
                int cameraIndex = -1;
                final Camera.CameraInfo cameraInfo = new Camera.CameraInfo();
                for (int camIdx = 0; camIdx < Camera.getNumberOfCameras(); ++camIdx) {
                    Camera.getCameraInfo(camIdx, cameraInfo);
                    if (cameraInfo.facing == FACE) {
                        cameraIndex = camIdx;
                        break;
                    }
                }
                camera = Camera.open(cameraIndex);
                final Camera.Parameters params = camera.getParameters();
                frameSize = chooser.chooseSize(params.getSupportedPreviewSizes());
                params.setPreviewSize(frameSize.width, frameSize.height);
                final List<String> focusModes = params.getSupportedFocusModes();
                if (focusModes.contains("continuous-video")) {
                    params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);
                }
                camera.setParameters(params);
                surfaceTexture = new SurfaceTexture(0);
                state = STATE_OPEN;
            }
        } catch (Throwable e) {
            throw new CameraManager.CameraManageException("Camera failed to open!",  e);
        }
    }

    public synchronized void startPreview(SurfaceTexture.OnFrameAvailableListener listener) {
        try {
            if (state == STATE_OPEN) {
                if (listener != null) {
                    surfaceTexture.setOnFrameAvailableListener(listener);
                }
                camera.setPreviewTexture(surfaceTexture);
                camera.startPreview();
                try {
                    surfaceTexture.detachFromGLContext();
                } catch (Throwable ignored) {
                    /* ignored */
                }
                state = STATE_PREVIEW;
            }
        } catch (Throwable e) {
            throw new CameraManager.CameraManageException(e);
        }
    }

    public synchronized void stopPreview() {
        try {
            if (state == STATE_PREVIEW) {
                camera.stopPreview();
                surfaceTexture.setOnFrameAvailableListener(null);
                state = STATE_OPEN;
            }
        } catch (Throwable e) {
            throw new CameraManager.CameraManageException(e);
        }
    }

    public synchronized void close() {
        try {
            if (count == 0 || --count != 0) {
                return;
            }
            if (state != STATE_CLOSED) {
                surfaceTexture.release();
                surfaceTexture = null;
                camera.release();
                camera = null;
                state = STATE_CLOSED;
            }
        } catch (Throwable e) {
            throw new CameraManager.CameraManageException(e);
        }
    }

    /**
     * {@link #STATE_CLOSED}, {@link #STATE_OPEN}, {@link #STATE_PREVIEW}
     */
    public synchronized int getState() {
        return state;
    }

    public synchronized SurfaceTexture getSurfaceTexture() {
        return surfaceTexture;
    }

    public synchronized Camera.Size getFrameSize() {
        return frameSize;
    }
}
