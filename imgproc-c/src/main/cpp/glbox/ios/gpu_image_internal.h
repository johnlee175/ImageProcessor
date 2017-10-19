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

struct tagInternalGContext {
    __unsafe_unretained EAGLContext *eagl_context;
};

static int prepare_off_screen_context(GContext *g_context, GCreateFlags *flag) {
    if (!g_context || !g_context->native_context) {
        base_error_log("pass a GContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }
    
    EAGLContext* eagl_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    g_context->native_context->eagl_context = eagl_context;

    if (![EAGLContext setCurrentContext:g_context->native_context->eagl_context]) {
        base_error_log("mark_current_off_screen_context: setCurrentContext failed\n");
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

static int mark_current_off_screen_context(GContext *context) {
    if (!context || !context->native_context) {
        base_error_log("pass a GContext pointer to NULL or its native_context is NULL\n");
        return -1;
    }
    if (context->native_context->eagl_context != nil) {
        if (![EAGLContext setCurrentContext:context->native_context->eagl_context]) {
            base_error_log("mark_current_off_screen_context: setCurrentContext failed\n");
            return -1;
        } else {
            return 0;
        }
    } else {
        base_error_log("mark_current_off_screen_context: "
                               "context == NULL || context->context_obj == NULL\n");
        return -1;
    }
}

static void release_off_screen_context(GContext *context) {
    if (!context || !context->native_context) {
        base_error_log("pass a GContext pointer to NULL or its native_context is NULL\n");
    }
    context->native_context->eagl_context = nil;
    if (![EAGLContext setCurrentContext:nil]) {
        base_error_log("mark_current_off_screen_context: setCurrentContext to nil failed\n");
    }
}
