#ifndef PROJECT_BASEIMAGEPROC_H
#define PROJECT_BASEIMAGEPROC_H

#include "base.h"

typedef int32_t (*component_filter) (int32_t);
typedef void (*reset_filter) ();

int32_t grey_to_color(int32_t avg);
void calc_raw_grey(int32_t *result, int32_t *argb, int32_t size);
void calc_grey_color(int32_t *result, int32_t *argb, int32_t size);
void calc_grey_color_filter(int32_t *result, int32_t *argb, int32_t size,
                            reset_filter reset, component_filter grey_filter);
void calc_rgb_color_filter(int32_t *result, int32_t *argb, int32_t size, reset_filter reset,
                           component_filter r_filter, component_filter g_filter, component_filter b_filter);

#endif //PROJECT_BASEIMAGEPROC_H
