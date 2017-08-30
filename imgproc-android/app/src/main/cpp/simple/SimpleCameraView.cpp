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
 * SimpleCameraView implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-31
 */
#include "base.h"
#include "SimpleCameraView.hpp"

const GLfloat SimpleCameraView::vertex_coords[] = {
        -1.0f, 1.0f, 0.0f, /* top left */
        -1.0f, -1.0f, 0.0f, /* bottom left */
        1.0f, -1.0f, 0.0f, /* bottom right */
        1.0f, 1.0f, 0.0f, /* top right */
};

const GLfloat SimpleCameraView::texture_coords[] = {
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
};

const GLushort SimpleCameraView::draw_order[] = { 0, 1, 2, 0, 2, 3};

const GLchar *SimpleCameraView::vertex_shader_source = ""
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTextureCoord;\n"
        "varying vec2 vTextureCoord;\n"
        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTextureCoord = aTextureCoord;\n"
        "}";

const GLchar *SimpleCameraView::fragment_shader_source = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
        "}";

const GLchar *SimpleCameraView::fragment_shader_source_x = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  vec2 uv = vTextureCoord;\n"
        "  uv.x = 1.0 - uv.x;\n"
        "  gl_FragColor = texture2D(sTexture, uv);\n"
        "}";

const GLchar *SimpleCameraView::fragment_shader_source_y = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  vec2 uv = vTextureCoord;\n"
        "  uv.y = 1.0 - uv.y;\n"
        "  gl_FragColor = texture2D(sTexture, uv);\n"
        "}";

SimpleCameraView::SimpleCameraView(FrameDataCallback *callback, GLuint w, GLuint h,
                                   EGLNativeWindowType window, FragmentShaderType type)
        :egl_window(NULL), egl_display(NULL), egl_config(NULL), egl_context(NULL),
         egl_surface(NULL), program(0), vertex_shader(0), fragment_shader(0),
         frame_width(0), frame_height(0), pixels(NULL), pixels_size(0),
         frame_data_callback(NULL), fragment_shader_type(FST_NORMAL) {
    this->frame_data_callback = callback;
    this->frame_width = w;
    this->frame_height = h;
    this->egl_window = window;
    this->fragment_shader_type = type;
    if (this->frame_data_callback) {
        this->pixels_size = sizeof(GLubyte) * this->frame_width * this->frame_height * 4;
        this->pixels = static_cast<GLubyte *>(malloc(this->pixels_size));
    }
}

SimpleCameraView::~SimpleCameraView() {
    if (this->frame_data_callback) {
        delete this->frame_data_callback;
    }
    if (this->pixels) {
        free(this->pixels);
    }
}

void SimpleCameraView::AssembleEglErrorString(GLint error, const char *prefix, char *buffer) {
    switch (error) {
        case EGL_SUCCESS:
            sprintf(buffer, "%s %s\n", prefix, "EGL_SUCCESS");
        case EGL_NOT_INITIALIZED:
            sprintf(buffer, "%s %s\n", prefix, "EGL_NOT_INITIALIZED");
        case EGL_BAD_ACCESS:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_ACCESS");
        case EGL_BAD_ALLOC:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_ALLOC");
        case EGL_BAD_ATTRIBUTE:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_ATTRIBUTE");
        case EGL_BAD_CONFIG:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_CONFIG");
        case EGL_BAD_CONTEXT:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_CONTEXT");
        case EGL_BAD_CURRENT_SURFACE:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_CURRENT_SURFACE");
        case EGL_BAD_DISPLAY:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_DISPLAY");
        case EGL_BAD_MATCH:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_MATCH");
        case EGL_BAD_NATIVE_PIXMAP:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_NATIVE_PIXMAP");
        case EGL_BAD_NATIVE_WINDOW:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_NATIVE_WINDOW");
        case EGL_BAD_PARAMETER:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_PARAMETER");
        case EGL_BAD_SURFACE:
            sprintf(buffer, "%s %s\n", prefix, "EGL_BAD_SURFACE");
        case EGL_CONTEXT_LOST:
            sprintf(buffer, "%s %s\n", prefix, "EGL_CONTEXT_LOST");
        default:
            sprintf(buffer, "%s 0x%d\n", prefix, error);
    }
}

bool SimpleCameraView::CreateEgl() {
    char buffer[256];
    if ((this->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        AssembleEglErrorString(eglGetError(), "eglGetDisplay failed:", buffer);
        LOGW("%s", buffer);
        return false;
    }
    EGLint major, minor;
    if (!eglInitialize(this->egl_display, &major, &minor)) {
        AssembleEglErrorString(eglGetError(), "eglInitialize failed:", buffer);
        LOGW("%s", buffer);
        return false;
    } else {
        LOGI("EGL version %d-%d\n", major, minor);
    }
    EGLint num_config;
    const EGLint configSpec[] = {
            EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_LUMINANCE_SIZE, 0,
            EGL_DEPTH_SIZE, 0,
            EGL_STENCIL_SIZE, 0,
            EGL_ALPHA_MASK_SIZE, 0,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };
    eglChooseConfig(this->egl_display, configSpec, &this->egl_config, 1, &num_config);
    if (num_config <= 0) {
        AssembleEglErrorString(eglGetError(), "eglChooseConfig failed:", buffer);
        LOGW("%s", buffer);
        return false;
    }
    GLint contextSpec[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    if ((this->egl_context = eglCreateContext(this->egl_display,
                                              this->egl_config,
                                              EGL_NO_CONTEXT,
                                              contextSpec)) == EGL_NO_CONTEXT) {
        AssembleEglErrorString(eglGetError(), "eglCreateContext failed:", buffer);
        LOGW("%s", buffer);
        return false;
    }
    if ((this->egl_surface = eglCreateWindowSurface(this->egl_display,
                                                       this->egl_config,
                                                       this->egl_window,
                                                       NULL)) == EGL_NO_SURFACE) {
        AssembleEglErrorString(eglGetError(), "eglCreateWindowSurface failed:", buffer);
        LOGW("%s", buffer);
        return false;
    }
    if (!eglMakeCurrent(this->egl_display, this->egl_surface,
                        this->egl_surface, this->egl_context)) {
        AssembleEglErrorString(eglGetError(), "eglMakeCurrent failed:", buffer);
        LOGW("%s", buffer);
        return false;
    }
    return true;
}

bool SimpleCameraView::SwapBuffers() {
    if (!eglSwapBuffers(this->egl_display, this->egl_surface)) {
        LOGW("Can not swap buffers\n");
        return false;
    }
    return true;
}

void SimpleCameraView::DestroyEgl() {
    eglMakeCurrent(this->egl_display,
                   EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(this->egl_display, this->egl_surface);
    eglDestroyContext(this->egl_display, this->egl_context);
    eglTerminate(this->egl_display);
}

bool SimpleCameraView::CreateShader() {
    if ((this->vertex_shader = glCreateShader(GL_VERTEX_SHADER)) == GL_FALSE) {
        LOGW("error occurs in glCreateShader()\n");
        return false;
    }
    if ((this->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)) == GL_FALSE) {
        LOGW("error occurs in glCreateShader()\n");
        return false;
    }
    glShaderSource(this->vertex_shader, 1,
                   &this->vertex_shader_source, NULL);
    switch (this->fragment_shader_type) {
        case FST_REVERSE_X:
            glShaderSource(this->fragment_shader, 1,
                           &this->fragment_shader_source_x, NULL);
            break;
        case FST_REVERSE_Y:
            glShaderSource(this->fragment_shader, 1,
                           &this->fragment_shader_source_y, NULL);
            break;
        default:
            glShaderSource(this->fragment_shader, 1,
                           &this->fragment_shader_source, NULL);
            break;
    }
    GLint status, log_length;
    glCompileShader(this->vertex_shader);
    glGetShaderiv(this->vertex_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderiv(this->vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar info_log[log_length];
        glGetShaderInfoLog(this->vertex_shader, sizeof(info_log),
                           NULL, info_log);
        LOGW("vertex shader compile failed: %s\n", info_log);
        return false;
    }
    glCompileShader(this->fragment_shader);
    glGetShaderiv(this->fragment_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        glGetShaderiv(this->fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar info_log[log_length];
        glGetShaderInfoLog(this->fragment_shader, sizeof(info_log),
                           NULL, info_log);
        LOGW("fragment shader compile failed: %s\n", info_log);
        return false;
    }
    if ((this->program = glCreateProgram()) == GL_FALSE) {
        LOGW("error occurs in glCreateProgram()\n");
        return false;
    }
    glAttachShader(this->program, this->vertex_shader);
    glAttachShader(this->program, this->fragment_shader);
    glLinkProgram(this->program);
    glGetProgramiv(this->program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &log_length);
        GLchar info_log[log_length];
        glGetProgramInfoLog(this->program, sizeof(info_log),
                            NULL, info_log);
        LOGW("program link failed: %s\n", info_log);
        return false;
    }
    return true;
}

bool SimpleCameraView::DrawFrame() {
    glViewport(0, 0, this->frame_width, this->frame_height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!this->program) {
        LOGW("is CreateShader() not called yet?\n");
        return false;
    }

    glUseProgram(this->program);

    GLint position_location = glGetAttribLocation(this->program, "aPosition");
    GLint texture_coord_location = glGetAttribLocation(this->program, "aTextureCoord");
    GLint texture_location = glGetUniformLocation(this->program, "sTexture");

    if (position_location < 0 || texture_coord_location < 0 || texture_location < 0) {
        LOGW("can't get location attribute for shader\n");
        return false;
    }

    GLuint position_index = static_cast<GLuint>(position_location);
    GLuint texture_coord_index = static_cast<GLuint>(texture_coord_location);
    GLuint texture_index = static_cast<GLuint>(texture_location);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glEnableVertexAttribArray(position_index);
    glVertexAttribPointer(position_index, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), vertex_coords);

    glEnableVertexAttribArray(texture_coord_index);
    glVertexAttribPointer(texture_coord_index, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texture_coords);

    glUniform1i(texture_index, 0);

    glDrawElements(GL_TRIANGLES, sizeof(draw_order) / sizeof(GLushort), GL_UNSIGNED_SHORT, draw_order);

    glDisableVertexAttribArray(position_index);
    glDisableVertexAttribArray(texture_coord_index);

    if (this->frame_data_callback && this->pixels) {
        glReadPixels(0, 0, this->frame_width, this->frame_height,
                     GL_RGBA, GL_UNSIGNED_BYTE, this->pixels);
        this->frame_data_callback->onDataCallback(this);
    }
    return true;
}

void SimpleCameraView::DestroyShader() {
    glDeleteShader(this->vertex_shader);
    glDeleteShader(this->fragment_shader);
    glDeleteProgram(this->program);
}

EGLNativeWindowType SimpleCameraView::GetWindow() {
    return this->egl_window;
}

GLuint SimpleCameraView::ReadPixels(GLubyte **pixels) {
    *pixels = this->pixels;
    return this->pixels_size;
}
