#ifndef PROJECT_BASEIMAGEPROC_H
#define PROJECT_BASEIMAGEPROC_H

#include <stdint.h>
#include <stdio.h>
#include <math.h>

typedef int32_t (*component_filter) (int32_t);

int32_t grey_to_greyColor(int32_t avg);
void calc_grey(int32_t *pResult, int32_t *pArgb, int32_t size);
void calc_greyColor(int32_t *pResult, int32_t *pArgb, int32_t size);
void calc_greyFilter(int32_t *pResult, int32_t *pArgb, int32_t size, component_filter filter);
void calc_colorFilter(int32_t *pResult, int32_t *pArgb, int32_t size, component_filter filter);

#endif //PROJECT_BASEIMAGEPROC_H
