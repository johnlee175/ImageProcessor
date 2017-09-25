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
#ifndef GLBOX_H
#define GLBOX_H

// Standard Header

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// OpenGL Header

#define GLEW_STATIC
#include <GL/glew.h>


#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL3/gl3.h>
#include <GL3/gl3ext.h>
#endif

#if defined(__APPLE__) || defined(__MACH__)
#include <OpenGL/OpenGL.h>
/* #include <OpenGL/gl.h> */
/* #include <OpenGL/glu.h> */
/* #include <OpenGL/glext.h> */
/* #include <OpenGL/gl3.h> */
/* #include <OpenGL/gl3ext.h> */
#endif

#ifdef __linux__
#include<X11/Xlib.h>
#include<GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL3/gl3.h>
#include <GL3/gl3ext.h>
#endif

#if defined(__unix__) || defined(unix)
#define is_unix_os
#endif

#ifdef __GNUC__
#define GCC_COMPILE
#endif

#ifdef __clang__
#define CLANG_COMPILE
#endif

#ifdef _MSC_VER
#define VS_COMPILE
#endif


#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

// Base Structure

typedef struct tagGLBox {
    const GLchar *vsSource;
    const GLchar *fsSource;
    GLFWwindow* window;
    int window_width;
    int window_height;
    int viewport_width;
    int viewport_height;
    int texture_width;
    int texture_height;
    GLuint program;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint fbo;
    GLuint texture;
    int texture_index;
} GLBox;

typedef void (*display_func)(GLBox *box);
typedef int (*setup_func)(GLBox *box);
typedef void (*teardown_func)(GLBox *box);

// Functions

#define glbox_info_log(...) ((void)fprintf(stdout, __VA_ARGS__))
#define glbox_error_log(...) ((void)fprintf(stderr, __VA_ARGS__))

GLBox *glbox_create();
int glbox_load_shader_file(GLBox *box, const char *vs_file, const char *fs_file);
int glbox_glfw_init_window_context(GLBox *box, int width, int height, const char *title);
int glbox_viewport_with_window(GLBox *box);
int glbox_compile_link_shader(GLBox *box);
int glbox_loop(GLBox *box, setup_func setup, display_func display, teardown_func teardown);
void glbox_destroy(GLBox *box);

void glbox_util_vertex_array_f(GLuint *vao, GLuint *vbo, GLuint program,
                               GLfloat *vertices, size_t vertices_size,
                               const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_util_use_vertex_array_f(GLuint vao, GLuint vbo, GLuint program,
                                   const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_util_vertex_element_f(GLuint *vao, GLuint *vbo, GLuint *ebo, GLuint program,
                                 GLfloat *vertices, size_t vertices_size, GLuint *indices, size_t indices_size,
                                 const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_util_use_vertex_element_f(GLuint vao, GLuint vbo, GLuint ebo, GLuint program,
                                     const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_util_draw_vertex_array(GLuint vao, GLuint vbo, GLsizei triangle_count);
void glbox_util_draw_vertex_element(GLuint vao, GLuint vbo, GLuint ebo, GLsizei triangle_count);

void glbox_vertex_array_f(GLBox *box, GLfloat *vertices, size_t vertices_size,
                               const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_use_vertex_array_f(GLBox *box, const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_vertex_element_f(GLBox *box, GLfloat *vertices, size_t vertices_size,
                            GLuint *indices, size_t indices_size,
                            const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_use_vertex_element_f(GLBox *box, const GLchar *vertex_attrib_name, GLint line_size, GLint offset);
void glbox_draw_vertex_array(GLBox *box, GLsizei triangle_count);
void glbox_draw_vertex_element(GLBox *box, GLsizei triangle_count);

void glbox_util_uniform_1f(GLuint program, const GLchar *vertex_attrib_name, GLfloat value);
void glbox_util_uniform_Matrix4fv(GLuint program, const GLchar *vertex_attrib_name, const GLfloat *value);
void glbox_util_attribute_3f(GLuint program, const GLchar *vertex_attrib_name,
                             GLfloat v1, GLfloat v2, GLfloat v3);

void glbox_util_draw_pixels(const void *pixels, GLsizei width, GLsizei height);
void glbox_util_save_pixels_ppm3(const char *ppm_file_path, GLsizei width, GLsizei height);
void glbox_save_pixels_ppm3(GLBox *box, const char *ppm_file_path);

int glbox_display_by_freeglut(int argc, char **argv, int width, int height, const char *title,
                              void (*display)(void));

void glbox_prepare_display(GLclampf r, GLclampf g, GLclampf b);

void glbox_util_texture_2d(GLuint *texture, int texture_index,
                           GLuint program, const GLchar *vertex_uniform_name, GLint st_param,
                           const void *pixels, GLsizei img_width, GLsizei img_height);
void glbox_util_use_texture_2d(GLuint texture, int texture_index,
                               GLuint program, const GLchar *vertex_uniform_name,
                               const void *pixels, GLsizei img_width, GLsizei img_height);
void glbox_texture_2d(GLBox *box, int texture_index,const GLchar *vertex_uniform_name, GLint st_param,
                      const void *pixels, GLsizei img_width, GLsizei img_height);
void glbox_use_texture_2d(GLBox *box, const GLchar *vertex_uniform_name, const void *pixels);

int glbox_cocoa_context();

void glbox_viewport_with_fix_size(GLBox *box, int width, int height);

int glbox_util_framebuffer(GLuint *framebuffer, GLsizei width, GLsizei height,
                           GLuint texture, GLenum attachment);

int glbox_framebuffer(GLBox *box, GLenum attachment);

#endif // GLBOX_H
