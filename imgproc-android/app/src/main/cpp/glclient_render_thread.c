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
 * GL client render thread implement
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#include "glclient_render_thread.h"

struct tagGLClientRenderThreadStruct {
    BoolProperty paused_prop;
    Drawer drawer;
    bool loop;
    bool available;
    EGLNativeWindowType egl_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    EGLSurface egl_surface;
    const char *thread_name;
    pthread_cond_t *thread_cond;
    pthread_mutex_t *thread_mutex;
    pthread_t *thread;
};

static void glcrt_egl_error_string(GLint error, const char *prefix, char *buffer) {
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

static void glcrt_throw_error(GLClientRenderThreadStruct *glcrt, const char *message) {
    glcrt->drawer.on_error(glcrt, message);
}

static int glcrt_init_egl(GLClientRenderThreadStruct *glcrt, EGLNativeWindowType window) {
    char buffer[256];
    if ((glcrt->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        glcrt_egl_error_string(eglGetError(), "eglGetDisplay failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
        return -1;
    }
    EGLint major, minor;
    if (!eglInitialize(glcrt->egl_display, &major, &minor)) {
        glcrt_egl_error_string(eglGetError(), "eglInitialize failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
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
    eglChooseConfig(glcrt->egl_display, configSpec, glcrt->egl_config, 1, &num_config);
    if (num_config <= 0) {
        glcrt_egl_error_string(eglGetError(), "eglChooseConfig failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
        return -1;
    }
    GLint contextSpec[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
    };
    if ((glcrt->egl_context = eglCreateContext(glcrt->egl_display, glcrt->egl_config,
                                             EGL_NO_CONTEXT, contextSpec)) == EGL_NO_CONTEXT) {
        glcrt_egl_error_string(eglGetError(), "eglCreateContext failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
        return -1;
    }
    if ((glcrt->egl_surface = eglCreateWindowSurface(glcrt->egl_display, glcrt->egl_config, window,
                                                   NULL)) == EGL_NO_SURFACE) {
        glcrt_egl_error_string(eglGetError(), "eglCreateWindowSurface failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
        return -1;
    }
    if (!eglMakeCurrent(glcrt->egl_display, glcrt->egl_surface, glcrt->egl_surface, glcrt->egl_context)) {
        glcrt_egl_error_string(eglGetError(), "eglMakeCurrent failed:", buffer);
        glcrt_throw_error(glcrt, buffer);
        return -1;
    }
    return 0;
}

static int glcrt_release_egl(GLClientRenderThreadStruct *glcrt, EGLNativeWindowType window) {
    eglMakeCurrent(glcrt->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(glcrt->egl_display, glcrt->egl_surface);
    eglDestroyContext(glcrt->egl_display, glcrt->egl_context);
    eglTerminate(glcrt->egl_display);
    glcrt->egl_display = EGL_NO_DISPLAY;
    glcrt->egl_context = EGL_NO_CONTEXT;
    glcrt->egl_surface = EGL_NO_SURFACE;
}

GLClientRenderThreadStruct *glcrt_create_initialize(const char *thread_name,
                                                    BoolProperty **boolProperty,
                                                    Drawer **drawer) {
    GLClientRenderThreadStruct *glcrt = _static_cast(GLClientRenderThreadStruct *)
            malloc(sizeof(GLClientRenderThreadStruct));
    if (glcrt && boolProperty && drawer) {
        memset(glcrt, 0, sizeof(GLClientRenderThreadStruct));
        glcrt->thread_name = thread_name;
        glcrt->loop = true;
        glcrt->available = false;
        *boolProperty = &glcrt->paused_prop;
        *drawer = &glcrt->drawer;
        return glcrt;
    }
    return NULL;
}

void glcrt_destroy_release(GLClientRenderThreadStruct *glcrt) {
    if (glcrt) {
        if (glcrt->thread_name) {
            free(_const_cast(void *) glcrt->thread_name);
        }
        if (glcrt->paused_prop.user_tag_data) {
            free(glcrt->paused_prop.user_tag_data);
            glcrt->paused_prop.user_tag_data = NULL;
        }
        if (glcrt->drawer.user_tag_data) {
            free(glcrt->drawer.user_tag_data);
            glcrt->drawer.user_tag_data = NULL;
        }
        free(glcrt);
    }
}

static void glcrt_thread_exit_handler(int signo) {
    pthread_t tid = pthread_self();
    LOGI("[%lu] exit thread with signo = %d \n", tid, signo);
    /* TODO release resource here */
    pthread_exit(NULL);
}

/* always return NULL or no return */static void *glcrt_run(GLClientRenderThreadStruct *glcrt) {
    pthread_t tid = pthread_self();
    LOGI("[%lu] glcrt_run ...\n", tid);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = glcrt_thread_exit_handler;
    sigaction(SIGUSR1, &sa, NULL);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    /* real logic */
    glcrt_init_egl(glcrt, glcrt->egl_window);
    glcrt->drawer.create(glcrt->drawer.user_tag_data);
    struct timeval now;
    struct timespec outtime;
    pthread_mutex_lock(glcrt->thread_mutex);
    while (glcrt->loop) {
        while (glcrt->paused_prop.get_prop(glcrt->paused_prop.user_tag_data)) {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 1;
            outtime.tv_nsec = 0;
            pthread_cond_timedwait(glcrt->thread_cond, glcrt->thread_mutex, &outtime);
        }
        glcrt->drawer.draw(glcrt->drawer.user_tag_data);
        if (!eglSwapBuffers(glcrt->egl_display, glcrt->egl_surface)) {
            glcrt_throw_error(glcrt, "Can not swap buffers");
        }
        while (!glcrt->available) {
            gettimeofday(&now, NULL);
            outtime.tv_sec = now.tv_sec + 1;
            outtime.tv_nsec = 0;
            pthread_cond_timedwait(glcrt->thread_cond, glcrt->thread_mutex, &outtime);
        }
        glcrt->available = false;
    }
    pthread_mutex_unlock(glcrt->thread_mutex);
    glcrt->drawer.destroy(glcrt->drawer.user_tag_data);
    glcrt_release_egl(glcrt, glcrt->egl_window);

    return NULL;
}

int glcrt_start(GLClientRenderThreadStruct *glcrt) {
    if (glcrt) {
        pthread_attr_t thread_attr;
        if (pthread_attr_init(&thread_attr) != 0) {
            LOGW("pthread_attr_init failed");
            return -1;
        }
        if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE) != 0) {
            LOGI("pthread_attr_setdetachstate(PTHREAD_CREATE_JOINABLE) failed");
        }
        if (pthread_attr_setstacksize(&thread_attr, 1024 * 1024 * 2) != 0) {
            LOGI("pthread_attr_setstacksize(1024 * 1024 * 2) failed");
        }
        pthread_mutex_init(glcrt->thread_mutex, NULL);
        pthread_cond_init(glcrt->thread_cond, NULL);
        if (pthread_create(glcrt->thread, &thread_attr,
                           _static_cast(void *(*)(void *))glcrt_run, glcrt) != 0) {
            LOGW("pthread_create failed");
            return -1;
        }
        if (pthread_setname_np(*glcrt->thread, glcrt->thread_name) != 0) {
            LOGI("pthread_setname_np(arg1, arg2) failed");
        }
        return 0;
    }
    return -1;
}

int glcrt_quit(GLClientRenderThreadStruct *glcrt) {
    if (glcrt && glcrt->thread) {
        glcrt->loop = false;
        pthread_kill(*glcrt->thread, SIGUSR1);
        void *exit_status;
        pthread_join(*glcrt->thread, &exit_status);
        glcrt_destroy_release(glcrt);
        return 0;
    }
    return -1;
}

int glcrt_get_loop(GLClientRenderThreadStruct *glcrt, bool *loop) {
    if (glcrt && loop) {
        *loop = glcrt->loop;
        return 0;
    }
    return -1;
}

int glcrt_get_name(GLClientRenderThreadStruct *glcrt, const char **name) {
    if (glcrt && name) {
        *name = glcrt->thread_name;
        return 0;
    }
    return -1;
}

int glcrt_set_available(GLClientRenderThreadStruct *glcrt, bool available) {
    if (glcrt) {
        glcrt->available = available;
        return 0;
    }
    return -1;
}

int glcrt_handle_message(GLClientRenderThreadStruct *glcrt, const char *message) {
    if (glcrt) {
        pthread_mutex_lock(glcrt->thread_mutex);
        pthread_cond_signal(glcrt->thread_cond);
        pthread_mutex_unlock(glcrt->thread_mutex);
        return 0;
    }
    return -1;
}

int glcrt_set_window(GLClientRenderThreadStruct *glcrt, EGLNativeWindowType window) {
    if (glcrt) {
        glcrt->egl_window = window;
        return 0;
    }
    return -1;
}
