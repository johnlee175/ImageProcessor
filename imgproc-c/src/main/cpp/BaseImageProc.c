#include "BaseImageProc.h"

int32_t grey_to_color(int32_t avg) {
    if (avg > 255) { avg = 255; } if (avg < 0) { avg = 0; } // not (avg & 0xFF)
    return (255 << 24) + (avg << 16) + (avg << 8) + avg;
}

void calc_raw_grey(int32_t *result, int32_t *argb, int32_t size) {
    int32_t r, g, b;
    for (int32_t i = 0; i < size; ++i) {
        r = (argb[i] >> 16) & 0xFF;
        g = (argb[i] >> 8) & 0xFF;
        b = argb[i] & 0xFF;
        result[i] = (r * 30 + g * 59 + b * 11 + 50) / 100;
    }
}

void calc_grey_color(int32_t *result, int32_t *argb, int32_t size) {
    int32_t r, g, b, avg;
    for (int32_t i = 0; i < size; ++i) {
        r = (argb[i] >> 16) & 0xFF;
        g = (argb[i] >> 8) & 0xFF;
        b = argb[i] & 0xFF;
        avg = (r * 30 + g * 59 + b * 11 + 50) / 100;
        result[i] = grey_to_color(avg);
    }
}

void calc_grey_color_filter(int32_t *result, int32_t *argb, int32_t size,
                            reset_filter reset, component_filter grey_filter) {
    int32_t r, g, b, avg;
    for (int32_t i = 0; i < size; ++i) {
        r = (argb[i] >> 16) & 0xFF;
        g = (argb[i] >> 8) & 0xFF;
        b = argb[i] & 0xFF;
        avg = (r * 30 + g * 59 + b * 11 + 50) / 100;
        reset();
        avg = grey_filter(avg);
        result[i] = grey_to_color(avg);
    }
}

void calc_rgb_color_filter(int32_t *result, int32_t *argb, int32_t size, reset_filter reset,
                      component_filter r_filter, component_filter g_filter, component_filter b_filter) {
    int32_t r, g, b;
    for (int32_t i = 0; i < size; ++i) {
        r = (argb[i] >> 16) & 0xFF;
        g = (argb[i] >> 8) & 0xFF;
        b = argb[i] & 0xFF;
        reset();
        r = r_filter(r);
        reset();
        g = g_filter(g);
        reset();
        b = b_filter(b);
        if (r > 255) { r = 255; } if (r < 0) { r = 0; } // not (r & 0xFF)
        if (g > 255) { g = 255; } if (g < 0) { g = 0; } // not (g & 0xFF)
        if (b > 255) { b = 255; } if (b < 0) { b = 0; } // not (b & 0xFF)
        result[i] = (255 << 24) + (r << 16) + (g << 8) + b;
    }
}