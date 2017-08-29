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

// TODO Not use function pointer, use abstract callback class instead
typedef void (*CCFrameDataCallback)(CCGLRenderCameraBox *glrcbox);

class CCGLRenderCameraBox {
public:
    CCGLRenderCameraBox();
    ~CCGLRenderCameraBox();
    bool IsInitSuccess();
    int SetUserTag(void *data);
    void *GetUserTag();
    int SetFrameSize(GLuint frameWidth,
                     GLuint frameHeight);
    int SetFrontCamera(GLboolean isFrontCamera);
    int SetShaderSource(const GLchar *vertexSource,
                        const GLchar *fragmentSource);
    int GetPixels(GLubyte **pixels, size_t *pixelsSize);
    int CreateShader();
    int DestroyShader();
    int DrawFrame(GLuint textureId);
    int SetWindow(EGLNativeWindowType window);
    EGLNativeWindowType GetWindow();
    int SwapBuffers();
    int CreateEGL();
    int DestroyEGL();
    int SetFrameDataCallback(CCFrameDataCallback callbackFunc);
    CCFrameDataCallback GetFrameDataCallback();
private:
    GLRenderCameraBox *glrcbox;
    void *data; // user tag
    CCFrameDataCallback callback;
};

}

#endif //IMGPROC_ANDROID_CCGLRENDERCAMERABOX_H
