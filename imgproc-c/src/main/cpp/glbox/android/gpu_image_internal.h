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
 * NOTICE: OpenGL need run on single thread!
 * @author John Kenrinus Lee
 * @version 2017-09-25
 */
#include "base/base.h"
#include "../gpu_image.h"

#if !defined(SURFACE_WIDTH)
#define SURFACE_WIDTH 1280
#endif
#if !defined(SURFACE_HEIGHT)
#define SURFACE_HEIGHT 960
#endif

struct tagInternalGContext {
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
};

static void egl_error_string(GLint error, const char *prefix, char *buffer) {
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

static int prepare_off_screen_context(GlboxContext *g_context, GCreateFlags *flag) {
    if (!g_context || !g_context->native_context) {
        base_error_log("pass a GlboxContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }

    char buffer[256];
    memset(buffer, 0, 256);

    if ((g_context->native_context->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY))
        == EGL_NO_DISPLAY) {
        egl_error_string(eglGetError(), "eglGetDisplay failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }

    EGLint major, minor;
    if (!eglInitialize(g_context->native_context->egl_display, &major, &minor)) {
        egl_error_string(eglGetError(), "eglInitialize failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    } else {
        base_info_log("EGL version %d-%d\n", major, minor);
    }

    EGLint num_config;
    const EGLint config_spec[] = {
            EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_LUMINANCE_SIZE, 0,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_ALPHA_MASK_SIZE, 0,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_NONE
    };
    if (!eglChooseConfig(g_context->native_context->egl_display, config_spec,
                    &g_context->native_context->egl_config, 1, &num_config)) {
        egl_error_string(eglGetError(), "eglChooseConfig failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }
    if (num_config <= 0) {
        egl_error_string(eglGetError(), "eglChooseConfig failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }

    const EGLint context_spec[] = {
            EGL_CONTEXT_CLIENT_VERSION, 3,
            EGL_NONE
    };
    if ((g_context->native_context->egl_context
                 = eglCreateContext(g_context->native_context->egl_display,
                                    g_context->native_context->egl_config,
                                    EGL_NO_CONTEXT,
                                    context_spec)) == EGL_NO_CONTEXT) {
        egl_error_string(eglGetError(), "eglCreateContext failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }

    EGLint surface_width = SURFACE_WIDTH, surface_height = SURFACE_HEIGHT;

    if (flag) { // custom surface width and surface height

    }

    const EGLint pbuffer_spec[] = {
                    EGL_WIDTH,  surface_width,
                    EGL_HEIGHT, surface_height,
                    EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
                    EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
                    EGL_LARGEST_PBUFFER, EGL_TRUE,
                    EGL_MIPMAP_TEXTURE, EGL_TRUE,
                    EGL_NONE
    };

    if ((g_context->native_context->egl_surface
                 = eglCreatePbufferSurface(g_context->native_context->egl_display,
                                           g_context->native_context->egl_config,
                                           pbuffer_spec)) == EGL_NO_SURFACE) {
        egl_error_string(eglGetError(), "eglCreatePbufferSurface failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }

    if (!eglMakeCurrent(g_context->native_context->egl_display,
                        g_context->native_context->egl_surface,
                        g_context->native_context->egl_surface,
                        g_context->native_context->egl_context)) {
        egl_error_string(eglGetError(), "eglMakeCurrent failed:", buffer);
        base_error_log("%s", buffer);
        return -1;
    }

    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    base_info_log("GPU Image Internal: version: '%s'; vendor: '%s'; renderer: '%s'\n",
                  version, vendor, renderer);
    int major_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    if (major_version < 3) {
        return -1;
    }
    return 0;
}

static int mark_current_off_screen_context(GlboxContext *context) {
    if (!context || !context->native_context) {
        base_error_log("pass a GlboxContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }
    if (context->native_context->egl_context != NULL
            && context->native_context->egl_context != EGL_NO_CONTEXT) {
        char buffer[256];
        if (!eglMakeCurrent(context->native_context->egl_display,
                            context->native_context->egl_surface,
                            context->native_context->egl_surface,
                            context->native_context->egl_context)) {
            egl_error_string(eglGetError(), "eglMakeCurrent failed:", buffer);
            base_error_log("%s", buffer);
            return -1;
        } else {
            return 0;
        }
    } else {
        base_error_log("mark_current_off_screen_context: "
                               "egl_context == NULL || egl_context == EGL_NO_CONTEXT\n");
        return -1;
    }
}

static void release_off_screen_context(GlboxContext *context) {
    if (!context || !context->native_context) {
        base_error_log("pass a GlboxContext pointer to NULL or its native_context is NULL\n");
    } else {
        if (context->native_context->egl_context != NULL
            && context->native_context->egl_context != EGL_NO_CONTEXT) {
            eglMakeCurrent(context->native_context->egl_display,
                           EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroySurface(context->native_context->egl_display,
                              context->native_context->egl_surface);
            eglDestroyContext(context->native_context->egl_display,
                              context->native_context->egl_context);
            eglTerminate(context->native_context->egl_display);
            context->native_context->egl_display = EGL_NO_DISPLAY;
            context->native_context->egl_context = EGL_NO_CONTEXT;
            context->native_context->egl_surface = EGL_NO_SURFACE;
        }
    }
}
