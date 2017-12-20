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
    Display *x_display;
    GLXContext glx_context;
    GLXPbuffer glx_pbuffer;
};

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static int prepare_off_screen_context(GlboxContext *g_context, GCreateFlags *flag) {
    if (!g_context || !g_context->native_context) {
        base_error_log("pass a GlboxContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }

    Display *display = XOpenDisplay(NULL);
    if (!display) {
        base_error_log("call XOpenDisplay failed\n");
        return -1;
    }

    const char *extensions = glXQueryExtensionsString(display, DefaultScreen(display));
    printf("glXQueryExtensionsString: %s\n", extensions);

    int nitems = 0;
    const int config_attrs[] = {
            GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
            GLX_X_RENDERABLE, True,
            GLX_DOUBLEBUFFER, True,
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT,
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            None
    };
    GLXFBConfig *configs = glXChooseFBConfig(display, DefaultScreen(display), config_attrs, &nitems);
    if (!configs || nitems <= 0) {
        base_error_log("call glXChooseFBConfig failed!\n");
        return -1;
    }

    XVisualInfo *vi = NULL;
    int best_config_index = -1, best_samples = -1;
    int sample_buffers = -1, samples = -1;
    for (int i = 0; i < nitems; ++i) {
        vi = glXGetVisualFromFBConfig(display, configs[i]);
        if (vi) {
            glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLE_BUFFERS, &sample_buffers);
            glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLES, &samples);
            if (best_config_index < 0 || (sample_buffers && samples > best_samples)) {
                best_config_index = i, best_samples = samples;
            }
        }
    }
    GLXFBConfig config = configs[best_config_index];
    /*
    vi = glXGetVisualFromFBConfig(display, config);
    // do something
    XFree(vi);
     */

    const int pbuffer_attrs[] = {
            GLX_PBUFFER_WIDTH, SURFACE_WIDTH,
            GLX_PBUFFER_HEIGHT, SURFACE_HEIGHT,
            GLX_LARGEST_PBUFFER, False,
            None
    };
    GLXPbuffer pbuffer = glXCreatePbuffer(display, config, pbuffer_attrs);

    // Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
    GLXContext context = glXCreateNewContext(display, config, GLX_RGBA_TYPE, NULL, True);
    if (!context) {
        base_error_log("call glXCreateNewContext failed\n");
        return -1;
    }

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
            (glXCreateContextAttribsARBProc) glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    glXMakeCurrent(display, 0, 0);
    glXDestroyContext(display, context);
    if (!glXCreateContextAttribsARB) {
        base_error_log("glXCreateContextAttribsARB entry point not found. Aborting.\n");
        return -1;
    }

    const int context_attrs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
    };
    context = glXCreateContextAttribsARB(display, config, NULL, True, context_attrs);
    if (!context) {
        base_error_log("call glXCreateContextAttribsARB failed, "
                               "Make sure glXQueryExtensionsString has GLX_ARB_create_context\n");
        return -1;
    }

    if (glXMakeCurrent(display, pbuffer, context) == False) {
        base_error_log("call glXMakeCurrent failed\n");
        return -1;
    }

    XFree(configs);
    XSync(display, False);

    glewExperimental = GL_TRUE;
    if (glewInit()) {
        base_error_log("call glewInit failed\n");
        return -1;
    }

    base_info_log("glXIsDirect: %d\n", glXIsDirect(display, context));
    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *shader_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
    base_info_log("GPU Image Internal: version: '%s'; vendor: '%s'; renderer: '%s'; shader-version: '%s'\n",
                  version, vendor, renderer, shader_version);
    g_context->native_context->x_display = display;
    g_context->native_context->glx_context = context;
    g_context->native_context->glx_pbuffer = pbuffer;

    return 0;
}

static int mark_current_off_screen_context(GlboxContext *context) {
    if (context && context->native_context && context->native_context->x_display) {
        if (glXMakeCurrent(context->native_context->x_display,
                           context->native_context->glx_pbuffer,
                           context->native_context->glx_context) == False) {
            base_error_log("call glXMakeCurrent failed\n");
            return -1;
        } else {
            return 0;
        }
    } else {
        base_error_log("mark_current_off_screen_context: "
                               "context == NULL || context->glx_context == NULL\n");
        return -1;
    }
}

static void release_off_screen_context(GlboxContext *context) {
    if (context && context->native_context && context->native_context->x_display) {
        glXMakeCurrent(context->native_context->x_display, 0, 0);
        glXDestroyPbuffer(context->native_context->x_display, context->native_context->glx_pbuffer);
        glXDestroyContext(context->native_context->x_display, context->native_context->glx_context);
        XCloseDisplay(context->native_context->x_display);
        context->native_context->x_display = NULL;
    }
}
