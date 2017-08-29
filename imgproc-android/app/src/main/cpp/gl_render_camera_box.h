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
 * gl render camera box c interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_GL_RENDER_CAMERA_BOX_H
#define IMGPROC_ANDROID_GL_RENDER_CAMERA_BOX_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Begin: you can custom const variable with define GLRCBOX_CONSTANT_DEFINED */
extern const GLfloat glrcbox_vertex_coords[];
extern const GLfloat glrcbox_back_texture_coords[];
extern const GLfloat glrcbox_front_texture_coords[];
extern const GLushort glrcbox_draw_order[];
extern const GLuint glrcbox_per_vertex_coord_component;
extern const GLuint glrcbox_per_texture_coord_component;
extern const GLchar *glrcbox_vertex_shader_source;
extern const GLchar *glrcbox_fragment_shader_source;
/* End: you can custom const variable with define GLRCBOX_CONSTANT_DEFINED */

typedef struct tagGLRenderCameraBox GLRenderCameraBox;
typedef void (*FrameDataCallback)(GLRenderCameraBox *glrcbox);

extern GLRenderCameraBox *glrcbox_create_initialize();
extern void glrcbox_destroy_release(GLRenderCameraBox *glrcbox);
extern int glrcbox_set_user_tag(GLRenderCameraBox *glrcbox, void *data);
extern void *glrcbox_get_user_tag(GLRenderCameraBox *glrcbox);
extern int glrcbox_set_frame_data_callback(GLRenderCameraBox *glrcbox,
                                           FrameDataCallback callback_func);
extern int glrcbox_set_frame_size(GLRenderCameraBox *glrcbox,
                                  GLuint frame_width,
                                  GLuint frame_height);
extern int glrcbox_set_front_camera(GLRenderCameraBox *glrcbox, GLboolean is_front_camera);
extern int glrcbox_set_shader_source(GLRenderCameraBox *glrcbox,
                                     const GLchar *vertex_source,
                                     const GLchar *fragment_source);
extern int glrcbox_get_pixels(GLRenderCameraBox *glrcbox,
                              GLubyte **pixels, size_t *pixels_size);
extern int glrcbox_create_shader(GLRenderCameraBox *glrcbox);
extern int glrcbox_destroy_shader(GLRenderCameraBox *glrcbox);
extern int glrcbox_draw_frame(GLRenderCameraBox *glrcbox, GLuint texture_id);
extern int glrcbox_set_window(GLRenderCameraBox *glrcbox, EGLNativeWindowType window);
extern EGLNativeWindowType glrcbox_get_window(GLRenderCameraBox *glrcbox);
extern int glrcbox_swap_buffers(GLRenderCameraBox *glrcbox);
extern int glrcbox_create_egl(GLRenderCameraBox *glrcbox);
extern int glrcbox_destroy_egl(GLRenderCameraBox *glrcbox);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IMGPROC_ANDROID_GL_RENDER_CAMERA_BOX_H */
