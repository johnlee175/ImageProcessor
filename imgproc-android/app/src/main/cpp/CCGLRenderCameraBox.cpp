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

static void MyFrameDataCallback(GLRenderCameraBox *glrcbox) {
    void *data = glrcbox_get_user_tag(glrcbox);
    CCGLRenderCameraBox *box = __static_cast(CCGLRenderCameraBox *, data);
    if (box) {
        CCFrameDataCallback callback = box->GetFrameDataCallback();
        if (callback) {
            callback(box);
        }
    }
}

CCGLRenderCameraBox::CCGLRenderCameraBox() {
    this->glrcbox = glrcbox_create_initialize();
    glrcbox_set_user_tag(this->glrcbox, this);
}

CCGLRenderCameraBox::~CCGLRenderCameraBox() {
    glrcbox_set_user_tag(this->glrcbox, nullptr);
    glrcbox_destroy_release(this->glrcbox);
}

bool CCGLRenderCameraBox::IsInitSuccess() {
    return this->glrcbox != nullptr;
}

int CCGLRenderCameraBox::SetUserTag(void *data) {
    this->data = data;
    return 0;
}

void *CCGLRenderCameraBox::GetUserTag() {
    return this->data;
}

int CCGLRenderCameraBox::SetFrameSize(GLuint frameWidth, GLuint frameHeight) {
    return glrcbox_set_frame_size(this->glrcbox, frameWidth, frameHeight);
}

int CCGLRenderCameraBox::SetFrontCamera(GLboolean isFrontCamera) {
    return glrcbox_set_front_camera(this->glrcbox, isFrontCamera);
}

int CCGLRenderCameraBox::SetShaderSource(const GLchar *vertexSource, const GLchar *fragmentSource) {
    return glrcbox_set_shader_source(this->glrcbox, vertexSource, fragmentSource);
}

int CCGLRenderCameraBox::GetPixels(GLubyte **pixels, size_t *pixelsSize) {
    return glrcbox_get_pixels(this->glrcbox, pixels, pixelsSize);
}

int CCGLRenderCameraBox::CreateShader() {
    return glrcbox_create_shader(this->glrcbox);
}

int CCGLRenderCameraBox::DestroyShader() {
    return glrcbox_destroy_shader(this->glrcbox);
}

int CCGLRenderCameraBox::DrawFrame(GLuint textureId) {
    return glrcbox_draw_frame(this->glrcbox, textureId);
}

int CCGLRenderCameraBox::SetWindow(EGLNativeWindowType window) {
    return glrcbox_set_window(this->glrcbox, window);
}

int CCGLRenderCameraBox::SwapBuffers() {
    return glrcbox_swap_buffers(this->glrcbox);
}

int CCGLRenderCameraBox::CreateEGL() {
    return glrcbox_create_egl(this->glrcbox);
}

int CCGLRenderCameraBox::DestroyEGL() {
    return glrcbox_destroy_egl(this->glrcbox);
}

int CCGLRenderCameraBox::SetFrameDataCallback(CCFrameDataCallback callbackFunc) {
    this->callback = callbackFunc;
    return glrcbox_set_frame_data_callback(this->glrcbox, MyFrameDataCallback);
}

CCFrameDataCallback CCGLRenderCameraBox::GetFrameDataCallback() {
    return this->callback;
}
