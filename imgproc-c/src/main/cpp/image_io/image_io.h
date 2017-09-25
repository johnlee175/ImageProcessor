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
#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base/base.h"
#include "octree_palette.h"
#include "bmp.h"
#include "tga.h"

#include <png.h>
#include <jpeglib.h>
#include <gif_lib.h>
#include <webp/encode.h>
#include <webp/mux.h>
#include <webp/decode.h>
#include <webp/demux.h>

typedef enum e_imgio_type {
    UNKNOWN, JPEG, PNG, GIF, WEBP, TGA, BMP, PPM, BPG/* feature */
} imgio_type;

imgio_type imgio_get_image_type(PARAM_IN const char *file_path, PARAM_IN const bool check_file_extend);

void imgio_dump_image_type(PARAM_IN imgio_type image_type);

int imgio_convert_rgb_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_rgba_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_rgb_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_argb_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_argb_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_rgba_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_bgr_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_abgr_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_convert_bgra_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size);

int imgio_read_ppm_p3_255(PARAM_IN const char *file_path,
                          PARAM_OUT uint8_t **rgb_buffer/* user should free */,
                          PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height);

int imgio_read_ppm_p6_255(PARAM_IN const char *file_path,
                          PARAM_OUT uint8_t **rgb_buffer/* user should free */,
                          PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height);

int imgio_write_ppm_p3_255(PARAM_IN const char *file_path,
                           PARAM_IN const uint8_t *rgb_buffer/* user should free */,
                           PARAM_IN const uint32_t width, PARAM_IN const uint32_t height);

int imgio_write_ppm_p6_255(PARAM_IN const char *file_path,
                           PARAM_IN const uint8_t *rgb_buffer/* user should free */,
                           PARAM_IN const uint32_t width, PARAM_IN const uint32_t height);

int imgio_read_png(PARAM_IN const char *file_path, PARAM_OUT png_bytep *buffer/* user should free */,
                   PARAM_OUT png_uint_32 *img_width, PARAM_OUT png_uint_32 *img_height,
                   PARAM_OUT png_uint_32 *format);

int imgio_write_png(PARAM_IN const char *file_path, PARAM_IN const png_bytep pixels,
                    PARAM_IN const png_uint_32 img_width, PARAM_IN const png_uint_32 img_height,
                    PARAM_IN const bool has_alpha);

int imgio_read_png2(PARAM_IN const char *file_path, PARAM_OUT png_bytep *buffer/* user should free */,
                    PARAM_OUT png_uint_32 *width, PARAM_OUT png_uint_32 *height);

int imgio_write_png2(PARAM_IN const char *file_name, PARAM_IN const png_bytep image,
                     PARAM_IN const png_uint_32 width, PARAM_IN const png_uint_32 height,
                     PARAM_IN const bool use_interlace, PARAM_IN const png_uint_32 dpi, PARAM_IN const bool has_alpha);

int imgio_write_jpeg(PARAM_IN const char *filename, PARAM_IN const JDIMENSION quality/* 1 ~ 100 */,
                     PARAM_IN const JSAMPLE *image_rgb_buffer,
                     PARAM_IN const JDIMENSION image_width, PARAM_IN const JDIMENSION image_height);

int imgio_read_jpeg(PARAM_IN char *filename, PARAM_OUT JSAMPLE **image_rgb_buffer,
                    PARAM_OUT JDIMENSION *width, PARAM_OUT JDIMENSION *height, PARAM_OUT JDIMENSION *component);

typedef struct tagGifPicture {
    GifWord image_count;
    GifWord screen_width;
    GifWord screen_height;
    GifWord anim_loop_times;
} GifPicture;

typedef struct tagGifFrame {
    GifWord image_index;
    GifWord per_pixel_bytes;
    /* after callback will be freed, user should save buffer to others */
    GifByteType *rgb_buffer;
    GifColorType background_color;
    GifColorType transparent_color;
    GifWord disposal_mode;
    GifWord delay_time;/* (x / 100) s */
    GifWord left;
    GifWord top;
    GifWord image_width;
    GifWord image_height;
} GifFrame;

typedef void (*func_on_gif_frame)(GifPicture *picture, GifFrame *frame);

typedef struct _GifDataModelInterface {
    GifWord (*frame_count)();

    void (*make_current_frame)(GifWord index);

    void (*release_current_frame)();

    GifWord (*current_frame_width)();

    GifWord (*current_frame_height)();

    GifByteType *(*current_frame_buffer)();

    OPTIONAL GifColorType (*current_frame_transparent_color)();

    OPTIONAL GifWord (*current_frame_delay_time)();

    OPTIONAL GifWord (*current_frame_disposal_mode)();
} GifDataModelInterface;

_inline_ bool check_GifDataModelInterface(const GifDataModelInterface *interface) {
    if (interface == NULL) return false;
    if (interface->frame_count == NULL) return false;
    if (interface->make_current_frame == NULL) return false;
    if (interface->release_current_frame == NULL) return false;
    if (interface->current_frame_width == NULL) return false;
    if (interface->current_frame_height == NULL) return false;
    if (interface->current_frame_buffer == NULL) return false;
    return true;
}

/* stream callback function, no interlace feature */
int imgio_read_gif(PARAM_IN char *filename, PARAM_IN bool make_alpha, PARAM_IN func_on_gif_frame on_gif_frame);

/* stream extract interface function, no interlace feature, global palette */
int imgio_write_gif(PARAM_IN const char *filename, PARAM_IN const GifDataModelInterface *interface,
                    PARAM_IN const GifWord loop_count, PARAM_IN const GifColorType *background_color);

int imgio_rgba_scale_bilinear(PARAM_IN const uint32_t component /* per pixel in bytes */,
                              PARAM_IN const uint8_t *_restrict_ buffer_src,
                              PARAM_INOUT uint8_t *_restrict_ buffer_dst,
                              PARAM_IN const uint32_t width_src, PARAM_IN const uint32_t height_src,
                              PARAM_IN const uint32_t width_dst, PARAM_IN const uint32_t height_dst);

typedef double_t (*imgio_scale_bicubic_s_func)(double_t);

int imgio_rgba_scale_bicubic_convolution(PARAM_IN const uint32_t component /* per pixel in bytes */,
                             PARAM_IN const uint8_t *_restrict_ buffer_src,
                             PARAM_INOUT uint8_t *_restrict_ buffer_dst,
                             PARAM_IN const uint32_t width_src, PARAM_IN const uint32_t height_src,
                             PARAM_IN const uint32_t width_dst, PARAM_IN const uint32_t height_dst,
                             PARAM_IN imgio_scale_bicubic_s_func s_func);

double_t imgio_scale_bicubic_s_custom(double_t a, double_t x); /* a in [-0.5, -1.0, -2.0] is good */

double_t imgio_scale_bicubic_s_default(double_t x); /* like imgio_scale_bicubic_s_custom(-1, x) */
DEPRECATED double_t imgio_scale_bicubic_s_bell(double_t x);
DEPRECATED double_t imgio_scale_bicubic_s_bspline(double_t x);
double_t imgio_scale_bicubic_s_lagrange(double_t x);
DEPRECATED double_t imgio_scale_bicubic_s_6param(double_t x);
DEPRECATED double_t imgio_scale_bicubic_s_8param(double_t x);

#ifndef imgio_scale_bicubic_s
#define imgio_scale_bicubic_s imgio_scale_bicubic_s_default
#endif

_inline_ int imgio_rgba_scale_bicubic(PARAM_IN const uint32_t component /* per pixel in bytes */,
                                      PARAM_IN const uint8_t *_restrict_ buffer_src,
                                      PARAM_INOUT uint8_t *_restrict_ buffer_dst,
                                      PARAM_IN const uint32_t width_src, PARAM_IN const uint32_t height_src,
                                      PARAM_IN const uint32_t width_dst, PARAM_IN const uint32_t height_dst) {
    return imgio_rgba_scale_bicubic_convolution(component, buffer_src, buffer_dst,
                                         width_src, height_src, width_dst, height_dst, imgio_scale_bicubic_s);
}

#ifndef imgio_rgba_scale
#define imgio_rgba_scale imgio_rgba_scale_bilinear
#endif

#ifdef __cplusplus
} // close 'extern "C" {'
#endif

#endif /* IMAGE_IO_H */
