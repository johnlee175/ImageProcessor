#include "BaseImageProc.h"

int32_t grey_to_color(int32_t avg) {
    if (avg > 255) { avg = 255; } if (avg < 0) { avg = 0; } // not (avg & 0xFF)
    return (255 << 24) + (avg << 16) + (avg << 8) + avg;
}

int32_t color_to_grey(int32_t argb) {
    int32_t r, g, b;
    r = (argb >> 16) & 0xFF;
    g = (argb >> 8) & 0xFF;
    b = argb & 0xFF;
    return (r * 30 + g * 59 + b * 11 + 50) / 100;
}

int32_t assemble_color_int(int32_t r, int32_t g, int32_t b) {
    if (r > 255) { r = 255; } if (r < 0) { r = 0; } // not (r & 0xFF)
    if (g > 255) { g = 255; } if (g < 0) { g = 0; } // not (g & 0xFF)
    if (b > 255) { b = 255; } if (b < 0) { b = 0; } // not (b & 0xFF)
    return (255 << 24) + (r << 16) + (g << 8) + b;
}

void calc_raw_grey(int32_t *result, int32_t *argb, int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
        result[i] = color_to_grey(argb[i]);
    }
}

void calc_grey_color(int32_t *result, int32_t *argb, int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
        result[i] = grey_to_color(color_to_grey(argb[i]));
    }
}

void calc_grey_color_filter(int32_t *result, int32_t *argb, int32_t size,
                            reset_filter reset, component_filter grey_filter) {
    for (int32_t i = 0; i < size; ++i) {
        reset();
        result[i] = grey_to_color(grey_filter(color_to_grey(argb[i])));
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
        result[i] = assemble_color_int(r, g, b);
    }
}

void calc_grey_bit_plane(int32_t *result, int32_t *argb, int32_t size, int32_t bit_position) {
    int32_t  avg;
    for (int32_t i = 0; i < size; ++i) {
        avg = color_to_grey(argb[i]);
        if (((avg & 0xFF) & (1 << bit_position)) == 0) {
            result[i] = 0xFF000000;
        } else {
            result[i] = 0xFFFFFFFF;
        }
    }
}

void calc_rgb_bit_plane(int32_t *result, int32_t *argb, int32_t size, int32_t bit_position) {
    for (int32_t i = 0; i < size; ++i) {
        if (((argb[i] & (((int32_t) 1) << bit_position)) & 0xFFFFFFFF) == 0) {
            result[i] = 0xFF000000;
        } else {
            if (bit_position < 8) {
                result[i] = 0xFF0000FF;
            } else if (bit_position < 16) {
                result[i] = 0xFF00FF00;
            } else if (bit_position < 24) {
                result[i] = 0xFFFF0000;
            } else {
                result[i] = 0xFFFFFFFF;
            }
        }
    }
}

void calc_rgb_component_plane(int32_t *result, int32_t *argb, int32_t size, int32_t position, int32_t type) {
    int32_t comp;
    for (int32_t i = 0; i < size; ++i) {
        switch(position) {
            case 0: /* a */
                result[i] = 0xFF000000;
                break;
            case 1: /* r */
                comp = (argb[i] >> 16) & 0xFF;
                if (type == 0) {
                    result[i] = assemble_color_int(comp, 0, 0);
                } else {
                    result[i] = grey_to_color(comp);
                }
                break;
            case 2: /* g */
                comp = (argb[i] >> 8) & 0xFF;
                if (type == 0) {
                    result[i] = assemble_color_int(0, comp, 0);
                } else {
                    result[i] = grey_to_color(comp);
                }
                break;
            case 3: /* b */
                comp = argb[i] & 0xFF;
                if (type == 0) {
                    result[i] = assemble_color_int(0, 0, comp);
                } else {
                    result[i] = grey_to_color(comp);
                }
                break;
            default:
                result[i] = 0xFFFFFFFF;
                break;
        }
    }
}