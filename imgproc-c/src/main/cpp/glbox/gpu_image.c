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
 * @author John Kenrinus Lee
 * @version 2017-09-25
 */
#include "gpu_image.h"

/*
 * TODO * A exception happened when call image process many times:
 * TODO [following up] glGetError return code 1280, line at 175;
 * TODO [following up] call glbox2_prepare_texture failed;
 */

struct tagGPrivateProcessFlags {
    GLuint program;
    GLuint vao;
    GLuint texture_id_unit_pair[2];
    GLuint fbo;
    GLuint vbo; /* for release */
    GLuint ibo; /* for release */
    GLuint color_attachment; /* for release */
    GLuint depth_stencil_attachment; /* for release */
};

typedef struct tagInternalGContext InternalGContext;

struct tagGContext {
    InternalGContext *native_context;
    GLuint texture_index;
};

#if defined(is_macosx_os)
#include "glbox/macosx/gpu_image_internal.h"
#elif defined(is_ios_os)
#include "glbox/ios/gpu_image_internal.h"
#elif defined(is_android_os)
#include "glbox/android/gpu_image_internal.h"
#endif /* is_macosx_os */

static GLfloat pos_coord[] = {
        /* Vertex Positions */  /* Texture Coordinates */
        -1.0f,  1.0f,           0.0f, 1.0f, /* Left Top */
         1.0f,  1.0f,           1.0f, 1.0f, /* Right Top */
         1.0f, -1.0f,           1.0f, 0.0f, /* Right Bottom */
        -1.0f, -1.0f,           0.0f, 0.0f, /* Left Bottom */
};
static GLushort element_index[] = { /* Note that we start from 0 with GL_CCW! */
        0, 3, 2, /* First Triangle */
        0, 2, 1, /* Second Triangle */
};
#if defined(GL_ES_VERSION_2_0)
static const char *vertex_shader_source = ""
        "precision mediump float;\n"
        "attribute vec2 vertexPosition;\n"
        "attribute vec2 textureCoordinate;\n"
        "varying vec2 vFragCoord;\n"
        "void main() {\n"
        "    vFragCoord = textureCoordinate;\n"
        "    gl_Position = vec4(vertexPosition, 0.0, 1.0);\n"
        "}";
#else
static const char *vertex_shader_source = ""
        "#version 330 core\n"
        "precision mediump float;\n"
        "in vec2 vertexPosition;\n"
        "in vec2 textureCoordinate;\n"
        "out vec2 vFragCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(vertexPosition, 0.0, 1.0);\n"
        "    vFragCoord = textureCoordinate;\n"
        "}";
#endif

#define CHECK_ERROR(code_enum) \
if ((code_enum = glGetError()) != GL_NO_ERROR) { \
    base_error_log("glGetError return code %d, line at %d\n", code_enum, __LINE__); \
    return -1; \
}

#define GET_TEXTURE_INDEX(g_context) \
g_context->texture_index++

int glbox2_compile_link_shader(const char *vs_source, const char *fs_source, GLuint *out_program) {
    GLint success;

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(vs, sizeof(info_log), NULL, info_log);
        base_error_log("ERROR::SHADER::VERTEX::COMPILATION_FAILED (Vertex) -> %s\n", info_log);
        return -1;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(fs, sizeof(info_log), NULL, info_log);
        base_error_log("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED (Fragment) -> %s\n", info_log);
        return -1;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar info_log[512];
        glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
        base_error_log("ERROR::SHADER::PROGRAM::LINKING_FAILED -> %s\n", info_log);
        return -1;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    *out_program = program;
    return 0;
}

int glbox2_gen_vao_bind_attrib(GLuint program, GLuint *out_vao, GLuint *out_vbo, GLuint *out_ibo) {
    GLuint vao, vbo, ibo;
    GLuint loc_attr;
    GLenum code;

    glGenVertexArrays(1, &vao); CHECK_ERROR(code);
    glGenBuffers(1, &vbo); CHECK_ERROR(code);
    glGenBuffers(1, &ibo); CHECK_ERROR(code);

    glBindVertexArray(vao); CHECK_ERROR(code);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); CHECK_ERROR(code);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); CHECK_ERROR(code);

    glBufferData(GL_ARRAY_BUFFER, sizeof(pos_coord), pos_coord, GL_STATIC_DRAW); CHECK_ERROR(code);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element_index), element_index, GL_STATIC_DRAW); CHECK_ERROR(code);

    loc_attr = _static_cast(GLuint) glGetAttribLocation(program, "vertexPosition"); CHECK_ERROR(code);
    glVertexAttribPointer(loc_attr, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GLfloat), _static_cast(GLvoid*) (0 * 2 * sizeof(GLfloat))); CHECK_ERROR(code);
    glEnableVertexAttribArray(loc_attr); CHECK_ERROR(code);

    loc_attr = (GLuint) glGetAttribLocation(program, "textureCoordinate"); CHECK_ERROR(code);
    glVertexAttribPointer(loc_attr, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GLfloat), _static_cast(GLvoid*) (1 * 2 * sizeof(GLfloat))); CHECK_ERROR(code);
    glEnableVertexAttribArray(loc_attr); CHECK_ERROR(code);

    glBindVertexArray(0); CHECK_ERROR(code);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); CHECK_ERROR(code);
    glBindBuffer(GL_ARRAY_BUFFER, 0); CHECK_ERROR(code);

    *out_vao = vao;
    *out_vbo = vbo;
    *out_ibo = ibo;
    return 0;
}

int glbox2_prepare_texture(GLuint texture_unit, GLuint *out_texture_id) {
    GLuint texture_id;
    GLenum code;
    glActiveTexture(GL_TEXTURE0 + texture_unit); CHECK_ERROR(code);
    glGenTextures(1, &texture_id); CHECK_ERROR(code);
    glBindTexture(GL_TEXTURE_2D, texture_id); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_ERROR(code);
    /* glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // Removed from GL 3.1 and above */
    glBindTexture(GL_TEXTURE_2D, 0); CHECK_ERROR(code);
    *out_texture_id = texture_id;
    return 0;
}

int glbox2_update_texture(GLuint texture_id, GImage *image) {
    GLenum code;
    glBindTexture(GL_TEXTURE_2D, texture_id); CHECK_ERROR(code);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); CHECK_ERROR(code);
    GLenum format = image->channels == 3 ? GL_RGB : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0,
                 format, GL_UNSIGNED_BYTE, image->image); CHECK_ERROR(code);
    glGenerateMipmap(GL_TEXTURE_2D); CHECK_ERROR(code);
    glBindTexture(GL_TEXTURE_2D, 0); CHECK_ERROR(code);
    return 0;
}

int glbox2_prepare_empty_texture(bool color, GImage *image, GLuint texture_unit, GLuint *out_texture_id) {
    GLuint texture_id;
    GLenum code;
    glActiveTexture(GL_TEXTURE0 + texture_unit); CHECK_ERROR(code);
    glGenTextures(1, &texture_id); CHECK_ERROR(code);
    glBindTexture(GL_TEXTURE_2D, texture_id); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); CHECK_ERROR(code);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); CHECK_ERROR(code);
    /* glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // Removed from GL 3.1 and above */
    if (color) {
#if defined(is_ios_os)
        GLenum internal_format = image->channels == 3 ? GL_RGB8 : GL_RGBA8;
#else
        GLenum internal_format = image->channels == 3 ? GL_RGB : GL_RGBA;
#endif
        GLenum format = image->channels == 3 ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image->width, image->height, 0,
                     format, GL_UNSIGNED_BYTE, NULL); CHECK_ERROR(code);
    } else {
#if defined(is_ios_os)
        GLenum internal_format = GL_DEPTH24_STENCIL8;
#else
        GLenum internal_format = GL_DEPTH_STENCIL;
#endif
        GLenum format = GL_DEPTH_STENCIL;
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image->width, image->height, 0,
                     format, GL_UNSIGNED_INT_24_8, NULL); CHECK_ERROR(code);
    }
    glBindTexture(GL_TEXTURE_2D, 0); CHECK_ERROR(code);
    *out_texture_id = texture_id;
    return 0;
}

int glbox2_gen_fbo_bind_attachment(bool fbo_by_texture, GImage *image, GContext *context,
                                   GLuint *out_fbo, GLuint *out_color, GLuint *out_depth_stencil) {
    GLuint fbo;
    GLuint color_attachment, depth_stencil_attachment;
    GLenum code;

    glGenFramebuffers(1, &fbo); CHECK_ERROR(code);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); CHECK_ERROR(code);

    if (fbo_by_texture) {
        if (glbox2_prepare_empty_texture(true, image, GET_TEXTURE_INDEX(context),
                                         &color_attachment) < 0) {
            base_error_log("call glbox2_prepare_empty_texture failed\n");
            return -1;
        }
        if (glbox2_prepare_empty_texture(false, image, GET_TEXTURE_INDEX(context),
                                         &depth_stencil_attachment) < 0) {
            base_error_log("call glbox2_prepare_empty_texture failed\n");
            return -1;
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, color_attachment, 0); CHECK_ERROR(code);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, depth_stencil_attachment, 0); CHECK_ERROR(code);
    } else {
#if defined(is_ios_os)
        GLenum color_format = image->channels == 3 ? GL_RGB8 : GL_RGBA8;
        GLenum depth_stencil_format = GL_DEPTH24_STENCIL8;
#else
        GLenum color_format = image->channels == 3 ? GL_RGB : GL_RGBA;
        GLenum depth_stencil_format = GL_DEPTH_STENCIL;
#endif
        glGenRenderbuffers(1, &color_attachment); CHECK_ERROR(code);
        glBindRenderbuffer(GL_RENDERBUFFER, color_attachment); CHECK_ERROR(code);
        glRenderbufferStorage(GL_RENDERBUFFER, color_format, image->width, image->height); CHECK_ERROR(code);
        glBindRenderbuffer(GL_RENDERBUFFER, 0); CHECK_ERROR(code);

        glGenRenderbuffers(1, &depth_stencil_attachment); CHECK_ERROR(code);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_attachment); CHECK_ERROR(code);
        glRenderbufferStorage(GL_RENDERBUFFER, depth_stencil_format, image->width, image->height); CHECK_ERROR(code);
        glBindRenderbuffer(GL_RENDERBUFFER, 0); CHECK_ERROR(code);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, color_attachment); CHECK_ERROR(code);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, depth_stencil_attachment); CHECK_ERROR(code);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        base_error_log("glCheckFramebufferStatus is %d not GL_FRAMEBUFFER_COMPLETE\n", status);
        return -1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); CHECK_ERROR(code);

    *out_fbo = fbo;
    *out_color = color_attachment;
    *out_depth_stencil = depth_stencil_attachment;
    return 0;
}

int glbox2_gpu_render(GLuint program, GLuint texture_unit, GLuint texture_id, GLuint vao,
                      GLsizei viewport_width, GLsizei viewport_height) {
    GLenum code;

    glUseProgram(program); CHECK_ERROR(code);

    GLint loc_uniform = glGetUniformLocation(program, "uTexture"); CHECK_ERROR(code);
    glUniform1i(loc_uniform, texture_unit); CHECK_ERROR(code);

    glViewport(0, 0, viewport_width, viewport_height); CHECK_ERROR(code);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); CHECK_ERROR(code);
    glClearDepthf(1.0f); CHECK_ERROR(code);
    glClearStencil(0); CHECK_ERROR(code);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); CHECK_ERROR(code);
    glCullFace(GL_BACK); CHECK_ERROR(code);
    glFrontFace(GL_CCW); CHECK_ERROR(code);
    glEnable(GL_CULL_FACE); CHECK_ERROR(code);
    glEnable(GL_DEPTH_TEST); CHECK_ERROR(code);
    glEnable(GL_STENCIL_TEST); CHECK_ERROR(code);

    glActiveTexture(GL_TEXTURE0 + texture_unit); CHECK_ERROR(code);

    glBindVertexArray(vao); CHECK_ERROR(code);
    glBindTexture(GL_TEXTURE_2D, texture_id); CHECK_ERROR(code);
    glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_SHORT, 0); CHECK_ERROR(code);
    glBindTexture(GL_TEXTURE_2D, 0); CHECK_ERROR(code);
    glBindVertexArray(0); CHECK_ERROR(code);

    glUseProgram(0); CHECK_ERROR(code);

    return 0;
}

int glbox2_read_pixels(GImage *image) {
    GLenum code;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); CHECK_ERROR(code);
    glReadPixels(0, 0, image->width, image->height, (image->channels == 3 ? GL_RGB : GL_RGBA),
                 GL_UNSIGNED_BYTE, image->image); CHECK_ERROR(code);
    return 0;
}

int glbox2_do_image_process(GContext *context, GImage *target, GImage *origin,
                            const char *fragment_shader_source, GProcessFlags *flag) {
    if (mark_current_off_screen_context(context) < 0) {
        base_error_log("call mark_current_off_screen_context failed\n");
        return -1;
    }

    if (!flag->private_data) {
        ASSIGN_HEAP_TYPE_POINTER(GPrivateProcessFlags, flag->private_data, {
            base_error_log("malloc GPrivateProcessFlags failed\n");
            return -1;
        });
    }

    GLuint program;
    if (flag->reuse_program && flag->private_data->program > 0) {
        program = flag->private_data->program;
    } else {
        if (glbox2_compile_link_shader(vertex_shader_source, fragment_shader_source,
                                &program) < 0) {
            base_error_log("call glbox2_compile_link_shader failed\n");
            return -1;
        }
        if (flag->reuse_program) {
            flag->private_data->program = program;
        }
    }

    GLuint vao, vbo, ibo;
    if (flag->reuse_vao && flag->private_data->vao > 0) {
        vao = flag->private_data->vao;
    } else {
        if (glbox2_gen_vao_bind_attrib(program, &vao, &vbo, &ibo) < 0) {
            base_error_log("call glbox2_gen_vao_bind_attrib failed\n");
            return -1;
        }
        if (flag->reuse_vao) {
            flag->private_data->vao = vao;
            flag->private_data->vbo = vbo;
            flag->private_data->ibo = ibo;
        }
    }

    GLuint texture_id, texture_unit;
    if (flag->reuse_texture && flag->private_data->texture_id_unit_pair[0] > 0) {
        texture_id = flag->private_data->texture_id_unit_pair[0];
        texture_unit = flag->private_data->texture_id_unit_pair[1];
    } else {
        texture_unit = GET_TEXTURE_INDEX(context);
        if (glbox2_prepare_texture(texture_unit, &texture_id) < 0) {
            base_error_log("call glbox2_prepare_texture failed\n");
            return -1;
        }
        if (flag->reuse_texture) {
            flag->private_data->texture_id_unit_pair[0] = texture_id;
            flag->private_data->texture_id_unit_pair[1] = texture_unit;
        }
    }

    if (glbox2_update_texture(texture_id, origin) < 0) {
        base_error_log("call glbox2_update_texture failed\n");
        return -1;
    }

    GLuint fbo;
    GLuint color_attachment, depth_stencil_attachment;
    if (flag->reuse_fbo && flag->private_data->fbo > 0) {
        fbo = flag->private_data->fbo;
    } else {
        if (glbox2_gen_fbo_bind_attachment(flag->fbo_by_texture, target, context,
                                           &fbo, &color_attachment, &depth_stencil_attachment) < 0) {
            base_error_log("call glbox2_gen_fbo_bind_attachment failed\n");
            return -1;
        }
        if (flag->reuse_fbo) {
            flag->private_data->fbo = fbo;
            flag->private_data->color_attachment = color_attachment;
            flag->private_data->depth_stencil_attachment = depth_stencil_attachment;
        }
    }

    GLenum code;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo); CHECK_ERROR(code);

    if (glbox2_gpu_render(program, texture_unit, texture_id, vao, target->width, target->height) < 0) {
        base_error_log("call glbox2_gpu_render failed\n");
        return -1;
    }

    if (glbox2_read_pixels(target) < 0) {
        base_error_log("call glbox2_read_pixels failed\n");
        return -1;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); CHECK_ERROR(code);
    return 0;
}

EXPORT int glbox2_image_process(PARAM_IN GContext *context, PARAM_IN GImage *target, PARAM_IN GImage *origin,
                                PARAM_IN const char *fragment_shader_source, PARAM_IN GProcessFlags *flag) {
    if (!target) {
        base_error_log("target is NULL\n");
        return -1;
    }
    if (!origin) {
        base_error_log("origin is NULL\n");
        return -1;
    }
    if (!fragment_shader_source) {
        base_error_log("fragment shader source is NULL\n");
        return -1;
    }
    if (!flag) {
        base_error_log("process flag is NULL\n");
        return -1;
    }
    if (!target->image) {
        base_error_log("target's image is NULL\n");
        return -1;
    }
    if (target->width <= 0 || target->height <= 0) {
        base_error_log("target's width or target's height is invalid\n");
        return -1;
    }
    if (target->channels != 1 && target->channels != 3 && target->channels != 4) {
        base_error_log("target's channels is not supported\n");
        return -1;
    }
    if (!origin->image) {
        base_error_log("origin's image is NULL\n");
        return -1;
    }
    if (origin->width <= 0 || origin->height <= 0) {
        base_error_log("origin's width or origin's height is invalid\n");
        return -1;
    }
    if (origin->channels != 1 && origin->channels != 3 && origin->channels != 4) {
        base_error_log("origin's channels is not supported\n");
        return -1;
    }
    return glbox2_do_image_process(context, target, origin, fragment_shader_source, flag);
}

EXPORT GContext *glbox2_create_context(PARAM_IN GCreateFlags *flag) {
    DEFINE_HEAP_TYPE_POINTER(GContext, context, {
        base_error_log("malloc GContext failed\n");
        return NULL;
    });

    DEFINE_HEAP_TYPE_POINTER(InternalGContext, native_context, {
        base_error_log("malloc InternalGContext failed\n");
        return NULL;
    });

    context->native_context = native_context;

    if (prepare_off_screen_context(context, flag) < 0) {
        base_error_log("call prepare_off_screen_context failed\n");
        return NULL;
    }
    return context;
}

EXPORT void glbox2_destroy_context(PARAM_IN GContext *context, PARAM_IN GCreateFlags *flag,
                                   PARAM_IN GProcessFlags **process_flags_array, uint32_t process_flags_size) {
    if (process_flags_array && process_flags_size > 0) {
        GProcessFlags *process_flags;
        for (uint32_t i = 0; i < process_flags_size; ++i) {
            process_flags = process_flags_array[i];
            if (process_flags && process_flags->private_data) {
                if (process_flags->reuse_program && process_flags->private_data->program > 0) {
                    glDeleteProgram(process_flags->private_data->program);
                }
                if (process_flags->reuse_vao && process_flags->private_data->vao > 0) {
                    glDeleteVertexArrays(1, &process_flags->private_data->vao);
                    glDeleteBuffers(1, &process_flags->private_data->vbo);
                    glDeleteBuffers(1, &process_flags->private_data->ibo);
                }
                if (process_flags->reuse_fbo && process_flags->private_data->fbo > 0) {
                    glDeleteFramebuffers(1, &process_flags->private_data->fbo);
                    if (process_flags->fbo_by_texture) {
                        glDeleteTextures(1, &process_flags->private_data->color_attachment);
                        glDeleteTextures(1, &process_flags->private_data->depth_stencil_attachment);
                    } else {
                        glDeleteRenderbuffers(1, &process_flags->private_data->color_attachment);
                        glDeleteRenderbuffers(1, &process_flags->private_data->depth_stencil_attachment);
                    }
                }
                if (process_flags->reuse_texture && process_flags->private_data->texture_id_unit_pair[0] > 0) {
                    glDeleteTextures(1, &process_flags->private_data->texture_id_unit_pair[0]);
                }
                FREE_POINTER(process_flags->private_data);
            }
        }
    }
    if (context) {
        release_off_screen_context(context);
        FREE_POINTER(context->native_context);
        FREE_POINTER(context);
    }
}
