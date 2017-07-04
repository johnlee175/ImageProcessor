#ifndef PROJECT_BASEIMAGEPROC_H
#define PROJECT_BASEIMAGEPROC_H

#include "base.h"

typedef int32_t (*component_filter) (int32_t);
typedef void (*reset_filter) ();

int32_t grey_to_color(int32_t avg);
int32_t color_to_grey(int32_t argb);
int32_t assemble_color_int(int32_t r, int32_t g, int32_t b);
void calc_raw_grey(int32_t *result, int32_t *argb, int32_t size);
void calc_grey_color(int32_t *result, int32_t *argb, int32_t size);
void calc_grey_color_filter(int32_t *result, int32_t *argb, int32_t size,
                            reset_filter reset, component_filter grey_filter);
void calc_rgb_color_filter(int32_t *result, int32_t *argb, int32_t size, reset_filter reset,
                           component_filter r_filter, component_filter g_filter, component_filter b_filter);
void calc_grey_bit_plane(int32_t *result, int32_t *argb, int32_t size, int32_t bit_position);
void calc_rgb_bit_plane(int32_t *result, int32_t *argb, int32_t size, int32_t bit_position);
void calc_rgb_component_plane(int32_t *result, int32_t *argb, int32_t size, int32_t position, int32_t type);
void calc_combine_simple_plane(int32_t *source, int32_t *target, int32_t size);

#endif //PROJECT_BASEIMAGEPROC_H
