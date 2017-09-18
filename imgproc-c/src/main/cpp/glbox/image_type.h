#ifndef PROJECT_IMAGE_TYPE_H
#define PROJECT_IMAGE_TYPE_H

#include "base/base.h"

typedef struct tagGImage {
    uint8_t *image;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
} GImage;

#endif /* PROJECT_IMAGE_TYPE_H */
