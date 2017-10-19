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
#ifndef PROJECT_GPU_IMAGE_H
#define PROJECT_GPU_IMAGE_H

#include "base/base.h"

#include "image_type.h"

#if defined(is_macosx_os)
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#define GLEW_STATIC
#include <GL/glew.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#elif defined(is_ios_os)
#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(is_android_os)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif /* is_macosx_os */

#if defined(GL_ES_VERSION_2_0)
static const char *default_fragment_shader_source = ""
        "precision mediump float;\n"
        "varying vec2 vFragCoord;\n"
        "uniform sampler2D uTexture;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(uTexture, vFragCoord);\n"
        "}";
#else
static const char *default_fragment_shader_source = ""
        "#version 330 core\n"
        "precision mediump float;\n"
        "in vec2 vFragCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D uTexture;\n"
        "void main() {\n"
        "    fragColor = texture(uTexture, vFragCoord);\n"
        "}";
#endif

typedef struct tagGCreateFlags {
} GCreateFlags;

typedef struct tagGPrivateProcessFlags GPrivateProcessFlags;

typedef struct tagGProcessFlags {
    bool reuse_program;
    bool reuse_vao;
    bool reuse_texture;
    bool reuse_fbo;
    bool fbo_by_texture;
    GPrivateProcessFlags *private_data;
} GProcessFlags;

typedef struct tagGContext GContext;

EXPORT GContext *glbox2_create_context(PARAM_IN GCreateFlags *flag);

EXPORT int glbox2_image_process(PARAM_IN GContext *context, PARAM_IN GImage *target, PARAM_IN GImage *origin,
                                PARAM_IN const char *fragment_shader_source, PARAM_IN GProcessFlags *flag);

EXPORT void glbox2_destroy_context(PARAM_IN GContext *context, PARAM_IN GCreateFlags *flag,
                                   PARAM_IN GProcessFlags **process_flags_array, uint32_t process_flags_size);

#endif /* PROJECT_GPU_IMAGE_H */
