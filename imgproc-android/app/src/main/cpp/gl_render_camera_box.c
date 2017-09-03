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
 * gl render camera box c implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "base.h"
#include "gl_render_camera_box.h"

#ifndef GLRCBOX_CONSTANT_DEFINED
#define GLRCBOX_CONSTANT_DEFINED

const GLfloat glrcbox_vertex_coords[] = {
        -1.0f, 1.0f, 0.0f, /* top left */
        -1.0f, -1.0f, 0.0f, /* bottom left */
        1.0f, -1.0f, 0.0f, /* bottom right */
        1.0f, 1.0f, 0.0f, /* top right */
};

/* texture coordinate in android OpenGL es:
   is not from bottom left to top right, just from top left to bottom right! */
const GLfloat glrcbox_back_texture_coords[] = { /* rotate 0 degree */
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
};

/* texture coordinate in android OpenGL es:
   is not from bottom left to top right, just from top left to bottom right! */
const GLfloat glrcbox_front_texture_coords[] = { /* rotate 180 degree */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
};

const GLushort glrcbox_draw_order[] = { 0, 1, 2, 0, 2, 3 };

const GLuint glrcbox_per_vertex_coord_component = 3;
const GLuint glrcbox_per_texture_coord_component = 2;

const GLchar *glrcbox_vertex_shader_source = ""
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTextureCoord;\n"
        "varying vec2 vTextureCoord;\n"
        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTextureCoord = aTextureCoord;\n"
        "}";
const GLchar *glrcbox_fragment_shader_source = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
        "}";

#endif /* GLRCBOX_CONSTANT_DEFINED */

struct tagGLRenderCameraBox {
    EGLNativeWindowType egl_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    const GLchar *vertex_shader_source;
    const GLchar *fragment_shader_source;
    GLboolean is_front_camera;
    GLuint frame_width;
    GLuint frame_height;
    GLubyte *pixels;
    size_t pixels_size;
    FrameDataCallback frame_data_callback;
    void *user_tag_data;
};

static void glrcbox_egl_error_string(GLint error, const char *prefix, char *buffer) {
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

static int glrcbox_upside_down(GLubyte *pixels, GLuint width, GLuint height, GLuint channels) {
    if (pixels) {
        size_t row_size = _static_cast(size_t) (width * channels);
        GLubyte *temp_row_pixels = _static_cast(GLubyte *) malloc(row_size);
        if (temp_row_pixels) {
            GLint i = 0, j = height - 1, temp_unit = -1;
            void *temp_ptr_i = NULL, *temp_ptr_j = NULL;
            while(i < j) {
                temp_unit = width * channels;
                temp_ptr_i = &pixels[i * temp_unit];
                temp_ptr_j = &pixels[j * temp_unit];
                memcpy(temp_row_pixels, temp_ptr_j, row_size);
                memcpy(temp_ptr_j, temp_ptr_i, row_size);
                memcpy(temp_ptr_i, temp_row_pixels, row_size);
                ++i, --j;
            }
            free(temp_row_pixels);
            return 0;
        }
    }
    return -1;
}

GLRenderCameraBox *glrcbox_create_initialize() {
    GLRenderCameraBox *glrcbox = _static_cast(GLRenderCameraBox *)
            malloc(sizeof(GLRenderCameraBox));
    if (!glrcbox) {
        LOGW("malloc GLRenderCameraBox failed\n");
        return NULL;
    }
    memset(glrcbox, 0, sizeof(GLRenderCameraBox));
    return glrcbox;
}

void glrcbox_destroy_release(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        if (glrcbox->pixels) {
            free(glrcbox->pixels);
            glrcbox->pixels = NULL;
        }
        free(glrcbox);
    }
}

int glrcbox_set_user_tag(GLRenderCameraBox *glrcbox, void *data) {
    if (glrcbox) {
        glrcbox->user_tag_data = data;
        return 0;
    }
    return -1;
}

void *glrcbox_get_user_tag(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        return glrcbox->user_tag_data;
    }
    return NULL;
}

int glrcbox_set_frame_data_callback(GLRenderCameraBox *glrcbox,
                                    FrameDataCallback callback_func) {
    if (glrcbox) {
        glrcbox->frame_data_callback = callback_func;
        return 0;
    }
    return -1;
}

int glrcbox_set_frame_size(GLRenderCameraBox *glrcbox,
                           GLuint frame_width,
                           GLuint frame_height) {
    if (glrcbox) {
        glrcbox->frame_width = frame_width;
        glrcbox->frame_height = frame_height;
        glrcbox->pixels_size = sizeof(GLubyte) * frame_width * frame_height * 4;
        GLubyte *pixels = _static_cast(GLubyte *) malloc(glrcbox->pixels_size);
        if (!pixels) {
            LOGW("malloc (frame_width * frame_height * 4)'s GLubyte for "
                         "GLRenderCameraBox.pixels failed\n");
            return -1;
        }
        memset(pixels, 0, glrcbox->pixels_size);
        glrcbox->pixels = pixels;
        return 0;
    }
    return -1;
}

int glrcbox_set_front_camera(GLRenderCameraBox *glrcbox, GLboolean is_front_camera) {
    if (glrcbox) {
        glrcbox->is_front_camera = is_front_camera;
        return 0;
    }
    return -1;
}

int glrcbox_set_shader_source(GLRenderCameraBox *glrcbox,
                              const GLchar *vertex_source,
                              const GLchar *fragment_source) {
    if (glrcbox) {
        if (vertex_source) {
            glrcbox->vertex_shader_source = vertex_source;
        }
        if (fragment_source) {
            glrcbox->fragment_shader_source = fragment_source;
        }
        return 0;
    }
    return -1;
}

int glrcbox_get_pixels(GLRenderCameraBox *glrcbox,
                       GLubyte **pixels, size_t *pixels_size) {
    if (glrcbox && glrcbox->pixels && glrcbox->pixels_size) {
        *pixels = glrcbox->pixels;
        *pixels_size = glrcbox->pixels_size;
        return 0;
    }
    return -1;
}

int glrcbox_create_shader(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        if (!glrcbox->vertex_shader_source) {
            glrcbox->vertex_shader_source = glrcbox_vertex_shader_source;
        }
        if (!glrcbox->fragment_shader_source) {
            glrcbox->fragment_shader_source = glrcbox_fragment_shader_source;
        }
        if ((glrcbox->vertex_shader = glCreateShader(GL_VERTEX_SHADER)) == GL_FALSE) {
            LOGW("error occurs in glCreateShader()\n");
            return -1;
        }
        if ((glrcbox->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)) == GL_FALSE) {
            LOGW("error occurs in glCreateShader()\n");
            return -1;
        }
        glShaderSource(glrcbox->vertex_shader, 1,
                       &glrcbox->vertex_shader_source, NULL);
        glShaderSource(glrcbox->fragment_shader, 1,
                       &glrcbox->fragment_shader_source, NULL);
        GLint status, log_length;
        glCompileShader(glrcbox->vertex_shader);
        glGetShaderiv(glrcbox->vertex_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(glrcbox->vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(glrcbox->vertex_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("vertex shader compile failed: %s\n", info_log);
            return -1;
        }
        glCompileShader(glrcbox->fragment_shader);
        glGetShaderiv(glrcbox->fragment_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(glrcbox->fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(glrcbox->fragment_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("fragment shader compile failed: %s\n", info_log);
            return -1;
        }
        if ((glrcbox->program = glCreateProgram()) == GL_FALSE) {
            LOGW("error occurs in glCreateProgram()\n");
            return -1;
        }
        glAttachShader(glrcbox->program, glrcbox->vertex_shader);
        glAttachShader(glrcbox->program, glrcbox->fragment_shader);
        glLinkProgram(glrcbox->program);
        glGetProgramiv(glrcbox->program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            glGetProgramiv(glrcbox->program, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetProgramInfoLog(glrcbox->program, sizeof(info_log),
                                NULL, info_log);
            LOGW("program link failed: %s\n", info_log);
            return -1;
        }
        return 0;
    }
    return -1;
}

int glrcbox_destroy_shader(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        glDeleteShader(glrcbox->vertex_shader);
        glDeleteShader(glrcbox->fragment_shader);
        glDeleteProgram(glrcbox->program);
        glrcbox->vertex_shader = 0;
        glrcbox->fragment_shader = 0;
        glrcbox->program = 0;
        return 0;
    }
    return -1;
}

int glrcbox_draw_frame(GLRenderCameraBox *glrcbox, GLuint texture_id) {
    if (glrcbox) {
        glViewport(0, 0, glrcbox->frame_width, glrcbox->frame_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!glrcbox->program) {
            LOGW("is glrcbox_create_shader(GLRenderCameraBox *) not called yet?\n");
            return -1;
        }

        glUseProgram(glrcbox->program);

        GLint position_location = glGetAttribLocation(glrcbox->program,
                                                      "aPosition");
        GLint texture_coord_location = glGetAttribLocation(glrcbox->program,
                                                           "aTextureCoord");
        GLint texture_location = glGetUniformLocation(glrcbox->program,
                                                      "sTexture");

        if (position_location < 0
            || texture_coord_location < 0
            || texture_location < 0) {
            LOGW("can't get location attribute for shader\n");
            return -1;
        }

        GLuint position_index = _static_cast(GLuint) position_location;
        GLuint texture_coord_index = _static_cast(GLuint) texture_coord_location;
        GLuint texture_index = _static_cast(GLuint) texture_location;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture_id);

        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glEnableVertexAttribArray(position_index);
        glVertexAttribPointer(position_index, glrcbox_per_vertex_coord_component,
                              GL_FLOAT, GL_FALSE,
                              glrcbox_per_vertex_coord_component * sizeof(GLfloat),
                              glrcbox_vertex_coords);

        glEnableVertexAttribArray(texture_coord_index);
        if (glrcbox->is_front_camera == GL_FALSE) {
            glVertexAttribPointer(texture_coord_index,
                                  glrcbox_per_texture_coord_component,
                                  GL_FLOAT, GL_FALSE,
                                  glrcbox_per_texture_coord_component * sizeof(GLfloat),
                                  glrcbox_back_texture_coords);
        } else {
            glVertexAttribPointer(texture_coord_index,
                                  glrcbox_per_texture_coord_component,
                                  GL_FLOAT, GL_FALSE,
                                  glrcbox_per_texture_coord_component * sizeof(GLfloat),
                                  glrcbox_front_texture_coords);
        }

        glUniform1i(texture_index, 0);

        glDrawElements(GL_TRIANGLES, sizeof(glrcbox_draw_order) / sizeof(GLushort),
                       GL_UNSIGNED_SHORT, glrcbox_draw_order);

        glDisableVertexAttribArray(position_index);
        glDisableVertexAttribArray(texture_coord_index);

        if (glrcbox->frame_data_callback && glrcbox->pixels) {
            glReadPixels(0, 0, glrcbox->frame_width, glrcbox->frame_height,
                         GL_RGBA, GL_UNSIGNED_BYTE, glrcbox->pixels);
            /* Open GL (0,0) at lower left corner */
            glrcbox_upside_down(glrcbox->pixels, glrcbox->frame_width, glrcbox->frame_height,
                                4 /* RGBA */);
            glrcbox->frame_data_callback(glrcbox);
        }

        return 0;
    }
    return -1;
}

int glrcbox_set_window(GLRenderCameraBox *glrcbox, EGLNativeWindowType window) {
    if (glrcbox) {
        glrcbox->egl_window = window;
        return 0;
    }
    return -1;
}

EGLNativeWindowType glrcbox_get_window(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        return glrcbox->egl_window;
    }
    return NULL;
}

int glrcbox_swap_buffers(GLRenderCameraBox *glrcbox) {
    if (!glrcbox || !eglSwapBuffers(glrcbox->egl_display, glrcbox->egl_surface)) {
        LOGW("Can not swap buffers\n");
        return -1;
    }
    return 0;
}

int glrcbox_create_egl(GLRenderCameraBox *glrcbox) {
    if (!glrcbox) {
        return -1;
    }
    char buffer[256];
    if ((glrcbox->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY))
        == EGL_NO_DISPLAY) {
        glrcbox_egl_error_string(eglGetError(), "eglGetDisplay failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    EGLint major, minor;
    if (!eglInitialize(glrcbox->egl_display, &major, &minor)) {
        glrcbox_egl_error_string(eglGetError(), "eglInitialize failed:", buffer);
        LOGW("%s", buffer);
        return -1;
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
    eglChooseConfig(glrcbox->egl_display, configSpec, &glrcbox->egl_config,
                    1, &num_config);
    if (num_config <= 0) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglChooseConfig failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    GLint contextSpec[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    if ((glrcbox->egl_context = eglCreateContext(glrcbox->egl_display,
                                                 glrcbox->egl_config,
                                                 EGL_NO_CONTEXT,
                                                 contextSpec)) == EGL_NO_CONTEXT) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglCreateContext failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    if ((glrcbox->egl_surface = eglCreateWindowSurface(glrcbox->egl_display,
                                                       glrcbox->egl_config,
                                                       glrcbox->egl_window,
                                                       NULL)) == EGL_NO_SURFACE) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglCreateWindowSurface failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    if (!eglMakeCurrent(glrcbox->egl_display, glrcbox->egl_surface,
                        glrcbox->egl_surface, glrcbox->egl_context)) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglMakeCurrent failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    return 0;
}

int glrcbox_destroy_egl(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        eglMakeCurrent(glrcbox->egl_display,
                       EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(glrcbox->egl_display, glrcbox->egl_surface);
        eglDestroyContext(glrcbox->egl_display, glrcbox->egl_context);
        eglTerminate(glrcbox->egl_display);
        glrcbox->egl_display = EGL_NO_DISPLAY;
        glrcbox->egl_context = EGL_NO_CONTEXT;
        glrcbox->egl_surface = EGL_NO_SURFACE;
        return 0;
    }
    return -1;
}
