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
 * GL client render thread interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_GLCLIENT_RENDER_THREAD_H
#define IMGPROC_ANDROID_GLCLIENT_RENDER_THREAD_H

#include "base.h"
#include <pthread.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct tagGLClientRenderThreadStruct GLClientRenderThreadStruct;

typedef struct tagBoolProperty {
    void *user_tag_data;
    bool (*get_prop)(void *user_tag_data);
    void (*set_prop)(void *user_tag_data, bool prop);
} BoolProperty;

typedef struct tagDrawer {
    void *user_tag_data;
    void (*create)(void *user_tag_data);
    void (*destroy)(void *user_tag_data);
    void (*draw)(void *user_tag_data);
    void (*on_error)(void *user_tag_data, const char *message);
} Drawer;

extern GLClientRenderThreadStruct *glcrt_create_initialize(const char *thread_name,
                                                           BoolProperty **boolProperty,
                                                           Drawer **drawer);
extern void glcrt_destroy_release(GLClientRenderThreadStruct *glcrt);
extern int glcrt_start(GLClientRenderThreadStruct *glcrt);
extern int glcrt_quit(GLClientRenderThreadStruct *glcrt);
extern int glcrt_get_loop(GLClientRenderThreadStruct *glcrt, bool *loop);
extern int glcrt_get_name(GLClientRenderThreadStruct *glcrt, const char **name);
extern int glcrt_set_available(GLClientRenderThreadStruct *glcrt, bool available);
extern int glcrt_handle_message(GLClientRenderThreadStruct *glcrt, const char *message);
extern int glcrt_set_window(GLClientRenderThreadStruct *glcrt, EGLNativeWindowType window);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IMGPROC_ANDROID_GLCLIENT_RENDER_THREAD_H */
