#ifndef PROJECT_GPU_IMAGE_H
#define PROJECT_GPU_IMAGE_H

#include "base/base.h"

#include "image_type.h"

#ifdef is_macosx_os
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#define GLEW_STATIC
#include <GL/glew.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif /* is_macosx_os */

static const char *default_fragment_shader_source = ""
        "#version 330 core\n"
        "precision mediump float;\n"
        "in vec2 vFragCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D uTexture;\n"
        "void main() {\n"
        "    fragColor = texture(uTexture, vFragCoord);\n"
        "}";

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
