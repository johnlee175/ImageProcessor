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
 * camera native view interface
 *
 * @author John Kenrinus Lee
 * @version 2017-08-18
 */
#ifndef IMGPROC_ANDROID_CAMERA_NATIVE_VIEW_H
#define IMGPROC_ANDROID_CAMERA_NATIVE_VIEW_H

#include "base.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const GLfloat cnv_vertex_coords[];
extern const GLfloat cnv_back_texture_coords[];
extern const GLfloat cnv_front_texture_coords[];
extern const GLushort cnv_draw_order[];
extern const GLuint cnv_per_vertex_coord_component;
extern const GLuint cnv_per_texture_coord_component;
extern const GLchar *cnv_vertex_shader_source;
extern const GLchar *cnv_fragment_shader_source;

typedef struct tagCameraNativeViewStruct CameraNativeViewStruct;
typedef void (*cnv_frame_data_callback)(CameraNativeViewStruct *cnv);

extern CameraNativeViewStruct *cnv_create_initialize();
extern void cnv_destroy_release(CameraNativeViewStruct *cnv);
extern int cnv_set_user_tag(CameraNativeViewStruct *cnv, void *data);
extern void *cnv_get_user_tag(CameraNativeViewStruct *cnv);
extern int cnv_set_frame_data_callback(CameraNativeViewStruct *cnv, cnv_frame_data_callback callback_func);
extern int cnv_set_frame_size(CameraNativeViewStruct *cnv, GLuint frame_width, GLuint frame_height);
extern int cnv_set_front_camera(CameraNativeViewStruct *cnv, GLboolean is_front_camera);
extern int cnv_set_shader_source(CameraNativeViewStruct *cnv, const GLchar *vertex_source, const GLchar *fragment_source);
extern int cnv_get_pixels(CameraNativeViewStruct *cnv, GLubyte **pixels, size_t *pixels_size);
extern int cnv_create_shader(CameraNativeViewStruct *cnv);
extern int cnv_destroy_shader(CameraNativeViewStruct *cnv);
extern int cnv_draw_frame(CameraNativeViewStruct *cnv, GLuint texture_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* IMGPROC_ANDROID_CAMERA_NATIVE_VIEW_H */
