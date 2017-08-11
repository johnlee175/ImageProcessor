#ifndef OCTREE_PALETTE_H
#define OCTREE_PALETTE_H

#include "base/base.h"

typedef struct _ColorQuantization ColorQuantization;

ColorQuantization *create_color_quantization();
void destroy_color_quantization(ColorQuantization *quantization);

bool prepare_color_palette(ColorQuantization *quantization, uint8_t *rgb_data,
                           uint32_t pixel_count, uint32_t max_colors, bool build_index);
bool generate_color_palette(ColorQuantization *quantization, uint8_t **palette/* user should free */);

int32_t index_of_palette(ColorQuantization *quantization, uint8_t r, uint8_t g, uint8_t b);

#endif /* OCTREE_PALETTE_H */
