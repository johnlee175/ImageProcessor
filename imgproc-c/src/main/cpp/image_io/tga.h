#ifndef PROJECT_TGA_H
#define PROJECT_TGA_H

#include "base/base.h"

int load_tga_24(PARAM_IN const char *tga_path,
                PARAM_OUT uint8_t **rgb_buffer/* should user free */,
                PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height);
int save_tga_24(PARAM_IN const char *tga_path,
                PARAM_IN const uint8_t *rgb_buffer/* should user free */,
                PARAM_IN const uint32_t width, PARAM_IN const uint32_t height);

#endif /* PROJECT_TGA_H */
