#include "bmp.h"

__pack_begin struct tagBITMAPFILEHEADER
{
    uint16_t bfType; /* little-endian: "MB", 0x4D42 */
    uint32_t bfSize; /* file size in bytes */
    uint16_t bfReserved1; /* 0 */
    uint16_t bfReserved2; /* 0 */
    uint32_t bfOffBits; /* image data offset */
} __pack_end;
typedef struct tagBITMAPFILEHEADER BITMAPFILEHEADER;

__pack_begin struct tagBITMAPINFOHEADER
{
    uint32_t biSize; /* INFOHEADER size */
    int32_t biWidth;
    int32_t biHeight; /* +: Bottom2Top; -: Top2Bottom; */
    uint16_t biPlanes; /* 1 */
    uint16_t biBitCount; /* bpp */
    uint32_t biCompression; /* 0: RGB with no compression; 1: RLE8; 2: RLE4; 3: BITFIELDS; */
    uint32_t biSizeImage; /* image data size with 4 bytes-align */
    int32_t biXPelsPerMeter; /* H-resolution with pixel per meter */
    int32_t biYPelsPerMeter; /* V-resolution with pixel per meter */
    uint32_t biClrUsed; /* Using palette index count; 0(always default): use all index */
    uint32_t biClrImportant; /* Important palette index count; 0(always default): all important */
} __pack_end;
typedef struct tagBITMAPINFOHEADER BITMAPINFOHEADER;

int load_bmp_24_b2t(const char *bmp_path, uint8_t **rgb_buffer/* should user free */,
                    uint32_t *width, uint32_t *height) {
    BITMAPFILEHEADER bmp_file_header;
    FILE *file = fopen(bmp_path, "rb");
    fread(&bmp_file_header, sizeof(BITMAPFILEHEADER), (size_t) 1, file);
    BITMAPINFOHEADER bmp_info_header;
    fread(&bmp_info_header, sizeof(BITMAPINFOHEADER), (size_t) 1, file);

    if (bmp_info_header.biBitCount != 24 || bmp_info_header.biCompression != 0
        || bmp_info_header.biHeight < 0) {
        fclose(file);
        return -1;
    }

    fseek(file, bmp_file_header.bfOffBits, SEEK_SET);

    *width = bmp_info_header.biWidth < 0 ? 0 : (uint32_t) bmp_info_header.biWidth;
    *height = bmp_info_header.biHeight < 0 ? 0 : (uint32_t) bmp_info_header.biHeight;

    const int32_t row_stride = ((bmp_info_header.biWidth * bmp_info_header.biBitCount + 31) / 32) * 4;
    const int32_t row_count = bmp_info_header.biHeight;
    const size_t data_size = (const size_t) (row_stride * row_count);
    DEFINE_HEAP_ARRAY_POINTER(uint8_t, image_data, data_size, {
        fclose(file);
        return -1;
    });
    fread(image_data, sizeof(uint8_t), data_size, file);
    fclose(file);
    ASSIGN_HEAP_ARRAY_POINTER(uint8_t, *rgb_buffer, data_size, {
        fclose(file);
        free(image_data);
        return -1;
    });
    uint8_t *temp = *rgb_buffer;
    for (int32_t i = row_count - 1; i >= 0; --i) {
        for (int32_t j = 0; j < row_stride; j += 3) {
            *temp++ = image_data[i * row_stride + j + 2];
            *temp++ = image_data[i * row_stride + j + 1];
            *temp++ = image_data[i * row_stride + j];
        }
    }
    fclose(file);
    free(image_data);
    return 0;
}

int save_bmp_24_b2t(const char *bmp_path, const uint8_t *rgb_buffer/* should user free */,
                    const uint32_t width, const uint32_t height) {
    uint32_t file_header_size = sizeof(BITMAPFILEHEADER);
    uint32_t info_header_size = sizeof(BITMAPINFOHEADER);
    uint32_t header_size = file_header_size + info_header_size;
    uint32_t row_stride = ((width * 24 + 31) / 32) * 4;
    uint32_t data_size = row_stride * height;

    BITMAPFILEHEADER bmp_file_header;
    bmp_file_header.bfType = 0x4D42;
    bmp_file_header.bfSize = header_size + data_size;
    bmp_file_header.bfReserved1 = 0;
    bmp_file_header.bfReserved2 = 0;
    bmp_file_header.bfOffBits = header_size;

    BITMAPINFOHEADER bmp_info_header;
    bmp_info_header.biSize = info_header_size;
    bmp_info_header.biWidth = width;
    bmp_info_header.biHeight = height;
    bmp_info_header.biPlanes = 1;
    bmp_info_header.biBitCount = 24;
    bmp_info_header.biCompression = 0;
    bmp_info_header.biSizeImage = data_size;
    bmp_info_header.biClrUsed = 0;
    bmp_info_header.biClrImportant = 0;
    bmp_info_header.biXPelsPerMeter = 0; /* TODO need calculate? */
    bmp_info_header.biYPelsPerMeter = 0; /* TODO need calculate? */

    FILE *file = fopen(bmp_path, "wb");
    fwrite(&bmp_file_header, file_header_size, (size_t) 1, file);
    fwrite(&bmp_info_header, info_header_size, (size_t) 1, file);

    DEFINE_HEAP_ARRAY_POINTER(uint8_t, image_data, data_size, {
        fclose(file);
        return -1;
    });

    const uint8_t *temp = rgb_buffer;
    for (int32_t i = height - 1; i >= 0; --i) {
        for (uint32_t j = 0; j < row_stride; j += 3) {
            image_data[i * row_stride + j + 2] = *temp++;
            image_data[i * row_stride + j + 1] = *temp++;
            image_data[i * row_stride + j] = *temp++;
        }
    }
    fwrite(image_data, data_size, (size_t) 1, file);

    fclose(file);
    free(image_data);
    return 0;
}
