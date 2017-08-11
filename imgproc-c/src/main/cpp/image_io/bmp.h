#ifndef PROJECT_BMP_H
#define PROJECT_BMP_H

#include "base/base.h"

int load_bmp_24_b2t(const char *bmp_path, uint8_t **rgb_buffer/* should user free */,
                    uint32_t *width, uint32_t *height);
int save_bmp_24_b2t(const char *bmp_path, const uint8_t *rgb_buffer/* should user free */,
                    const uint32_t width, const uint32_t height);

#endif /* PROJECT_BMP_H */
