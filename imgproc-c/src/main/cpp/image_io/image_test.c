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
//#define imgio_scale_bicubic_s imgio_scale_bicubic_s_lagrange
//#define imgio_rgba_scale imgio_rgba_scale_bicubic
#include "image_io.h"

uint32_t img_width, img_height;
uint8_t *pixels;

void init_rgb() {
    img_width = 600;
    img_height = 375;
    pixels = malloc((size_t) (img_width * img_height * 3));
    FILE *file = fopen("../resources/test1.rgb", "rb");
    fread(pixels, sizeof(uint8_t), (size_t) (img_width * img_height * 3), file);
    fclose(file);
}

void init_small_rgb() {
    static const uint8_t arr_size = 6;
    static const uint8_t mask[] = { 0x0, 0x33, 0x66, 0x99, 0xcc, 0xff };
    img_width = 18;
    img_height = 12;
    pixels = malloc((size_t) (img_width * img_height * 3));
    uint8_t *temp = pixels;
    for (int r = 0; r < arr_size; ++r) {
        for (int g = 0; g < arr_size; ++g) {
            for (int b = 0; b < arr_size; ++b) {
                *temp++ = mask[r];
                *temp++ = mask[g];
                *temp++ = mask[b];
            }
        }
    }
}

int test_write_png() {
    init_rgb();
    return imgio_write_png("../resources/mm.png", pixels, img_width, img_height, false);
}

int test_write_jpg() {
    init_rgb();
    return imgio_write_jpeg("../resources/mm.jpg", 100, pixels, img_width, img_height);
}

int test_read_jpg_write_to_png() {
    JSAMPLE *buffer;
    JDIMENSION width, height;
    JDIMENSION component;
    if (!imgio_read_jpeg("../resources/702.jpg",
                       &buffer, &width, &height, &component)) {
        return imgio_write_png("../resources/702a1.png", buffer, width, height, false);
    }
    return -1;
}

int test_read_png_write_to_jpg() {
    png_bytep buffer_rgba;
    png_uint_32 width, height;
    png_uint_32 format;
    if (!imgio_read_png("../resources/704.png", &buffer_rgba, &width, &height, &format)) {
        JSAMPLE *buffer_rgb = malloc(width * height * 3);
        imgio_convert_rgba_to_rgb(buffer_rgba, buffer_rgb, width * height * 3);
        return imgio_write_jpeg("../resources/704a1.jpg", 100, buffer_rgb, width, height);
    }
    return -1;
}

int test_dump_image_type() {
    imgio_dump_image_type(imgio_get_image_type("../resources/704.png", true));
    imgio_dump_image_type(imgio_get_image_type("../resources/702.jpg", false));
    imgio_dump_image_type(imgio_get_image_type("../resources/gogo.gif", true));
    imgio_dump_image_type(imgio_get_image_type("../resources/a.webp", false));
    imgio_dump_image_type(imgio_get_image_type("../resources/a.tif", true));
    imgio_dump_image_type(imgio_get_image_type("../resources/302.bmp", false));
    return 0;
}

int test_convert_components() {
    uint8_t *buffer;
    size_t size;
    uint8_t buffer_rgb[] = {
            21, 42, 112, 200, 69, 158, 50, 50, 50,
            115, 205, 92, 210, 219, 240, 42, 80, 140,
    };
    uint8_t buffer_bgr[] = {
            112, 42, 21, 158, 69, 200, 50, 50, 50,
            92, 205, 115, 240, 219, 210, 140, 80, 42,
    };
    uint8_t buffer_rgba[] = {
            21, 42, 112, 255, 200, 69, 158, 255, 50, 50, 50, 255,
            115, 205, 92, 255, 210, 219, 240, 255, 42, 80, 140, 255,
    };
    uint8_t buffer_argb[] = {
            255, 21, 42, 112, 255, 200, 69, 158, 255, 50, 50, 50,
            255, 115, 205, 92, 255, 210, 219, 240, 255, 42, 80, 140,
    };

    size = 24;
    buffer = malloc(size);
    imgio_convert_rgb_to_rgba(buffer_rgb, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 18;
    buffer = malloc(size);
    imgio_convert_rgba_to_rgb(buffer_rgba, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 24;
    buffer = malloc(size);
    imgio_convert_rgb_to_argb(buffer_rgb, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 18;
    buffer = malloc(size);
    imgio_convert_argb_to_rgb(buffer_argb, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 24;
    buffer = malloc(size);
    imgio_convert_argb_to_rgba(buffer_argb, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 24;
    buffer = malloc(size);
    imgio_convert_rgba_to_argb(buffer_rgba, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    size = 18;
    buffer = malloc(size);
    imgio_convert_bgr_to_rgb(buffer_bgr, buffer, size);
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", buffer[i]);
    }
    free(buffer);
    printf("\n");

    return 0;
}

int op_bmp_load() {
    uint8_t *buffer_rgb;
    uint32_t width, height;
    if (load_bmp_24_b2t("../resources/302.bmp", &buffer_rgb, &width, &height) == 0) {
        return imgio_write_jpeg("../resources/302a1.jpg", 100, buffer_rgb,
                                (const JDIMENSION) width, (const JDIMENSION) height);
    }
    return -1;
}

int op_bmp_save() {
    init_rgb();
    return save_bmp_24_b2t("../resources/mm.bmp", pixels, img_width, img_height);
}

int op_tga_load() {
    uint8_t *buffer_rgb;
    uint32_t width, height;
    if (load_tga_24("../resources/5031.tga", &buffer_rgb, &width, &height) == 0) {
        return imgio_write_jpeg("../resources/5031a1.jpg", 100, buffer_rgb,
                                (const JDIMENSION) width, (const JDIMENSION) height);
    }
    return -1;
}

int op_tga_save() {
    init_rgb();
    return save_tga_24("../resources/mm.tga", pixels, img_width, img_height);
}

int op_ppm3_read() {
    uint8_t *buffer_rgb;
    uint32_t width, height;
    if (imgio_read_ppm_p3_255("../resources/8391.ppm", &buffer_rgb, &width, &height) == 0) {
        return imgio_write_jpeg("../resources/8391a1.jpg", 100, buffer_rgb,
                                (const JDIMENSION) width, (const JDIMENSION) height);
    }
    return -1;
}

int op_ppm6_read() {
    uint8_t *buffer_rgb;
    uint32_t width, height;
    if (imgio_read_ppm_p6_255("../resources/8191.ppm", &buffer_rgb, &width, &height) == 0) {
        return imgio_write_jpeg("../resources/8191a1.jpg", 100, buffer_rgb,
                                (const JDIMENSION) width, (const JDIMENSION) height);
    }
    return -1;
}

int op_ppm3_write() {
    init_rgb();
    return imgio_write_ppm_p3_255("../resources/mm3.ppm", pixels, img_width, img_height);
}

int op_ppm6_write() {
    init_rgb();
    return imgio_write_ppm_p6_255("../resources/mm.ppm", pixels, img_width, img_height);
}

int op_color_palette() {
    init_small_rgb();

    uint8_t *palette;

    ColorQuantization *quantization = create_color_quantization();

    prepare_color_palette(quantization, pixels, img_width * img_height, 256, true);
    generate_color_palette(quantization, &palette);

    int32_t idx = -1;
    if ((idx = index_of_palette(quantization, 51, 153, 255)) >= 0) {
        printf("Oh, index = %d!\n", idx);
    } else {
        printf("Sorry, calc index failed!\n");
    }

    destroy_color_quantization(quantization);

    FILE *file = fopen("../resources/plt.txt", "w");
    for (int i = 0; i < 256 * 3; i += 3) {
        fprintf(file, "%d %d %d\n", palette[i], palette[i + 1], palette[i + 2]);
    }
    fclose(file);

    free(palette);
    return 0;
}

int op_color_palette2() {
    init_rgb();

    uint8_t *palette;

    ColorQuantization *quantization = create_color_quantization();

    prepare_color_palette(quantization, pixels, img_width * img_height, 256, true);
    generate_color_palette(quantization, &palette);

    FILE *file = fopen("../resources/idc_pale.txt", "w");
    for (uint32_t i = 0; i < img_height; ++i) {
        for (uint32_t j = 0; j < img_width * 3; j += 3) {
            int idx = i * img_width * 3 + j;
            fprintf(file, "%d\t", index_of_palette(quantization, pixels[idx], pixels[idx + 1], pixels[idx + 2]));
        }
        fprintf(file, "\n");
    }
    fclose(file);

    destroy_color_quantization(quantization);
    free(palette);
    return 0;
}

void callback_my_on_gif_frame(GifPicture *picture, GifFrame *frame) {
    char file_name[128];
    memset(file_name, '\0', 128);
    sprintf(file_name, "../resources/ZZZ%d.tga", frame->image_index);
    save_tga_24(file_name, frame->rgb_buffer, (const uint32_t) picture->screen_width,
                (const uint32_t) picture->screen_height);
}

int test_read_gif_write_to_tga() {
    return imgio_read_gif("../resources/welcome2.gif", false, callback_my_on_gif_frame);
}

void callback_gif_frame_rgb_file(GifPicture *picture, GifFrame *frame) {
    char file_name[128];
    memset(file_name, '\0', 128);
    sprintf(file_name, "../resources/ZZZ_%dx%d_%d.rgb", picture->screen_width, picture->screen_height,
            frame->image_index);
    size_t img_size = (size_t) (picture->screen_width * picture->screen_height * 3);
    FILE *file = fopen(file_name, "wb");
    fwrite(frame->rgb_buffer, 1, img_size, file);
    fclose(file);
}

int data_prepare_read_gif_to_rgb() {
    return imgio_read_gif("../resources/welcome2.gif", false, callback_gif_frame_rgb_file);
}

/*+++++++++++++++++++++++++++test_write_gif++++++++++++++++++++++++++++*/
uint8_t *test_buffer;

static GifWord my_frame_count() {
    return 6;
}
static GifWord my_current_frame_width() {
    return 290;
}
static GifWord my_current_frame_height() {
    return 48;
}
static void my_make_current_frame(GifWord index) {
    char name[128];
    sprintf(name, "../resources/ZZZ_290x48_%d.rgb", index);
    FILE *file = fopen(name, "rb");
    size_t size = (size_t) (my_current_frame_width() * my_current_frame_height() * 3);
    test_buffer = (uint8_t *) malloc(size);
    fread(test_buffer, 1, size, file);
    fclose(file);
}
static void my_release_current_frame() {
    free(test_buffer);
}
static GifByteType *my_current_frame_buffer() {
    return test_buffer;
}
static GifWord my_current_frame_delay_time() {
    return 40;
}

int test_write_gif() {
    DEFINE_HEAP_TYPE_POINTER(GifDataModelInterface, interface, {
        base_error_log("Malloc failed!");
        return -1;
    });
    interface->frame_count = my_frame_count;
    interface->current_frame_width = my_current_frame_width;
    interface->current_frame_height = my_current_frame_height;
    interface->make_current_frame = my_make_current_frame;
    interface->release_current_frame = my_release_current_frame;
    interface->current_frame_buffer = my_current_frame_buffer;
    interface->current_frame_delay_time = my_current_frame_delay_time;
    return imgio_write_gif("../resources/hello11a1.gif", interface, 0, NULL);
}

int simple_scale(const uint32_t target_width, const uint32_t target_height, const int index) {
    uint8_t *buffer_rgb;
    uint32_t width, height;
    if (load_bmp_24_b2t("../resources/302.bmp", &buffer_rgb, &width, &height) == 0) {
        const uint32_t component = 3;
        DEFINE_HEAP_ARRAY_POINTER(uint8_t, buffer_scaled, (target_width * 3 * target_height), {
            base_error_log("Malloc failed!");
            return -1;
        });
        char file_name[128];
        memset(file_name, '\0', 128);
        sprintf(file_name, "../resources/302a1_scale%ux%u_%d.tga", target_width, target_height, index);
        clock_t start = clock(); /* see CLOCKS_PER_SEC in <time.h> */
        if (!imgio_rgba_scale(component, buffer_rgb, buffer_scaled, width, height,
                             target_width, target_height)) {
            clock_t end = clock(); /* see CLOCKS_PER_SEC in <time.h> */
            printf("take time: %ld\n", end - start);
            return save_tga_24(file_name, buffer_scaled, target_width, target_height);
        }
    }
    return -1;
}

int test_scales(int idx) {
    int result;
    result = simple_scale(200, 300, idx); // 500x600
    if (result != 0) return result;
    result = simple_scale(1000, 800, idx);  // 500x600
    if (result != 0) return result;
    return 0;
}

int main(void) {
//    return test_write_png();
//    return test_write_jpg();
//    return test_read_jpg_write_to_png();
//    return test_read_png_write_to_jpg();
//    return test_dump_image_type();
//    return test_convert_components();
//    return op_bmp_load();
//    return op_bmp_save();
//    return op_tga_load();
//    return op_tga_save();
//    return op_ppm3_read();
//    return op_ppm6_read();
//    return op_ppm3_write();
//    return op_ppm6_write();
//    return op_color_palette();
//    return op_color_palette2();
//    return test_read_gif_write_to_tga();
//    return data_prepare_read_gif_to_rgb();
//    return test_write_gif();
//    return test_scales(0); // no imgio_rgba_scale define, comment imgio_rgba_scale define in first line
//    return test_scales(1); // define imgio_rgba_scale before include image_io.h, comment imgio_scale_bicubic_s
//    return test_scales(4); // define imgio_rgba_scale and change imgio_scale_bicubic_s before include image_io.h
//    return 0;
}