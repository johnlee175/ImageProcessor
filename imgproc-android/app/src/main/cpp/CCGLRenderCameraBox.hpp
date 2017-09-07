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
/**
 * gl render camera box cc interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_CCGLRENDERCAMERABOX_H
#define IMGPROC_ANDROID_CCGLRENDERCAMERABOX_H

#include "gl_render_camera_box.h"

namespace camerabox {

class CCGLRenderCameraBox;

class CCFrameDataCallback {
public:
    virtual ~CCFrameDataCallback() { }
    virtual void OnDataCallback(CCGLRenderCameraBox *glrcbox) = 0;
};

class CCGLRenderCameraBox {
public:
    static CCGLRenderCameraBox *create();
    virtual ~CCGLRenderCameraBox() { };
    virtual bool IsInitSuccess() = 0;
    virtual int SetUserTag(void *data) = 0;
    virtual void *GetUserTag() = 0;
    virtual int SetFrameSize(GLuint frameWidth, GLuint frameHeight) = 0;
    virtual int SetFragmentShaderType(enum FragmentShaderType fragmentShaderType) = 0;
    virtual int SetShaderSource(const GLchar *vertexSource,
                                const GLchar *fragmentSource) = 0;
    virtual int GetPixels(GLubyte **pixels, size_t *pixelsSize) = 0;
    virtual int CreateShader() = 0;
    virtual int DestroyShader() = 0;
    virtual int DrawFrame(GLuint textureId) = 0;
    virtual int SetWindow(EGLNativeWindowType window) = 0;
    virtual EGLNativeWindowType GetWindow() = 0;
    virtual int SwapBuffers() = 0;
    virtual int CreateEGL() = 0;
    virtual int DestroyEGL() = 0;
    virtual int SetFrameDataCallback(CCFrameDataCallback *normalCallbackFunc,
                                     CCFrameDataCallback *filteredCallbackFunc) = 0;
    virtual CCFrameDataCallback *GetNormalFrameDataCallback() = 0;
    virtual CCFrameDataCallback *GetFilteredFrameDataCallback() = 0;
};

}

#endif //IMGPROC_ANDROID_CCGLRENDERCAMERABOX_H
