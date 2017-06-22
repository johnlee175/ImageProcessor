#include "BaseImageProc.h"


int32_t grey_to_greyColor(int32_t avg) {
    if (avg > 255) { avg = 255; } if (avg < 0) { avg = 0; } // not (avg & 0xFF)
    return (255 << 24) + (avg << 16) + (avg << 8) + avg;
}

void calc_grey(int32_t *pResult, int32_t *pArgb, int32_t size) {
    int32_t r, g, b;
    for (int i = 0; i < size; ++i) {
        r = (pArgb[i] >> 16) & 0xFF;
        g = (pArgb[i] >> 8) & 0xFF;
        b = pArgb[i] & 0xFF;
        pResult[i] = (r * 30 + g * 59 + b * 11 + 50) / 100;
    }
}

void calc_greyColor(int32_t *pResult, int32_t *pArgb, int32_t size) {
    int32_t r, g, b, avg;
    for (int i = 0; i < size; ++i) {
        r = (pArgb[i] >> 16) & 0xFF;
        g = (pArgb[i] >> 8) & 0xFF;
        b = pArgb[i] & 0xFF;
        avg = (r * 30 + g * 59 + b * 11 + 50) / 100;
        pResult[i] = grey_to_greyColor(avg);
    }
}

void calc_greyFilter(int32_t *pResult, int32_t *pArgb, int32_t size, component_filter filter) {
    int32_t r, g, b, avg;
    for (int i = 0; i < size; ++i) {
        r = (pArgb[i] >> 16) & 0xFF;
        g = (pArgb[i] >> 8) & 0xFF;
        b = pArgb[i] & 0xFF;
        avg = (r * 30 + g * 59 + b * 11 + 50) / 100;
        pResult[i] = grey_to_greyColor(filter(avg));
    }
}

void calc_colorFilter(int32_t *pResult, int32_t *pArgb, int32_t size, component_filter filter) {
    int32_t r, g, b;
    for (int i = 0; i < size; ++i) {
        r = (pArgb[i] >> 16) & 0xFF;
        g = (pArgb[i] >> 8) & 0xFF;
        b = pArgb[i] & 0xFF;
        r = filter(r);
        g = filter(g);
        b = filter(b);
        if (r > 255) { r = 255; } if (r < 0) { r = 0; } // not (r & 0xFF)
        if (g > 255) { g = 255; } if (g < 0) { g = 0; } // not (g & 0xFF)
        if (b > 255) { b = 255; } if (b < 0) { b = 0; } // not (b & 0xFF)
        pResult[i] = (255 << 24) + (r << 16) + (g << 8) + b;
    }
}