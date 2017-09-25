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
#ifndef PROJECT_BASEIMAGEPROC_H
#define PROJECT_BASEIMAGEPROC_H

#include "base/base.h"

#define TYPE_GREY_COLOR 0
#define TYPE_ARGB_COLOR 5
#define COMPONENT_RED 1
#define COMPONENT_GREEN 2
#define COMPONENT_BLUE 3
#define COMPONENT_ALPHA 4

typedef int32_t (*component_filter) (int32_t);
typedef void (*reset_filter) ();

_inline_ int32_t grey_to_color(int32_t avg) {
    if (avg > 255) { avg = 255; } if (avg < 0) { avg = 0; } // not (avg & 0xFF)
    return (255 << 24) + (avg << 16) + (avg << 8) + avg;
}

_inline_ int32_t color_to_grey(int32_t argb) {
    int32_t r, g, b;
    r = (argb >> 16) & 0xFF;
    g = (argb >> 8) & 0xFF;
    b = argb & 0xFF;
    return (r * 19595 + g * 38469 + b * 7472) >> 16;
}

_inline_ int32_t assemble_color_int(int32_t r, int32_t g, int32_t b) {
    if (r > 255) { r = 255; } if (r < 0) { r = 0; } // not (r & 0xFF)
    if (g > 255) { g = 255; } if (g < 0) { g = 0; } // not (g & 0xFF)
    if (b > 255) { b = 255; } if (b < 0) { b = 0; } // not (b & 0xFF)
    return (255 << 24) + (r << 16) + (g << 8) + b;
}

void calc_raw_grey(int32_t *result, const int32_t *__restrict__ argb, int32_t size);
void calc_grey_color(int32_t *result, const int32_t *__restrict__ argb, int32_t size);
void calc_grey_normal_filter(double *result, const double *__restrict__ argb, int32_t size,
                            reset_filter reset, component_filter grey_filter);
void calc_grey_color_filter(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                            reset_filter reset, component_filter grey_filter);
void calc_rgb_color_filter(int32_t *result, const int32_t *__restrict__ argb, int32_t size, reset_filter reset,
                           component_filter r_filter, component_filter g_filter, component_filter b_filter);
void calc_grey_bit_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size, int32_t bit_position);
void calc_rgb_bit_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size, int32_t bit_position);
void calc_rgb_component_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                              int32_t position, int32_t type);
void calc_combine_simple_plane(int32_t *source, const int32_t *__restrict__ target, int32_t size);
void calc_combine_bits_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                             int32_t type, int32_t mask);

void map_component_color_count(int32_t **result, int32_t  *result_size, const int32_t *__restrict__ argb,
                               int32_t size, int32_t type);
void histogram_equalization_classic(int32_t **result, const int32_t *__restrict__ hist, int32_t size,
                                   int32_t counts);
void histogram_equalization_photoshop(int32_t **result, const int32_t *__restrict__ hist, int32_t size,
                                      int32_t counts);
void map_component_equalization(int32_t **result, const int32_t *__restrict__ hist, int32_t hist_size,
                                const int32_t *__restrict__ argb, int32_t size, int32_t type);

#endif //PROJECT_BASEIMAGEPROC_H
