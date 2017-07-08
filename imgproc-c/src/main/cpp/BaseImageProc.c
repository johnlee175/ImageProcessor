#include "BaseImageProc.h"

void calc_raw_grey(int32_t *result, const int32_t *__restrict__ argb, int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
        result[i] = color_to_grey(argb[i]);
    }
}

void calc_grey_color(int32_t *result, const int32_t *__restrict__ argb, int32_t size) {
    for (int32_t i = 0; i < size; ++i) {
        result[i] = grey_to_color(color_to_grey(argb[i]));
    }
}

void calc_grey_normal_filter(double *result, const double *__restrict__ argb, int32_t size,
                             reset_filter reset, component_filter grey_filter) {
    for (int32_t i = 0; i < size; ++i) {
        reset();
        result[i] = grey_filter((int32_t) (argb[i] * 255.0)) / 255.0;
    }
}

void calc_grey_color_filter(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                            reset_filter reset, component_filter grey_filter) {
    for (int32_t i = 0; i < size; ++i) {
        reset();
        result[i] = grey_to_color(grey_filter(color_to_grey(argb[i])));
    }
}

void calc_rgb_color_filter(int32_t *result, const int32_t *__restrict__ argb, int32_t size, reset_filter reset,
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

void calc_grey_bit_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size, int32_t bit_position) {
    int32_t avg;
    for (int32_t i = 0; i < size; ++i) {
        avg = color_to_grey(argb[i]);
        if (((avg & 0xFF) & (1 << bit_position)) == 0) {
            result[i] = 0xFF000000;
        } else {
            result[i] = 0xFFFFFFFF;
        }
    }
}

void calc_rgb_bit_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size, int32_t bit_position) {
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

void calc_rgb_component_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                              int32_t position, int32_t type) {
    int32_t comp;
    for (int32_t i = 0; i < size; ++i) {
        switch(position) {
            case COMPONENT_ALPHA:
                result[i] = 0xFF000000;
                break;
            case COMPONENT_RED:
                comp = (argb[i] >> 16) & 0xFF;
                if (type == TYPE_ARGB_COLOR) {
                    result[i] = assemble_color_int(comp, 0, 0);
                } else {
                    result[i] = grey_to_color(comp);
                }
                break;
            case COMPONENT_GREEN:
                comp = (argb[i] >> 8) & 0xFF;
                if (type == TYPE_ARGB_COLOR) {
                    result[i] = assemble_color_int(0, comp, 0);
                } else {
                    result[i] = grey_to_color(comp);
                }
                break;
            case COMPONENT_BLUE:
                comp = argb[i] & 0xFF;
                if (type == TYPE_ARGB_COLOR) {
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

void calc_combine_simple_plane(int32_t *source, const int32_t *__restrict__ target, int32_t size) {
    int32_t sr, sg, sb, tr, tg, tb;
    for (int32_t i = 0; i < size; ++i) {
        sr = (source[i] >> 16) & 0xFF;
        sg = (source[i] >> 8) & 0xFF;
        sb = source[i] & 0xFF;
        tr = (target[i] >> 16) & 0xFF;
        tg = (target[i] >> 8) & 0xFF;
        tb = target[i] & 0xFF;
        source[i] = assemble_color_int((tr == 0 ? sr : tr), (tg == 0 ? sg : tg), (tb == 0 ? sb : tb));
    }
}

void calc_combine_bits_plane(int32_t *result, const int32_t *__restrict__ argb, int32_t size,
                             int32_t type, int32_t mask) {
    for (int32_t i = 0; i < size; ++i) {
        if (type == TYPE_GREY_COLOR) {
            result[i] = grey_to_color(color_to_grey(argb[i]) & mask);
        } else {
            result[i] = argb[i] & mask;
        }
    }
}

void map_component_color_count(int32_t **result, int32_t  *result_size, const int32_t *__restrict__ argb, int32_t size,
                               int32_t type) {
    *result_size = 256;
    const size_t result_byte_size = *result_size * sizeof(int32_t);
    int32_t *map = (int32_t *) malloc(result_byte_size);
    memset(map, 0, result_byte_size);
    for (int32_t i = 0; i < size; ++i) {
        switch (type) {
            case COMPONENT_ALPHA:
                ++map[(argb[i] >> 24) & 0xFF];
                break;
            case COMPONENT_RED:
                ++map[(argb[i] >> 16) & 0xFF];
                break;
            case COMPONENT_GREEN:
                ++map[(argb[i] >> 8) & 0xFF];
                break;
            case COMPONENT_BLUE:
                ++map[argb[i] & 0xFF];
                break;
            default:
                ++map[color_to_grey(argb[i])];
                break;
        }
    }
    *result = map;
}

void histogram_equalization_classic(int32_t **result, const int32_t *__restrict__ hist, int32_t size,
                                   int32_t counts) {
    const size_t result_byte_size = size * sizeof(int32_t);
    int32_t *map = (int32_t *) malloc(result_byte_size);
    memset(map, 0, result_byte_size);
    double cum = 0.0;
    for (int32_t i = 0; i < size; ++i) {
        cum = cum + hist[i] / (double) counts;
        map[i] = (int32_t) round(cum * 255.0);
    }
    *result = map;
}

void histogram_equalization_photoshop(int32_t **result, const int32_t *__restrict__ hist, int32_t size,
                                      int32_t counts) {
    int32_t *map;
    histogram_equalization_classic(&map, hist, size, counts);
    int32_t min_val = map[0];
    double scale = 255.0 / (255.0 - min_val);
    for (int32_t i = 0; i < size; ++i) {
        map[i] = (int32_t) round((map[i] - min_val) * scale);
    }
    *result = map;
}

void map_component_equalization(int32_t **result, const int32_t *__restrict__ hist, int32_t hist_size,
                                const int32_t *__restrict__ argb, int32_t size, int32_t type) {
    const size_t result_byte_size = size * sizeof(int32_t);
    int32_t *map = (int32_t *) malloc(result_byte_size);
    memset(map, 0, result_byte_size);
    for (int32_t i = 0; i < size; ++i) {
        switch (type) {
            case COMPONENT_ALPHA:
                map[i] = (hist[(argb[i] >> 24) & 0xFF] << 24) | (argb[i] & 0x00FFFFFF);
                break;
            case COMPONENT_RED:
                map[i] = (hist[(argb[i] >> 16) & 0xFF] << 16) | (argb[i] & 0xFF00FFFF);
                break;
            case COMPONENT_GREEN:
                map[i] = (hist[(argb[i] >> 8) & 0xFF] << 8) | (argb[i] & 0xFFFF00FF);
                break;
            case COMPONENT_BLUE:
                map[i] = (hist[argb[i] & 0xFF]) | (argb[i] & 0xFFFFFF00);
                break;
            default:
                map[i] = grey_to_color(hist[color_to_grey(argb[i])]);
                break;
        }
    }
    *result = map;
}
