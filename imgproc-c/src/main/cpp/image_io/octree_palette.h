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
