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
 * SimpleCameraView interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
#ifndef EXPR_RECO_SIMPLECAMERAVIEW_HPP
#define EXPR_RECO_SIMPLECAMERAVIEW_HPP

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class SimpleCameraView;

class FrameDataCallback {
public:
    virtual ~FrameDataCallback() { }
    virtual void onDataCallback(SimpleCameraView *view) = 0;
};

enum EnumFragmentShaderType {
    FST_NORMAL = 0, FST_REVERSE_X = -1, FST_REVERSE_Y = 1
};

typedef enum EnumFragmentShaderType FragmentShaderType;

class SimpleCameraViewImpl;

class SimpleCameraView {
public:
    SimpleCameraView(FrameDataCallback *callback, GLuint w, GLuint h,
                     EGLNativeWindowType window, FragmentShaderType type);
    ~SimpleCameraView();

    bool CreateEgl();
    bool SwapBuffers();
    void DestroyEgl();

    bool CreateShader();
    bool DrawFrame();
    void DestroyShader();

    EGLNativeWindowType GetWindow();
    GLuint ReadPixels(GLubyte **pixels);
private:
    SimpleCameraViewImpl *impl;
};

#endif // EXPR_RECO_SIMPLECAMERAVIEW_HPP
