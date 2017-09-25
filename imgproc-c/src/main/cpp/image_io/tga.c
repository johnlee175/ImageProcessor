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
#include "tga.h"

__pack_begin struct tagTgaFileHeader {
    uint8_t image_id_length; /* 0=no image info */
    uint8_t color_map_type; /* 0=no map, 1=has map */
    /* image_type:
        0=no image data, 1=uncompressed color-mapped image,
        2=uncompressed true-color image,
        3=uncompressed black-white image, 4.5.6.encoded image */
    uint8_t image_type;
    uint16_t color_map_entry_addr;
    uint16_t color_map_count;
    uint8_t color_map_item_bits; /* 16, 24, 32 */
    uint16_t image_x; /* left-bottom point */
    uint16_t image_y; /* left-bottom point */
    uint16_t image_width;
    uint16_t image_height;
    uint8_t image_pixel_bits; /* 16, 24, 32 */
    /* bits 0-3: tga24 is 0;
       bits 4-5: origin point position, tga24 is 0;
       bits 6-7: data cross store flag, tga24 is 0; */
    uint8_t image_descriptor;
} __pack_end;
typedef struct tagTgaFileHeader TgaFileHeader;

__pack_begin struct tagRGB24 {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} __pack_end;
typedef struct tagRGB24 RGB24;

int load_tga_24(PARAM_IN const char *tga_path,
                PARAM_OUT uint8_t **rgb_buffer/* should user free */,
                PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height) {
    TgaFileHeader header;
    const size_t header_size = sizeof(TgaFileHeader);
    FILE *file = fopen(tga_path, "rb");
    if (fread(&header, header_size, (size_t) 1, file) != 1) {
        base_error_log("read tga header failed!");
        fclose(file);
        return -1;
    }
    if (header.image_type != 2 && header.image_pixel_bits != 24) {
        base_error_log("just support tga 24-bits true color image!");
        fclose(file);
        return -1;
    }
    uint8_t skip = header.image_id_length;
    if (header.color_map_type != 0) {
        skip += header.color_map_count * (header.color_map_item_bits / 8);
    }
    if (fseek(file, skip, SEEK_CUR) < 0) {
        base_error_log("seek image data failed!");
        fclose(file);
        return -1;
    }
    const size_t image_pixel_count = header.image_width * header.image_height;
    const size_t image_size = image_pixel_count * sizeof(RGB24);
    DEFINE_HEAP_ARRAY_POINTER(RGB24, rgb, image_pixel_count, {
        base_error_log("malloc buffer failed!");
        fclose(file);
        return -1;
    });
    if (fread(rgb, sizeof(RGB24), (size_t) image_pixel_count, file) != image_pixel_count) {
        base_error_log("read tga image data failed!");
        fclose(file);
        free(rgb);
        return -1;
    }
    const uint32_t row_stride = *width = header.image_width;
    const uint32_t row_count = *height = header.image_height;
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, *rgb_buffer, image_size, {
        base_error_log("malloc buffer failed!");
        fclose(file);
        free(rgb);
        return -1;
    });
    uint8_t *temp = *rgb_buffer;
    int32_t idx = 0;
    for (int32_t i = row_count - 1; i >= 0; --i) {
        for (uint32_t j = 0; j < row_stride; ++j) {
            idx = i * row_stride + j;
            *temp++ = rgb[idx].r;
            *temp++ = rgb[idx].g;
            *temp++ = rgb[idx].b;
        }
    }
    fclose(file);
    free(rgb);
    return 0;
}

int save_tga_24(PARAM_IN const char *tga_path,
                PARAM_IN const uint8_t *rgb_buffer/* should user free */,
                PARAM_IN const uint32_t width, PARAM_IN const uint32_t height) {
    TgaFileHeader header;
    header.image_id_length = 0;
    header.color_map_type = 0;
    header.image_type = 2;
    header.color_map_entry_addr = 0;
    header.color_map_count = 0;
    header.color_map_item_bits = 0;
    header.image_x = 0;
    header.image_y = 0;
    header.image_width = (uint16_t) width;
    header.image_height = (uint16_t) height;
    header.image_pixel_bits = 24;
    header.image_descriptor = 0;
    const size_t header_size = sizeof(TgaFileHeader);
    FILE *file = fopen(tga_path, "wb");
    if (fwrite(&header, header_size, (size_t) 1, file) != 1) {
        base_error_log("write tga header failed!");
        fclose(file);
        return -1;
    }

    const size_t image_pixel_count = header.image_width * header.image_height;
    const uint32_t row_stride = header.image_width;
    const uint32_t row_count = header.image_height;
    const uint8_t *temp = rgb_buffer;
    DEFINE_HEAP_ARRAY_POINTER(RGB24, rgb, image_pixel_count, {
        base_error_log("malloc buffer failed!");
        fclose(file);
        return -1;
    });
    int32_t idx = 0;
    for (int32_t i = row_count - 1; i >= 0; --i) {
        for (uint32_t j = 0; j < row_stride; ++j) {
            idx = i * row_stride + j;
            rgb[idx].r = *temp++;
            rgb[idx].g = *temp++;
            rgb[idx].b = *temp++;
        }
    }
    if (fwrite(rgb, sizeof(RGB24), (size_t) image_pixel_count, file) != image_pixel_count) {
        base_error_log("write tga image data failed!");
        fclose(file);
        free(rgb);
        return -1;
    }

    fclose(file);
    free(rgb);
    return 0;
}