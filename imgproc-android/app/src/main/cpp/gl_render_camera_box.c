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
#include "base/base.h"
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
const GLfloat glrcbox_texture_coords[] = {
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
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
/* if change shader source code, search replace_target in this file */
const GLchar *glrcbox_fragment_shader_source = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  vec2 vTexture = vTextureCoord;\n"
        "  gl_FragColor = texture2D(sTexture, vTexture);\n"
        "}";
const GLchar *glrcbox_fragment_shader_source_x = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  vec2 vTexture = vTextureCoord;\n"
        "  vTexture.x = 1.0 - vTexture.x;\n"
        "  gl_FragColor = texture2D(sTexture, vTexture);\n"
        "}";
const GLchar *glrcbox_fragment_shader_source_y = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  vec2 vTexture = vTextureCoord;\n"
        "  vTexture.y = 1.0 - vTexture.y;\n"
        "  gl_FragColor = texture2D(sTexture, vTexture);\n"
        "}";

#endif /* GLRCBOX_CONSTANT_DEFINED */

struct tagGLRenderCameraBox {
    EGLNativeWindowType egl_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    GLuint normal_program;
    GLuint filter_program;
    const GLchar *vertex_shader_source;
    const GLchar *fragment_shader_source;
    enum FragmentShaderType fragment_shader_type;
    GLuint frame_width;
    GLuint frame_height;
    GLubyte *pixels;
    size_t pixels_size;
    FrameDataCallback normal_frame_data_callback;
    FrameDataCallback filtered_frame_data_callback;
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
            GLint i = 0, j = height - 1;
            void *temp_ptr_i = NULL, *temp_ptr_j = NULL;
            while(i < j) {
                temp_ptr_i = &pixels[i * row_size];
                temp_ptr_j = &pixels[j * row_size];
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

static void glrcbox_replace_string_once_with(char *_restrict_ out, size_t out_size,
                                             const char *_restrict_ origin,
                                             const char *_restrict_ pattern,
                                             const char *_restrict_ replacement) {
    if (out && origin && pattern) {
        memset(out, 0, out_size);
        const char *ptr = strstr(origin, pattern);
        if (ptr) {
            char *dst = strncpy(out, origin, ptr - origin) + (ptr - origin);
            dst = strncpy(dst, replacement, strlen(replacement)) + strlen(replacement);
            const char *retain = ptr + strlen(pattern);
            strncpy(dst, retain, strlen(retain));
        } else {
            strncpy(out, origin, strlen(origin));
        }
    }
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

int glrcbox_set_frame_data_callback(GLRenderCameraBox *glrcbox, GLboolean force_update,
                                    FrameDataCallback normal_callback_func,
                                    FrameDataCallback filtered_callback_func) {
    if (glrcbox) {
        if (force_update) {
            glrcbox->normal_frame_data_callback = normal_callback_func;
            glrcbox->filtered_frame_data_callback = filtered_callback_func;
        } else {
            if (normal_callback_func) {
                glrcbox->normal_frame_data_callback = normal_callback_func;
            }
            if (filtered_callback_func) {
                glrcbox->filtered_frame_data_callback = filtered_callback_func;
            }
        }
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

int glrcbox_set_fragment_shader_type(GLRenderCameraBox *glrcbox,
                                     enum FragmentShaderType fragment_shader_type) {
    if (glrcbox) {
        glrcbox->fragment_shader_type = fragment_shader_type;
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

static int glrcbox_do_create_shader(GLRenderCameraBox *glrcbox, GLboolean normal) {
    if (glrcbox) {
        LOGI("do create shader (normal=%d)", normal);
        GLint status, log_length;
        GLuint vertex_shader, fragment_shader;
        if ((vertex_shader = glCreateShader(GL_VERTEX_SHADER)) == 0) {
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("error occurs in glCreateShader(): %s\n", info_log);
            return -1;
        }
        if ((fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)) == 0) {
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("error occurs in glCreateShader(): %s\n", info_log);
            return -1;
        }
        const char *vertex_shader_source = NULL, *fragment_shader_source = NULL;
        if (normal) {
            vertex_shader_source = glrcbox_vertex_shader_source;
            switch(glrcbox->fragment_shader_type) {
                case REVERSE_X:
                    fragment_shader_source = glrcbox_fragment_shader_source_x;
                    break;
                case REVERSE_Y:
                    fragment_shader_source = glrcbox_fragment_shader_source_y;
                    break;
                default:
                    fragment_shader_source = glrcbox_fragment_shader_source;
                    break;
            }
        } else {
            if (!glrcbox->fragment_shader_source) {
                LOGW("fragment shader source not set");
                return -1;
            }
            vertex_shader_source = glrcbox_vertex_shader_source;
            const char *replace_target = "  gl_FragColor = texture2D(sTexture, vTexture);\n";
            const int temp_buffer_size = 1024;
            char temp_buffer[temp_buffer_size];
            switch(glrcbox->fragment_shader_type) {
                case REVERSE_X: {
                    glrcbox_replace_string_once_with(temp_buffer, temp_buffer_size,
                                                     glrcbox_fragment_shader_source_x,
                                                     replace_target, glrcbox->fragment_shader_source);
                    fragment_shader_source = temp_buffer;
                }
                    break;
                case REVERSE_Y: {
                    glrcbox_replace_string_once_with(temp_buffer, temp_buffer_size,
                                                     glrcbox_fragment_shader_source_y,
                                                     replace_target, glrcbox->fragment_shader_source);
                    fragment_shader_source = temp_buffer;
                }
                    break;
                default: {
                    glrcbox_replace_string_once_with(temp_buffer, temp_buffer_size,
                                                     glrcbox_fragment_shader_source,
                                                     replace_target, glrcbox->fragment_shader_source);
                    fragment_shader_source = temp_buffer;
                }
                    break;
            }
        }
        glShaderSource(vertex_shader, 1,
                       &vertex_shader_source, NULL);
        glShaderSource(fragment_shader, 1,
                       &fragment_shader_source, NULL);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("vertex shader compile failed: %s\n", info_log);
            return -1;
        }
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(fragment_shader, sizeof(info_log),
                               NULL, info_log);
            LOGW("fragment shader compile failed: %s\n", info_log);
            return -1;
        }
        GLuint program;
        if ((program = glCreateProgram()) == 0) {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetProgramInfoLog(program, sizeof(info_log),
                               NULL, info_log);
            LOGW("error occurs in glCreateProgram():%s\n", info_log);
            return -1;
        }
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetProgramInfoLog(program, sizeof(info_log),
                                NULL, info_log);
            LOGW("program link failed: %s\n", info_log);
            return -1;
        }
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        if (normal) {
            glrcbox->normal_program = program;
        } else {
            glrcbox->filter_program = program;
        }
        return 0;
    }
    return -1;
}

int glrcbox_create_shader(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        int normal = 0, filter = 0;
        normal = glrcbox_do_create_shader(glrcbox, GL_TRUE);
        if (glrcbox->fragment_shader_source) {
            filter = glrcbox_do_create_shader(glrcbox, GL_FALSE);
        }
        if (normal == 0 && filter == 0) {
            return 0;
        }
    }
    return -1;
}

int glrcbox_destroy_shader(GLRenderCameraBox *glrcbox) {
    if (glrcbox) {
        if (glrcbox->normal_program) {
            glDeleteProgram(glrcbox->normal_program);
        }
        if (glrcbox->filter_program) {
            glDeleteProgram(glrcbox->filter_program);
        }
        glrcbox->normal_program = 0;
        glrcbox->filter_program = 0;
        return 0;
    }
    return -1;
}

/* TODO use VBO IBO(EBO) VAO, FBO PBO(with DMA) to instead raw glReadPixels, need api >= 19 */

static int glrcbox_do_draw_frame(GLRenderCameraBox *glrcbox, GLuint texture_id, GLboolean normal) {
    if (glrcbox) {
        glViewport(0, 0, glrcbox->frame_width, glrcbox->frame_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        GLuint program;
        if (normal) {
            program = glrcbox->normal_program;
        } else {
            program = glrcbox->filter_program;
        }
        if (!program) {
            LOGW("is glrcbox_create_shader(GLRenderCameraBox *) not called yet?\n");
            return -1;
        }
        glUseProgram(program);

        GLint position_location = glGetAttribLocation(program, "aPosition");
        GLint texture_coord_location = glGetAttribLocation(program, "aTextureCoord");
        GLint texture_location = glGetUniformLocation(program, "sTexture");

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
        glVertexAttribPointer(texture_coord_index, glrcbox_per_texture_coord_component,
                              GL_FLOAT, GL_FALSE,
                              glrcbox_per_texture_coord_component * sizeof(GLfloat),
                              glrcbox_texture_coords);

        glUniform1i(texture_index, 0);

        glDrawElements(GL_TRIANGLES, sizeof(glrcbox_draw_order) / sizeof(GLushort),
                       GL_UNSIGNED_SHORT, glrcbox_draw_order);

        glDisableVertexAttribArray(position_index);
        glDisableVertexAttribArray(texture_coord_index);

        FrameDataCallback frame_data_callback = normal ? glrcbox->normal_frame_data_callback
                                                       : glrcbox->filtered_frame_data_callback;
        if (frame_data_callback && glrcbox->pixels) {
            glReadPixels(0, 0, glrcbox->frame_width, glrcbox->frame_height,
                         GL_RGBA, GL_UNSIGNED_BYTE, glrcbox->pixels);
            /* Open GL (0,0) at lower left corner */
            if (glrcbox_upside_down(glrcbox->pixels, glrcbox->frame_width, glrcbox->frame_height,
                                    4 /* RGBA */) == -1) {
                LOGW("call glrcbox_upside_down failed!");
            }
            frame_data_callback(glrcbox);
        }
        return 0;
    }
    return -1;
}

int glrcbox_draw_frame(GLRenderCameraBox *glrcbox, GLuint texture_id) {
    if (glrcbox) {
        int normal = 0, filter = 0;
        if (glrcbox->normal_program > 0) {
            normal = glrcbox_do_draw_frame(glrcbox, texture_id, GL_TRUE);
        }
        if (glrcbox->filter_program > 0) {
            filter = glrcbox_do_draw_frame(glrcbox, texture_id, GL_FALSE);
        }
        if (normal == 0 && filter == 0) {
            return 0;
        }
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
        glrcbox_egl_error_string(eglGetError(),
                                 "eglGetDisplay failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    EGLint major, minor;
    if (!eglInitialize(glrcbox->egl_display, &major, &minor)) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglInitialize failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    } else {
        LOGI("EGL version %d-%d\n", major, minor);
    }
    EGLint num_config;
    const EGLint config_spec[] = {
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
    eglChooseConfig(glrcbox->egl_display, config_spec, &glrcbox->egl_config,
                    1, &num_config);
    if (num_config <= 0) {
        glrcbox_egl_error_string(eglGetError(),
                                 "eglChooseConfig failed:", buffer);
        LOGW("%s", buffer);
        return -1;
    }
    const EGLint context_spec[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    if ((glrcbox->egl_context = eglCreateContext(glrcbox->egl_display,
                                                 glrcbox->egl_config,
                                                 EGL_NO_CONTEXT,
                                                 context_spec)) == EGL_NO_CONTEXT) {
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
