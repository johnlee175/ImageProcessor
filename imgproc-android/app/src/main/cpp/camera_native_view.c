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
 * camera native view implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "camera_native_view.h"

#ifndef CNV_CONSTANT_DEFINED
#define CNV_CONSTANT_DEFINED

const GLfloat cnv_vertex_coords[] = {
        -1.0f, 1.0f, 0.0f, /* top left */
        -1.0f, -1.0f, 0.0f, /* bottom left */
        1.0f, -1.0f, 0.0f, /* bottom right */
        1.0f, 1.0f, 0.0f, /* top right */
};

/* texture coordinate in android OpenGL es:
   is not from bottom left to top right, just from top left to bottom right! */
const GLfloat cnv_back_texture_coords[] = { /* rotate 0 degree */
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
};

/* texture coordinate in android OpenGL es:
   is not from bottom left to top right, just from top left to bottom right! */
const GLfloat cnv_front_texture_coords[] = { /* rotate 180 degree */
        1.0f, 1.0f, /* bottom right */
        1.0f, 0.0f, /* top right */
        0.0f, 0.0f, /* top left */
        0.0f, 1.0f, /* bottom left */
};

const GLushort cnv_draw_order[] = { 0, 1, 2, 0, 2, 3 };

const GLuint cnv_per_vertex_coord_component = 3;
const GLuint cnv_per_texture_coord_component = 2;

const GLchar *cnv_vertex_shader_source = ""
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTextureCoord;\n"
        "varying vec2 vTextureCoord;\n"
        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTextureCoord = aTextureCoord;\n"
        "}";
const GLchar *cnv_fragment_shader_source = ""
        "#extension GL_OES_EGL_image_external : require\n"
        "precision mediump float;\n"
        "varying vec2 vTextureCoord;\n"
        "uniform samplerExternalOES sTexture;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
        "}";

#endif /* CNV_CONSTANT_DEFINED */

struct tagCameraNativeViewStruct {
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
    cnv_frame_data_callback frame_data_callback;
    void *user_tag_data;
};

CameraNativeViewStruct *cnv_create_initialize() {
    CameraNativeViewStruct *cnv = _static_cast(CameraNativeViewStruct *) malloc(sizeof(CameraNativeViewStruct));
    if (!cnv) {
        LOGW("malloc CameraNativeViewStruct failed");
        return NULL;
    }
    memset(cnv, 0, sizeof(CameraNativeViewStruct));
    return cnv;
}

void cnv_destroy_release(CameraNativeViewStruct *cnv) {
    if (cnv) {
        if (cnv->pixels) {
            free(cnv->pixels);
            cnv->pixels = NULL;
        }
        free(cnv);
    }
}

int cnv_set_user_tag(CameraNativeViewStruct *cnv, void *data) {
    if (cnv) {
        cnv->user_tag_data = data;
        return 0;
    }
    return -1;
}

void *cnv_get_user_tag(CameraNativeViewStruct *cnv) {
    if (cnv) {
        return cnv->user_tag_data;
    }
    return NULL;
}

int cnv_set_frame_data_callback(CameraNativeViewStruct *cnv, cnv_frame_data_callback callback_func) {
    if (cnv) {
        cnv->frame_data_callback = callback_func;
        return 0;
    }
    return -1;
}

int cnv_set_frame_size(CameraNativeViewStruct *cnv, GLuint frame_width, GLuint frame_height) {
    if (cnv) {
        cnv->frame_width = frame_width;
        cnv->frame_height = frame_height;
        cnv->pixels_size = sizeof(GLubyte) * frame_width * frame_height;
        GLubyte *pixels = _static_cast(GLubyte *) malloc(cnv->pixels_size);
        if (!pixels) {
            LOGW("malloc (frame_width * frame_height)'s GLubyte for CameraNativeViewStruct.pixels failed");
            return -1;
        }
        cnv->pixels = pixels;
        return 0;
    }
    return -1;
}

int cnv_set_front_camera(CameraNativeViewStruct *cnv, GLboolean is_front_camera) {
    if (cnv) {
        cnv->is_front_camera = is_front_camera;
        return 0;
    }
    return -1;
}

int cnv_set_shader_source(CameraNativeViewStruct *cnv, const GLchar *vertex_source, const GLchar *fragment_source) {
    if (cnv) {
        if (vertex_source) {
            cnv->vertex_shader_source = vertex_source;
        }
        if (fragment_source) {
            cnv->fragment_shader_source = fragment_source;
        }
        return 0;
    }
    return -1;
}

int cnv_get_pixels(CameraNativeViewStruct *cnv, GLubyte **pixels, size_t *pixels_size) {
    if (cnv && cnv->pixels && cnv->pixels_size) {
        *pixels = cnv->pixels;
        *pixels_size = cnv->pixels_size;
        return 0;
    }
    return -1;
}

int cnv_create_shader(CameraNativeViewStruct *cnv) {
    if (cnv) {
        if (!cnv->vertex_shader_source) {
            cnv->vertex_shader_source = cnv_vertex_shader_source;
        }
        if (!cnv->fragment_shader_source) {
            cnv->fragment_shader_source = cnv_fragment_shader_source;
        }
        if ((cnv->vertex_shader = glCreateShader(GL_VERTEX_SHADER)) == GL_FALSE) {
            LOGW("error occurs in glCreateShader()");
            return -1;
        }
        if ((cnv->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)) == GL_FALSE) {
            LOGW("error occurs in glCreateShader()");
            return -1;
        }
        glShaderSource(cnv->vertex_shader, 1, &cnv->vertex_shader_source, NULL);
        glShaderSource(cnv->fragment_shader, 1, &cnv->fragment_shader_source, NULL);
        GLint status, log_length;
        glCompileShader(cnv->vertex_shader);
        glGetShaderiv(cnv->vertex_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(cnv->vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(cnv->vertex_shader, sizeof(info_log), NULL, info_log);
            LOGW("vertex shader compile failed: %s\n", info_log);
            return -1;
        }
        glCompileShader(cnv->fragment_shader);
        glGetShaderiv(cnv->fragment_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            glGetShaderiv(cnv->fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetShaderInfoLog(cnv->fragment_shader, sizeof(info_log), NULL, info_log);
            LOGW("fragment shader compile failed: %s\n", info_log);
            return -1;
        }
        if ((cnv->program = glCreateProgram()) == GL_FALSE) {
            LOGW("error occurs in glCreateProgram()");
            return -1;
        }
        glAttachShader(cnv->program, cnv->vertex_shader);
        glAttachShader(cnv->program, cnv->fragment_shader);
        glLinkProgram(cnv->program);
        glGetProgramiv(cnv->program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            glGetProgramiv(cnv->program, GL_INFO_LOG_LENGTH, &log_length);
            GLchar info_log[log_length];
            glGetProgramInfoLog(cnv->program, sizeof(info_log), NULL, info_log);
            LOGW("program link failed: %s\n", info_log);
            return -1;
        }
        return 0;
    }
    return -1;
}

int cnv_destroy_shader(CameraNativeViewStruct *cnv) {
    if (cnv) {
        glDeleteShader(cnv->vertex_shader);
        glDeleteShader(cnv->fragment_shader);
        glDeleteProgram(cnv->program);
        cnv->vertex_shader = 0;
        cnv->fragment_shader = 0;
        cnv->program = 0;
        return 0;
    }
    return -1;
}

int cnv_draw_frame(CameraNativeViewStruct *cnv, GLuint texture_id) {
    if (cnv) {
        glViewport(0, 0, cnv->frame_width, cnv->frame_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!cnv->program) {
            LOGW("is cnv_create_shader(CameraNativeViewStruct *) not called yet?");
            return -1;
        }

        glUseProgram(cnv->program);

        GLint position_location = glGetAttribLocation(cnv->program, "aPosition");
        GLint texture_coord_location = glGetAttribLocation(cnv->program, "aTextureCoord");
        GLint texture_location = glGetUniformLocation(cnv->program, "sTexture");

        if (position_location < 0 || texture_coord_location < 0 || texture_location < 0) {
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
        glVertexAttribPointer(position_index, cnv_per_vertex_coord_component, GL_FLOAT, GL_FALSE,
                              cnv_per_vertex_coord_component * sizeof(GLfloat), cnv_vertex_coords);

        glEnableVertexAttribArray(texture_coord_index);
        if (cnv->is_front_camera == GL_FALSE) {
            glVertexAttribPointer(texture_coord_index, cnv_per_texture_coord_component, GL_FLOAT, GL_FALSE,
                                  cnv_per_texture_coord_component * sizeof(GLfloat), cnv_back_texture_coords);
        } else {
            glVertexAttribPointer(texture_coord_index, cnv_per_texture_coord_component, GL_FLOAT, GL_FALSE,
                                  cnv_per_texture_coord_component * sizeof(GLfloat), cnv_front_texture_coords);
        }

        glUniform1i(texture_index, 0);


        glDrawElements(GL_TRIANGLES, sizeof(cnv_draw_order) / sizeof(GLushort), GL_UNSIGNED_SHORT, cnv_draw_order);

        glDisableVertexAttribArray(position_index);
        glDisableVertexAttribArray(texture_coord_index);

        if (cnv->frame_data_callback && cnv->pixels) {
            glReadPixels(0, 0, cnv->frame_width, cnv->frame_height,
                         GL_RGBA, GL_UNSIGNED_BYTE, cnv->pixels);
            cnv->frame_data_callback(cnv);
        }

        return 0;
    }
    return -1;
}