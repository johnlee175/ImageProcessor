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
#include "image_io.h"

static bool is_image_type(const uint8_t *start, const uint8_t *type, const int32_t byte_count) {
    bool is_type = true;
    for (int32_t i = 0; i < byte_count; ++i) {
        if (start[i] != type[i]) {
            is_type = false;
            break;
        }
    }
    return is_type;
}

static bool str_ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return (str_len >= suffix_len) && strcmp(str + (str_len - suffix_len), suffix) == 0;
}

imgio_type imgio_get_image_type(PARAM_IN const char *file_path, PARAM_IN  const bool check_file_extend) {
    FILE *file = fopen(file_path, "rb");
    uint8_t start[8];
    fread(start, sizeof(uint8_t), 8, file);

    uint8_t png[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    if (is_image_type(start, png, 8) && (check_file_extend ? str_ends_with(file_path, ".png") : true)) {
        fclose(file);
        return PNG;
    }

    uint8_t gif[4] = { 'G', 'I', 'F', '8' };
    if (is_image_type(start, gif, 4) && (check_file_extend ? str_ends_with(file_path, ".gif") : true)) {
        fclose(file);
        return GIF;
    }

    uint8_t bmp[2] = { 'B', 'M' };
    if (is_image_type(start, bmp, 2) && (check_file_extend ? str_ends_with(file_path, ".bmp") : true)) {
        fclose(file);
        return BMP;
    }

    uint8_t ppm_asc[3] = { 'P', '3', '\n' };
    uint8_t ppm_bin[3] = { 'P', '6', '\n' };
    if ((is_image_type(start, ppm_asc, 3) || is_image_type(start, ppm_bin, 3))
        && (check_file_extend ? str_ends_with(file_path, ".ppm") : true)) {
        fclose(file);
        return PPM;
    }

    uint8_t webp_start[4] = { 'R', 'I', 'F', 'F'};
    uint8_t webp_end[4] = { 'W', 'E', 'B', 'P'};
    uint8_t middle[4];
    fread(middle, sizeof(uint8_t), 4, file);
    if (is_image_type(start, webp_start, 4) && is_image_type(middle, webp_end, 4)
        && (check_file_extend ? str_ends_with(file_path, ".webp") : true)) {
        fclose(file);
        return WEBP;
    }

    uint8_t end[2];
    fseek(file, -2L,SEEK_END);
    fread(end, sizeof(uint8_t), 2, file);
    if (start[0] == 0xFF && start[1] == 0xD8 && end[0] == 0xFF && end[1] == 0xD9
        && (check_file_extend ? str_ends_with(file_path, ".jpg") : true)) {
        fclose(file);
        return JPEG;
    }

    return UNKNOWN;
}

void imgio_dump_image_type(PARAM_IN imgio_type image_type) {
    switch (image_type) {
        case JPEG:
            base_info_log("dump_image_type: jpeg\n");
            break;
        case PNG:
            base_info_log("dump_image_type: png\n");
            break;
        case GIF:
            base_info_log("dump_image_type: gif\n");
            break;
        case WEBP:
            base_info_log("dump_image_type: webp\n");
            break;
        case BMP:
            base_info_log("dump_image_type: bmp\n");
            break;
        case PPM:
            base_info_log("dump_image_type: ppm\n");
            break;
        case TGA:
            base_info_log("dump_image_type: tga\n");
            break;
        default:
            base_info_log("dump_image_type: unknown\n");
            break;
    }
}

int imgio_convert_rgb_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    for (uint32_t i = 0; i < dst_size; ++i) {
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = 255;
    }
    return 0;
}

int imgio_convert_rgba_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    for (uint32_t i = 0; i < dst_size; ++i) {
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
        src++;
    }
    return 0;
}

int imgio_convert_rgb_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    for (uint32_t i = 0; i < dst_size; ++i) {
        *dst++ = 255;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
    }
    return 0;
}

int imgio_convert_argb_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    for (uint32_t i = 0; i < dst_size; ++i) {
        src++;
        *dst++ = *src++;
        *dst++ = *src++;
        *dst++ = *src++;
    }
    return 0;
}

int imgio_convert_argb_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    uint8_t r, g, b, a;
    for (uint32_t i = 0; i < dst_size; ++i) {
        a = *src++;
        r = *src++;
        g = *src++;
        b = *src++;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
    return 0;
}

int imgio_convert_rgba_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    uint8_t a, r, g, b;
    for (uint32_t i = 0; i < dst_size; ++i) {
        r = *src++;
        g = *src++;
        b = *src++;
        a = *src++;
        *dst++ = a;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
    return 0;
}

int imgio_convert_bgr_to_rgb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    uint8_t r, g, b;
    for (uint32_t i = 0; i < dst_size; ++i) {
        b = *src++;
        g = *src++;
        r = *src++;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
    return 0;
}

int imgio_convert_abgr_to_rgba(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    uint8_t r, g, b, a;
    for (uint32_t i = 0; i < dst_size; ++i) {
        a = *src++;
        b = *src++;
        g = *src++;
        r = *src++;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
    return 0;
}

int imgio_convert_bgra_to_argb(PARAM_IN const uint8_t *src, PARAM_OUT uint8_t *dst, PARAM_IN const uint32_t dst_size) {
    uint8_t a, r, g, b;
    for (uint32_t i = 0; i < dst_size; ++i) {
        b = *src++;
        g = *src++;
        r = *src++;
        a = *src++;
        *dst++ = a;
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
    }
    return 0;
}

int imgio_read_ppm_p3_255(PARAM_IN const char *file_path,
                PARAM_OUT uint8_t **rgb_buffer/* user should free */,
                PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height) {
    FILE *pFile = fopen(file_path, "r");
    if (fgetc(pFile) != 'P' || fgetc(pFile) != '3') {
        fclose(pFile);
        return -1;
    }
    size_t image_size = 0;
    uint8_t *temp = NULL;
    bool begin_comment = false;
    bool begin_digit = false;
    const int32_t max_ppm3_digit_len = 16;
    char digit[max_ppm3_digit_len];
    memset(digit, '\0', max_ppm3_digit_len);
    int32_t idx = 0;
    uint32_t ptr = 0;
    char ch;
    while ((ch = (char) fgetc(pFile)) != EOF) {
        /* handle comment */
        if (ch == '#') {
            begin_comment = true;
            continue;
        }
        if (begin_comment) {
            if (ch == '\n') {
                begin_comment = false;
            }
            continue;
        }
        if (isdigit(ch)) {
            begin_digit = true;
            digit[idx++] = ch;
        } else {
            if (begin_digit) {
                begin_digit = false;
                if (ptr == 0) {
                    sscanf(digit, "%d", width);
                    ++ptr;
                } else if (ptr == 1) {
                    sscanf(digit, "%d", height);
                    ++ptr;
                } else if (ptr == 2) {
                    int32_t max_val;
                    sscanf(digit, "%d", &max_val);
                    if (max_val != 255) {
                        fclose(pFile);
                        return -1;
                    }
                    ++ptr;
                    image_size = *width * *height * 3;
                    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, *rgb_buffer, image_size, {
                        fclose(pFile);
                        return -1;
                    });
                    temp = *rgb_buffer;
                } else {
                    int32_t rgb_val;
                    sscanf(digit, "%d", &rgb_val);
                    *temp++ = (uint8_t) rgb_val;
                    ++ptr;
                }
                memset(digit, '\0', max_ppm3_digit_len);
                idx = 0;
            } else {
                continue;
            }
        }
    }
    if (begin_digit && temp != NULL) {
        int32_t rgb_val;
        sscanf(digit, "%d", &rgb_val);
        *temp = (uint8_t) rgb_val;
        ++ptr;
    }
    if (ptr != (image_size + 3)) {
        fclose(pFile);
        free(*rgb_buffer);
        return -1;
    }
    fclose(pFile);
    free(*rgb_buffer);
    return 0;
}

int imgio_read_ppm_p6_255(PARAM_IN const char *file_path,
                PARAM_OUT uint8_t **rgb_buffer/* user should free */,
                PARAM_OUT uint32_t *width, PARAM_OUT uint32_t *height) {
    FILE *pFile = fopen(file_path, "r");
    if (fgetc(pFile) != 'P' || fgetc(pFile) != '6') {
        fclose(pFile);
        return -1;
    }
    bool begin_comment = false;
    bool begin_digit = false;
    const int32_t max_ppm3_digit_len = 16;
    char digit[max_ppm3_digit_len];
    memset(digit, '\0', max_ppm3_digit_len);
    int32_t idx = 0, ptr = 0;
    char ch;
    while ((ch = (char) fgetc(pFile)) != EOF) {
        /* handle comment */
        if (ch == '#') {
            begin_comment = true;
            continue;
        }
        if (begin_comment) {
            if (ch == '\n') {
                begin_comment = false;
            }
            continue;
        }
        if (isdigit(ch)) {
            begin_digit = true;
            digit[idx++] = ch;
        } else {
            if (begin_digit) {
                begin_digit = false;
                if (ptr == 0) {
                    sscanf(digit, "%d", width);
                    ++ptr;
                } else if (ptr == 1) {
                    sscanf(digit, "%d", height);
                    ++ptr;
                } else if (ptr == 2) {
                    int32_t max_val;
                    sscanf(digit, "%d", &max_val);
                    if (max_val != 255) {
                        fclose(pFile);
                        return -1;
                    }
                    break;
                }
                memset(digit, '\0', max_ppm3_digit_len);
                idx = 0;
            } else {
                continue;
            }
        }
    }
    size_t image_size = *width * *height * 3;
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, *rgb_buffer, image_size, {
        fclose(pFile);
        return -1;
    });
    if (fread(*rgb_buffer, image_size, 1, pFile) != 1) {
        fclose(pFile);
        free(*rgb_buffer);
        return -1;
    }
    fclose(pFile);
    free(*rgb_buffer);
    return 0;
}

int imgio_write_ppm_p3_255(PARAM_IN const char *file_path,
                 PARAM_IN const uint8_t *rgb_buffer/* user should free */,
                 PARAM_IN const uint32_t width, PARAM_IN const uint32_t height) {
    size_t i, j, k;
    FILE *pFile = fopen(file_path, "w");
    fprintf(pFile, "P3\n");
    fprintf(pFile, "%d %d\n", width, height);
    fprintf(pFile, "255\n");
    k = 0;
    for(i = 0; i < width; ++i) {
        for(j = 0; j < height; ++j) {
            fprintf(pFile, "%u %u %u ", rgb_buffer[k], rgb_buffer[k + 1], rgb_buffer[k + 2]);
            k = k + 3;
        }
        fprintf(pFile, "\n");
    }
    fclose(pFile);
    return 0;
}

int imgio_write_ppm_p6_255(PARAM_IN const char *file_path,
                 PARAM_IN const uint8_t *rgb_buffer/* user should free */,
                 PARAM_IN const uint32_t width, PARAM_IN const uint32_t height) {
    FILE *pFile = fopen(file_path, "w");
    fprintf(pFile, "P6\n");
    fprintf(pFile, "%d %d\n", width, height);
    fprintf(pFile, "255\n");
    fwrite(rgb_buffer, width * height * 3, 1, pFile);
    fprintf(pFile, "\n");
    fclose(pFile);
    return 0;
}

int imgio_read_png(PARAM_IN const char *file_path, PARAM_OUT png_bytep *buffer/* user should free */,
             PARAM_OUT png_uint_32 *img_width, PARAM_OUT png_uint_32 *img_height,
             PARAM_OUT png_uint_32 *format) {
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&image, file_path) != 0) {
        *format = image.format;
        *img_width = image.width;
        *img_height = image.height;
        int32_t bpp = 3;
        switch (*format) {
            case PNG_FORMAT_RGBA:
            case PNG_FORMAT_ABGR:
            case PNG_FORMAT_ARGB:
            case PNG_FORMAT_BGRA:
            case PNG_FORMAT_LINEAR_RGB_ALPHA:
                bpp = 4;
                break;
            default:
                break;
        }
        ASSIGN_HEAP_ARRAY_POINTER(png_byte, *buffer, image.width * image.height * bpp, {
            return -1;
        });
        if (png_image_finish_read(&image, NULL/*background*/, *buffer,
                                  0/*row_stride*/, NULL/*colormap*/)) {
            return 0;
        }
    }
    return -1;
}

int imgio_write_png(PARAM_IN const char *file_path, PARAM_IN const png_bytep pixels,
              PARAM_IN const png_uint_32 img_width, PARAM_IN const png_uint_32 img_height,
              PARAM_IN const bool has_alpha) {
    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.width = img_width;
    image.height = img_height;
    image.format =  has_alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB;
    image.flags = 0;
    image.warning_or_error = 0;
    image.colormap_entries = 0;
    image.message[0] = '\0';
    image.opaque = NULL;

    int result_code = png_image_write_to_file(&image, file_path,
                                   0/*convert_to_8bit*/, pixels, 0/*row_stride*/, NULL/*colormap*/);
    return result_code > 0 ? 0 : -1;
}

int imgio_read_png2(PARAM_IN const char *file_path, PARAM_OUT png_bytep *buffer/* user should free */,
              PARAM_OUT png_uint_32 *width, PARAM_OUT png_uint_32 *height) {
    FILE *fp;
    if ((fp = fopen(file_path, "rb")) == NULL) return -1;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return -1;
    }

    /* Set error handling if you are using the setjmp/longjmp method (this is
     * the normal method of doing things with libpng).  REQUIRED unless you
     * set up your own error handlers in the png_create_read_struct() earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    int sig_read = 0;
    png_set_sig_bytes(png_ptr, sig_read);

    int bit_depth, color_type, interlace_type;

    png_read_info(png_ptr, info_ptr);
    png_get_IHDR(png_ptr, info_ptr, width, height, &bit_depth, &color_type,
                 &interlace_type, NULL, NULL);

    /* Expand paletted colors into true RGB triplets */
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    /* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    /* Expand paletted or RGB images with transparency to full alpha channels
     * so the data will be available as RGBA quartets.
     */
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) != 0) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    /* Set the background color to draw transparent and alpha images over.
     * It is possible to set the red, green, and blue components directly
     * for paletted images instead of supplying a palette index.  Note that
     * even if the PNG file supplies a background, you are not required to
     * use it - you should use the (solid) application background if it has one.
     */
    png_color_16 my_background, *image_background;
    if (png_get_bKGD(png_ptr, info_ptr, &image_background) != 0) {
        png_set_background(png_ptr, image_background, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
    } else {
        png_set_background(png_ptr, &my_background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
    }

    double_t screen_gamma = 2.2;
    int intent;
    if (png_get_sRGB(png_ptr, info_ptr, &intent) != 0) {
        png_set_gamma(png_ptr, screen_gamma, PNG_DEFAULT_sRGB);
    } else {
        double_t image_gamma;
        if (png_get_gAMA(png_ptr, info_ptr, &image_gamma) != 0) {
            png_set_gamma(png_ptr, screen_gamma, image_gamma);
        } else {
            png_set_gamma(png_ptr, screen_gamma, 0.45455);
        }
    }

    /* Optional call to gamma correct and add the background to the palette
     * and update info structure.  REQUIRED if you are expecting libpng to
     * update the palette for you (ie you selected such a transform above).
     */
    png_read_update_info(png_ptr, info_ptr);

    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    ASSIGN_HEAP_ARRAY_POINTER(png_byte, *buffer, (row_bytes * *height), {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return -1;
    });

    for (png_uint_32 y = 0; y < *height; y++) {
        png_read_rows(png_ptr, NULL, buffer, 1);
        *buffer += row_bytes;
    }

    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

    return 0;
}

int imgio_write_png2(PARAM_IN const char *file_name, PARAM_IN const png_bytep image,
               PARAM_IN const png_uint_32 width, PARAM_IN const png_uint_32 height,
               PARAM_IN const bool use_interlace, PARAM_IN const png_uint_32 dpi, PARAM_IN const bool has_alpha) {
    FILE *fp = fopen(file_name, "wb");
    if (fp == NULL) return -1;

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct(&png_ptr,  NULL);
        fclose(fp);
        return -1;
    }

    /* Set error handling.  REQUIRED if you aren't supplying your own
     * error handling functions in the png_create_write_struct() call.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, has_alpha ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                 use_interlace ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* Set the physical pixel size (resolution, given in DPI). */
    if (dpi > 0) {
        /* one metre = 100 centimetre (cm), and 2.54 cm = 1 inch */
        /* 1 metre is about 40 inches (well, 100/2.54 or 39.37) */
        /* so the number of dots per metre is about 40 times */
        /* larger than the number of dots per inch */
        /* thus DPM = DPI * 100 / 2.54 = DPI * 10000 / 254 */
        png_uint_32 ppm_x, ppm_y; /* pixels per metre */
        ppm_x = (png_uint_32) ((dpi * 10000 + 127) / 254); /* round to nearest */
        ppm_y = ppm_x;
        png_set_pHYs(png_ptr, info_ptr, ppm_x, ppm_y,
                     PNG_RESOLUTION_METER);
    }

    png_color_8 sig_bit;
    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    /* Optional gamma chunk is strongly suggested if you have any guess
     * as to the correct gamma of the image.
     */
    png_set_gAMA(png_ptr, info_ptr, 2.2);

    /* Other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs.
     * Note that if sRGB is present the gAMA and cHRM chunks must be ignored
     * on read and, if your application chooses to write them, they must
     * be written in accordance with the sRGB profile
     */

    png_write_info(png_ptr, info_ptr);

    /* Shift the pixels up to a legal bit depth and fill in
     * as appropriate to correctly scale the image.
     */
    png_set_shift(png_ptr, &sig_bit);
    /* Pack pixels into bytes */
    png_set_packing(png_ptr);

    int number_passes;
    if (use_interlace != 0)
        number_passes = png_set_interlace_handling(png_ptr);
    else
        number_passes = 1;

    png_bytep row_pointers[height];

    if (height > PNG_UINT_32_MAX / (sizeof (png_bytep))) {
        png_error(png_ptr, "Image is too tall to process in memory");
    }

    /* Set up pointers into your "image" byte array */
    png_uint_32 bpp = has_alpha ? 4 : 3;
    for (png_uint_32 k = 0; k < height; k++) {
        row_pointers[k] = image + k * width * bpp;
    }

    /* The number of passes is either 1 for non-interlaced images,
     * or 7 for interlaced images.
     */
    for (int pass = 0; pass < number_passes; pass++) {
        for (png_uint_32 y = 0; y < height; y++) {
            png_write_rows(png_ptr, &row_pointers[y], 1);
        }
    }

    /* You can write optional chunks like tEXt, zTXt, and tIME at the end
     * as well.  Shouldn't be necessary in 1.2.0 and up as all the public
     * chunks are supported and you can use png_set_unknown_chunks() to
     * register unknown chunks into the info structure to be written out.
     */

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

int imgio_write_jpeg(PARAM_IN const char *filename, PARAM_IN const JDIMENSION quality/* 1 ~ 100 */,
                    PARAM_IN const JSAMPLE *image_rgb_buffer,
                    PARAM_IN const JDIMENSION image_width, PARAM_IN const JDIMENSION image_height) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    FILE * outfile;
    if ((outfile = fopen(filename, "wb")) == NULL) {
        base_error_log("can't open %s\n", filename);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

    jpeg_start_compress(&cinfo, TRUE);

    JDIMENSION row_stride = image_width * 3;

    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = (JSAMPROW) &image_rgb_buffer[cinfo.next_scanline * row_stride];
        (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);

    return 0;
}

int imgio_read_jpeg(PARAM_IN char * filename, PARAM_OUT JSAMPLE **image_rgb_buffer,
                   PARAM_OUT JDIMENSION *width, PARAM_OUT JDIMENSION *height, PARAM_OUT JDIMENSION *component) {
    FILE * infile;
    if ((infile = fopen(filename, "rb")) == NULL) {
        base_error_log("can't open %s\n", filename);
        return -1;
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    *width = cinfo.output_width;
    *height = cinfo.output_height;
    *component = (JDIMENSION) cinfo.output_components;

    JDIMENSION row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    const size_t size_in_bytes = row_stride * cinfo.output_height;
    ASSIGN_HEAP_ARRAY_POINTER(JSAMPLE, *image_rgb_buffer, size_in_bytes, {
        base_error_log("malloc data failed!");
        fclose(infile);
        return -1;
    });
    JSAMPLE *tmp = *image_rgb_buffer;

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(tmp, *buffer, row_stride);
        tmp += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return 0;
}

static void parse_extension_block(int32_t extension_block_count, ExtensionBlock *extension_block_ptr,
                                  GifWord *curr_delay_time, GifWord *curr_disposal_mode,
                                  GifWord *curr_trans_color_idx, GifWord *global_loop_times) {
    bool has_app_extension = false;
    for (int32_t x = 0, eb_count = extension_block_count; x < eb_count; ++x) {
        ExtensionBlock extension_block = extension_block_ptr[x];
        if (extension_block.Function == GRAPHICS_EXT_FUNC_CODE && extension_block.ByteCount == 4) {
            if (extension_block.Bytes[0] & 0x01) {
                *curr_trans_color_idx = extension_block.Bytes[3];
            }
            *curr_delay_time = extension_block.Bytes[1] | (extension_block.Bytes[2] << 8);
            *curr_disposal_mode = (extension_block.Bytes[0] >> 2) & 0x07;
        } else if (extension_block.Function == APPLICATION_EXT_FUNC_CODE
                   && extension_block.ByteCount == 11
                   && !memcmp((const char*) (extension_block.Bytes), "NETSCAPE2.0", 11)) {
            has_app_extension = true;
        } else if (has_app_extension && extension_block.Function == CONTINUE_EXT_FUNC_CODE
                   && extension_block.ByteCount == 3
                   && extension_block.Bytes[0] == 1) {
            has_app_extension = false;
            *global_loop_times = extension_block.Bytes[1] | (extension_block.Bytes[2] << 8);
        }
    }
}

int imgio_read_gif(PARAM_IN char * filename, PARAM_IN bool make_alpha, PARAM_IN func_on_gif_frame on_gif_frame) {
    const GifWord component = make_alpha ? 4 : 3;
    int error;

    GifFileType *gif_file = DGifOpenFileName(filename, &error);
    if (gif_file == NULL) {
        base_error_log("gif open: %s\n", GifErrorString(error));
        return -1;
    }
    if ((error = DGifSlurp(gif_file)) != GIF_OK) {
        base_error_log("gif slurp: %s\n", GifErrorString(error));
        return -1;
    }

    GifPicture picture;
    GifWord image_count = picture.image_count = gif_file->ImageCount;
    GifWord screen_width = picture.screen_width = gif_file->SWidth;
    GifWord screen_height = picture.screen_height = gif_file->SHeight;
    const GifWord background_color_idx = gif_file->SBackGroundColor;
    const size_t  screen_size = (size_t) (screen_width * screen_height);
    ColorMapObject *global_color_map = gif_file->SColorMap;
    DEFINE_HEAP_ARRAY_POINTER(GifByteType, rgb_buffer, screen_size * component, {
        base_error_log("gif buffer malloc failed\n");
        return -1;
    });

    GifWord curr_delay_time = 0;
    GifWord curr_trans_color_idx = NO_TRANSPARENT_COLOR;
    GifWord curr_disposal_mode = DISPOSAL_UNSPECIFIED;
    parse_extension_block(gif_file->ExtensionBlockCount, gif_file->ExtensionBlocks,
                          &curr_delay_time, &curr_disposal_mode, &curr_trans_color_idx, &picture.anim_loop_times);

    for (GifWord i = 0; i < image_count; ++i) {
        SavedImage *curr_image = gif_file->SavedImages;
        ColorMapObject *local_color_map = curr_image->ImageDesc.ColorMap;
        ColorMapObject *curr_color_map = local_color_map == NULL ? global_color_map : local_color_map;
        GifWord local_left = curr_image->ImageDesc.Left;
        GifWord local_top = curr_image->ImageDesc.Top;
        GifWord local_width = curr_image->ImageDesc.Width;
        GifWord local_height = curr_image->ImageDesc.Height;
        GifColorType bg_color = curr_color_map->Colors[background_color_idx];
        GifColorType trans_color = curr_color_map->Colors[curr_trans_color_idx];

        parse_extension_block(curr_image->ExtensionBlockCount, curr_image->ExtensionBlocks,
                              &curr_delay_time, &curr_disposal_mode, &curr_trans_color_idx, &picture.anim_loop_times);
        /* c99 feature */
        GifFrame frame = {
                .image_index = i,
                .per_pixel_bytes = component,
                .rgb_buffer = rgb_buffer,
                .delay_time = curr_delay_time,
                .disposal_mode = curr_disposal_mode,
                .background_color = bg_color,
                .transparent_color = trans_color,
                .left = local_left,
                .top = local_top,
                .image_width = local_width,
                .image_height = local_height
        };

        GifByteType *temp = rgb_buffer;
        GifColorType color;
        for (GifWord y = 0; y < screen_height; ++y) {
            for (GifWord x = 0; x < screen_width; ++x) {
                if (x < local_left || y < local_top) {
                    color = bg_color;
                } else {
                    color = curr_color_map->Colors[curr_image->RasterBits[y * screen_width + x]];
                }
                *temp++ = color.Red;
                *temp++ = color.Green;
                *temp++ = color.Blue;
                if (component == 4) {
                    *temp++ = 255;
                }
            }
        }

        on_gif_frame(&picture, &frame);
        ++gif_file->SavedImages;
    }

    free(rgb_buffer);
    return 0;
}

int imgio_write_gif(PARAM_IN const char * filename, PARAM_IN const GifDataModelInterface *interface,
                    PARAM_IN const GifWord loop_count, PARAM_IN const GifColorType *background_color) {
    if (!check_GifDataModelInterface(interface)) {
        base_error_log("interface not implements correctly!\n");
        return -1;
    }

    const GifWord frame_count = interface->frame_count();
    if (frame_count < 1) {
        base_error_log("frame not enough!\n");
        return -1;
    }

    int error;
    GifFileType *gif_file = EGifOpenFileName(filename, false, &error);
    if (gif_file == NULL) {
        base_error_log("target file open failed with code %d!\n", error);
        return -1;
    }
    EGifSetGifVersion(gif_file, true);

    ColorQuantization *quantization = create_color_quantization();

    GifWord max_width = 0, max_height = 0;
    for (GifWord x = 0; x < frame_count; ++x) {
        interface->make_current_frame(x);

        const GifWord curr_frame_width = interface->current_frame_width();
        const GifWord curr_frame_height = interface->current_frame_height();
        GifByteType *const curr_frame_buffer = interface->current_frame_buffer();

        if (max_width < curr_frame_width) {
            max_width = curr_frame_width;
        }
        if (max_height < curr_frame_height) {
            max_height = curr_frame_height;
        }

        prepare_color_palette(quantization, curr_frame_buffer, ((uint32_t) curr_frame_width * curr_frame_height),
                              256, x == frame_count - 1);

        interface->release_current_frame();
    }

    uint8_t *palette = NULL;
    generate_color_palette(quantization, &palette);
    if (palette == NULL) {
        base_error_log("generate color palette failed!\n");
        EGifCloseFile(gif_file, &error);
        destroy_color_quantization(quantization);
        return -1;
    }

    ColorMapObject *color_map_object = GifMakeMapObject(256, NULL);
    color_map_object->ColorCount = 0;
    for (int32_t i = 0; i < 256 * 3; i += 3) {
        color_map_object->Colors[color_map_object->ColorCount].Red = palette[i];
        color_map_object->Colors[color_map_object->ColorCount].Green = palette[i + 1];
        color_map_object->Colors[color_map_object->ColorCount++].Blue = palette[i + 2];
    }

    free(palette);

    int background_color_index = 0;
    if (background_color != NULL) {
        background_color_index = index_of_palette(quantization, background_color->Red,
                                                  background_color->Green, background_color->Blue);
        if (background_color_index < 0) {
            background_color_index = 0;
        }
    }

    EGifPutScreenDesc(gif_file, max_width, max_height, 8, background_color_index, color_map_object);
    GifFreeMapObject(color_map_object);

    static const GifByteType app_bytes[11] = { 'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0' };
    GifByteType app_sub_bytes[3];
    app_sub_bytes[0] = 0x01;
    app_sub_bytes[1] = (GifByteType) (loop_count & 0xFF);
    app_sub_bytes[2] = (GifByteType) ((loop_count >> 8) & 0xFF);
    EGifPutExtensionLeader(gif_file, APPLICATION_EXT_FUNC_CODE);
    EGifPutExtensionBlock(gif_file, 11, app_bytes);
    EGifPutExtensionBlock(gif_file, 3, app_sub_bytes);
    EGifPutExtensionTrailer(gif_file);

    GraphicsControlBlock gcb;
    for (GifWord x = 0; x < frame_count; ++x) {
        interface->make_current_frame(x);

        const GifWord curr_frame_width = interface->current_frame_width();
        const GifWord curr_frame_height = interface->current_frame_height();
        GifByteType *const curr_frame_buffer = interface->current_frame_buffer();

        gcb.UserInputFlag = false;
        if (interface->current_frame_delay_time == NULL) {
            gcb.DelayTime = 100;
        } else {
            gcb.DelayTime = interface->current_frame_delay_time();
        }
        if (interface->current_frame_disposal_mode == NULL) {
            gcb.DisposalMode = DISPOSE_PREVIOUS;
        } else {
            gcb.DisposalMode = interface->current_frame_disposal_mode();
        }
        int transparent_color_index = NO_TRANSPARENT_COLOR;
        if (interface->current_frame_transparent_color != NULL) {
            GifColorType transparent_color = interface->current_frame_transparent_color();
            transparent_color_index = index_of_palette(quantization, transparent_color.Red,
                                                      transparent_color.Green, transparent_color.Blue);
            if (transparent_color_index < 0) {
                transparent_color_index = NO_TRANSPARENT_COLOR;
            }
        }
        gcb.TransparentColor = transparent_color_index;
        GifByteType gcb_bytes[4];
        EGifGCBToExtension(&gcb, gcb_bytes);
        EGifPutExtension(gif_file, GRAPHICS_EXT_FUNC_CODE, 4, gcb_bytes);

        const GifWord left = (max_width - curr_frame_width) / 2;
        const GifWord top = (max_height - curr_frame_height) / 2;
        EGifPutImageDesc(gif_file, left, top, curr_frame_width, curr_frame_height,
                         false, NULL);
        GifWord array_index;
        int32_t pixel_index;
        for (GifWord i = 0; i < curr_frame_height; ++i) {
            for (GifWord j = 0; j < curr_frame_width * 3; j += 3) {
                array_index = i * curr_frame_width * 3 + j;
                pixel_index = index_of_palette(quantization,
                                               curr_frame_buffer[array_index],
                                               curr_frame_buffer[array_index + 1],
                                               curr_frame_buffer[array_index + 2]);
                EGifPutPixel(gif_file, (const GifPixelType) pixel_index);
            }
        }

        interface->release_current_frame();
    }

    EGifCloseFile(gif_file, &error);
    destroy_color_quantization(quantization);
    return 0;
}

/*
 * f(x,y)=f(0,0)(1-x)(1-y)+f(1,0)(1-y)x+f(0,1)(1-x)y+f(1,1)xy;
 * f(i+u,j+v) = (1-u)*(1-v)*f(i,j) + (1-u)*v*f(i,j+1) + u*(1-v)*f(i+1,j) + u*v*f(i+1,j+1);
 */
int imgio_rgba_scale_bilinear(PARAM_IN const uint32_t component /* per pixel in bytes */,
                              PARAM_IN const uint8_t *_restrict_ buffer_src,
                              PARAM_INOUT uint8_t *_restrict_ buffer_dst,
                              PARAM_IN const uint32_t width_src, PARAM_IN const uint32_t height_src,
                              PARAM_IN const uint32_t width_dst, PARAM_IN const uint32_t height_dst) {
    double_t x_src, y_src;
    int32_t x_left, x_right, y_top, y_bottom;
    double_t fx_left, fx_right, fy_top, fy_bottom;
    double_t s1, s2, s3, s4;
    uint32_t index_dst, index_src1, index_src2, index_src3, index_src4;

    for (uint32_t i = 0; i < height_dst; ++i) {
        for (uint32_t j = 0; j < width_dst; ++j) {
            x_src = j * ((width_src + 0.5) / (width_dst - 0.5));
            y_src = i * ((height_src + 0.5) / (height_dst - 0.5));

            x_left = (int32_t) x_src; x_right = x_left + 1;
            y_top = (int32_t) y_src; y_bottom = y_top + 1;

            fx_left = x_src - x_left; fx_right = 1.0 - fx_left;
            fy_top = y_src - y_top; fy_bottom = 1.0 - fy_top;

            s1 = fx_left * fy_top; s2 = fx_right * fy_top;
            s3 = fx_right * fy_bottom; s4 = fx_left * fy_bottom;

            index_dst = i * (width_dst * component) + (j * component);
            index_src1 = y_bottom * (width_src * component) + (x_right * component);
            index_src2 = y_bottom * (width_src * component) + (x_left * component);
            index_src3 = y_top * (width_src * component) + (x_left * component);
            index_src4 = y_top * (width_src * component) + (x_right * component);
            for (uint32_t k = 0; k < component; ++k) {
                buffer_dst[index_dst + k] = (uint8_t) (buffer_src[index_src1 + k] * s1
                                                       + buffer_src[index_src2 + k] * s2
                                                       + buffer_src[index_src3 + k] * s3
                                                       + buffer_src[index_src4 + k] * s4);
            }
        }
    }
    return 0;
}

/*
S(a, x) = {
        y = abs(x);
        if 0<=y<1; then return (a+2)*y^3 - (a+3)*y^2 + 1;
		if 1<=y<2; then return a*y^3 - 5*a*y^2 + 8*a*y - 4*a;
		else return 0;
}
S(-1, x) = {
		y = abs(x);
		if 0<=y<1; then return 1 - 2*y^2 + y^3;
		if 1<=y<2; then return 4 - 8*y + 5*y^2 - y^3;
		else return 0;
}

[A] = [S(u+1) S(u+0) S(u-1) S(u-2)]

[B] = [ f(i-1,j-1) f(i-1,j+0) f(i-1,j+1) f(i-1,j+2)
		f(i+0,j-1) f(i+0,j+0) f(i+0,j+1) f(i+0,j+2)
		f(i+1,j-1) f(i+1,j+0) f(i+1,j+1) f(i+1,j+2)
		f(i+2,j-1) f(i+2,j+0) f(i+2,j+1) f(i+2,j+2) ]

[C] = [ S(v+1)
		S(v+0)
		S(v-1)
		S(v-2) ]

[A] * [B] * [C] = (S(u+1)*f(i-1,j-1) + S(u+0)*f(i+0,j-1) + S(u-1)*f(i+1,j-1) + S(u-2)*f(i+2,j-1)) * S(v+1)
			  	+ (S(u+1)*f(i-1,j+0) + S(u+0)*f(i+0,j+0) + S(u-1)*f(i+1,j+0) + S(u-2)*f(i+2,j+0)) * S(v+0)
			  	+ (S(u+1)*f(i-1,j+1) + S(u+0)*f(i+0,j+1) + S(u-1)*f(i+1,j+1) + S(u-2)*f(i+2,j+1)) * S(v-1)
			  	+ (S(u+1)*f(i-1,j+2) + S(u+0)*f(i+0,j+2) + S(u-1)*f(i+1,j+2) + S(u-2)*f(i+2,j+2)) * S(v-2)

f(i+u,j+v) = [A] * [B] * [C]
*/
int imgio_rgba_scale_bicubic_convolution(PARAM_IN const uint32_t component /* per pixel in bytes */,
                             PARAM_IN const uint8_t *_restrict_ buffer_src,
                             PARAM_INOUT uint8_t *_restrict_ buffer_dst,
                             PARAM_IN const uint32_t width_src, PARAM_IN const uint32_t height_src,
                             PARAM_IN const uint32_t width_dst, PARAM_IN const uint32_t height_dst,
                             PARAM_IN imgio_scale_bicubic_s_func s_func) {
#define F(x, y, idx) (buffer_src[(int32_t)((y) * (width_src * component) + ((x) * component) + (idx))])

    double_t x_src, y_src;
    double_t x_int, y_int, x_float, y_float;
    uint32_t index_dst;
    double_t ab0, ab1, ab2, ab3;
    double_t fx;

    for (uint32_t i = 0; i < height_dst; ++i) {
        for (uint32_t j = 0; j < width_dst; ++j) {
            x_src = j * ((width_src + 0.5) / (width_dst - 0.5));
            y_src = i * ((height_src + 0.5) / (height_dst - 0.5));

            x_int = floor(x_src); y_int = floor(y_src);
            x_float = x_src - x_int; y_float = y_src - y_int;

            index_dst = i * (width_dst * component) + (j * component);

            for (uint32_t k = 0; k < component; ++k) {
                ab0 = s_func(x_float + 1.0) * F(x_int - 1, y_int - 1, k)
                      + s_func(x_float + 0.0) * F(x_int + 0, y_int - 1, k)
                      + s_func(x_float - 1.0) * F(x_int + 1, y_int - 1, k)
                      + s_func(x_float - 2.0) * F(x_int + 2, y_int - 1, k);
                ab1 = s_func(x_float + 1.0) * F(x_int - 1, y_int + 0, k)
                      + s_func(x_float + 0.0) * F(x_int + 0, y_int + 0, k)
                      + s_func(x_float - 1.0) * F(x_int + 1, y_int + 0, k)
                      + s_func(x_float - 2.0) * F(x_int + 2, y_int + 0, k);
                ab2 = s_func(x_float + 1.0) * F(x_int - 1, y_int + 1, k)
                      + s_func(x_float + 0.0) * F(x_int + 0, y_int + 1, k)
                      + s_func(x_float - 1.0) * F(x_int + 1, y_int + 1, k)
                      + s_func(x_float - 2.0) * F(x_int + 2, y_int + 1, k);
                ab3 = s_func(x_float + 1.0) * F(x_int - 1, y_int + 2, k)
                      + s_func(x_float + 0.0) * F(x_int + 0, y_int + 2, k)
                      + s_func(x_float - 1.0) * F(x_int + 1, y_int + 2, k)
                      + s_func(x_float - 2.0) * F(x_int + 2, y_int + 2, k);

                fx = (ab0 * s_func(y_float + 1) + ab1 * s_func(y_float + 0)
                           + ab2 * s_func(y_float - 1) + ab3 * s_func(y_float - 2));
                buffer_dst[index_dst + k] = (uint8_t) (fx > 255.0 ? 255 : fx < 0.0 ? 0 : fx);
            }
        }
    }
    return 0;

#undef F
}

/* S(a,x) */
double_t imgio_scale_bicubic_s_custom(double_t a, double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y >= 0.0 && y < 1.0) {
        return (a + 2.0) * pow(y, 3.0) - (a + 3.0) * pow(y, 2.0) + 1.0;
    } else if (y >= 1.0 && y < 2.0) {
        return a * pow(y, 3.0) - 5.0 * a * pow(y, 2.0) + 8.0 * a * y - 4.0 * a;
    }
    return 0.0;
}

/* S(-1,x) */
double_t imgio_scale_bicubic_s_default(double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y >= 0.0 && y < 1.0) {
        return 1.0 - 2.0 * pow(y, 2.0) + pow(y, 3.0);
    } else if (y >= 1.0 && y < 2.0) {
        return 4.0 - 8.0 * y + 5.0 * pow(y, 2.0) - pow(y, 3.0);
    }
    return 0.0;
}

double_t imgio_scale_bicubic_s_bell(double_t x) {
    double_t y = (x / 2.0) * 1.5;
    if (y >= -1.5 && y <= -0.5) {
        return 0.5 * pow(y + 1.5, 2.0);
    } else if (y > -0.5 && y <= 0.5) {
        return 0.75 - pow(y, 2.0);
    } else if (y > 0.5 && y <= 1.5) {
        return 0.5 * pow(y - 1.5, 2.0);
    }
    return 0.0;
}

double_t imgio_scale_bicubic_s_bspline(double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y >= 0.0 && y <= 1.0) {
        return 2.0 / 3.0 + 1.0 / 2.0 * pow(y, 3.0) - pow(y, 2.0);
    } else if (y > 1.0 && y <= 2.0) {
        return 1.0 / 6.0 * pow(2.0 - y, 3.0);
    }
    return 0.0;
}

double_t imgio_scale_bicubic_s_lagrange(double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y < 1.0) {
        return 1.0 / 2.0 * pow(y, 3.0) - pow(y, 2.0) - 1.0 / 2.0 * y + 1.0;
    } else if (y < 2.0) {
        return -1.0 / 6.0 * pow(y, 3.0) + pow(y, 2.0) - 11.0 / 6.0 * y + 1.0;
    }
    return 0.0;
}

double_t imgio_scale_bicubic_s_6param(double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y <= 1.0) {
        return 1.2 * pow(y, 3.0) - 2.2 * pow(y, 2.0) + 1.0;
    } else if (y < 2.0) {
        return 2.8 - 0.6 * pow(y, 3.0) + 3.2 * pow(y, 2.0) - 5.4 * y;
    } else if (y < 3.0) {
        return 0.2 * pow(y, 3.0) - 1.6 * pow(y, 2.0) + 4.2 * y - 3.6;
    }
    return 0.0;
}

double_t imgio_scale_bicubic_s_8param(double_t x) {
    double_t y = (x >= 0.0 ? x : -x);
    if (y <= 1.0) {
        return 67.0 / 56.0 * pow(y, 3.0) - 123.0 / 56.0 * pow(y, 2.0) + 1.0;
    } else if (y <= 2.0) {
        return -33.0 / 56.0 * pow(y, 3.0) + 177.0 / 56.0 * pow(y, 2.0) - 75.0 / 14.0 * y + 39.0 / 14.0;
    } else if (y <= 3.0) {
        return 9.0 / 56.0 * pow(y, 3.0) - 75.0 / 56.0 * pow(y, 2.0) + 51.0 / 14.0 * y - 45.0 / 14.0;
    } else if (y <= 4.0) {
        return -3.0 / 56.0 * pow(y, 3.0) + 33.0 / 56.0 * pow(y, 2.0) - 15.0 / 7.0 * y + 18.0 / 7.0;
    }
    return 0.0;
}


