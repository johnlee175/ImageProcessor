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
 * gl render camera box cc implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "CCGLRenderCameraBox.hpp"

static void MyNormalFrameDataCallback(GLRenderCameraBox *glrcbox) {
    void *data = glrcbox_get_user_tag(glrcbox);
    camerabox::CCGLRenderCameraBox *box = __static_cast(camerabox::CCGLRenderCameraBox *, data);
    if (box) {
        camerabox::CCFrameDataCallback *callback = box->GetNormalFrameDataCallback();
        if (callback) {
            callback->OnDataCallback(box);
        }
    }
}

static void MyFilteredFrameDataCallback(GLRenderCameraBox *glrcbox) {
    void *data = glrcbox_get_user_tag(glrcbox);
    camerabox::CCGLRenderCameraBox *box = __static_cast(camerabox::CCGLRenderCameraBox *, data);
    if (box) {
        camerabox::CCFrameDataCallback *callback = box->GetFilteredFrameDataCallback();
        if (callback) {
            callback->OnDataCallback(box);
        }
    }
}

namespace camerabox {
    class CCGLRenderCameraBoxImpl: public CCGLRenderCameraBox {
    public:
        CCGLRenderCameraBoxImpl();
        virtual ~CCGLRenderCameraBoxImpl();
        virtual bool IsInitSuccess() override;
        virtual int SetUserTag(void *data) override;
        virtual void *GetUserTag() override;
        virtual int SetFrameSize(GLuint frameWidth, GLuint frameHeight) override;
        virtual int SetFragmentShaderType(enum FragmentShaderType fragmentShaderType) override;
        virtual int SetShaderSource(const GLchar *vertexSource,
                                    const GLchar *fragmentSource) override;
        virtual int GetPixels(GLubyte **pixels, size_t *pixelsSize) override;
        virtual int CreateShader() override;
        virtual int DestroyShader() override;
        virtual int DrawFrame(GLuint textureId) override;
        virtual int SetWindow(EGLNativeWindowType window) override;
        virtual EGLNativeWindowType GetWindow() override;
        virtual int SwapBuffers() override;
        virtual int CreateEGL() override;
        virtual int DestroyEGL() override;
        virtual int SetFrameDataCallback(CCFrameDataCallback *normalCallbackFunc,
                                         CCFrameDataCallback *filteredCallbackFunc) override;
        virtual CCFrameDataCallback *GetNormalFrameDataCallback() override;
        virtual CCFrameDataCallback *GetFilteredFrameDataCallback() override;
    private:
        GLRenderCameraBox *glrcbox;
        void *data; // user tag
        CCFrameDataCallback *normalCallback;
        CCFrameDataCallback *filteredCallback;
    };
}

camerabox::CCGLRenderCameraBox* camerabox::CCGLRenderCameraBox::create() {
    return new CCGLRenderCameraBoxImpl;
}

camerabox::CCGLRenderCameraBoxImpl::CCGLRenderCameraBoxImpl()
        :glrcbox(nullptr), data(nullptr), normalCallback(nullptr), filteredCallback(nullptr) {
    this->glrcbox = glrcbox_create_initialize();
    glrcbox_set_user_tag(this->glrcbox, this);
}

camerabox::CCGLRenderCameraBoxImpl::~CCGLRenderCameraBoxImpl() {
    glrcbox_set_user_tag(this->glrcbox, nullptr);
    glrcbox_destroy_release(this->glrcbox);
}

bool camerabox::CCGLRenderCameraBoxImpl::IsInitSuccess() {
    return this->glrcbox != nullptr;
}

int camerabox::CCGLRenderCameraBoxImpl::SetUserTag(void *data) {
    this->data = data;
    return 0;
}

void *camerabox::CCGLRenderCameraBoxImpl::GetUserTag() {
    return this->data;
}

int camerabox::CCGLRenderCameraBoxImpl::SetFrameSize(GLuint frameWidth, GLuint frameHeight) {
    return glrcbox_set_frame_size(this->glrcbox, frameWidth, frameHeight);
}

int camerabox::CCGLRenderCameraBoxImpl::SetFragmentShaderType(enum FragmentShaderType fragmentShaderType) {
    return glrcbox_set_fragment_shader_type(this->glrcbox, fragmentShaderType);
}

int camerabox::CCGLRenderCameraBoxImpl::SetShaderSource(const GLchar *vertexSource, const GLchar *fragmentSource) {
    return glrcbox_set_shader_source(this->glrcbox, vertexSource, fragmentSource);
}

int camerabox::CCGLRenderCameraBoxImpl::GetPixels(GLubyte **pixels, size_t *pixelsSize) {
    return glrcbox_get_pixels(this->glrcbox, pixels, pixelsSize);
}

int camerabox::CCGLRenderCameraBoxImpl::CreateShader() {
    return glrcbox_create_shader(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::DestroyShader() {
    return glrcbox_destroy_shader(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::DrawFrame(GLuint textureId) {
    return glrcbox_draw_frame(this->glrcbox, textureId);
}

int camerabox::CCGLRenderCameraBoxImpl::SetWindow(EGLNativeWindowType window) {
    return glrcbox_set_window(this->glrcbox, window);
}

EGLNativeWindowType camerabox::CCGLRenderCameraBoxImpl::GetWindow() {
    return glrcbox_get_window(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::SwapBuffers() {
    return glrcbox_swap_buffers(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::CreateEGL() {
    return glrcbox_create_egl(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::DestroyEGL() {
    return glrcbox_destroy_egl(this->glrcbox);
}

int camerabox::CCGLRenderCameraBoxImpl::SetFrameDataCallback(camerabox::CCFrameDataCallback *normalCallbackFunc,
                                                         camerabox::CCFrameDataCallback *filteredCallbackFunc) {
    this->normalCallback = normalCallbackFunc;
    this->filteredCallback = filteredCallbackFunc;
    return glrcbox_set_frame_data_callback(this->glrcbox, GL_FALSE,
                                           MyNormalFrameDataCallback, MyFilteredFrameDataCallback);
}

camerabox::CCFrameDataCallback *camerabox::CCGLRenderCameraBoxImpl::GetNormalFrameDataCallback() {
    return this->normalCallback;
}

camerabox::CCFrameDataCallback *camerabox::CCGLRenderCameraBoxImpl::GetFilteredFrameDataCallback() {
    return this->filteredCallback;
}