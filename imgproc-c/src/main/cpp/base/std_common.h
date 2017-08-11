#ifndef PROJECT_COMMON_H
#define PROJECT_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>

#define base_info_log(...) ((void)fprintf(stdout, __VA_ARGS__))
#define base_error_log(...) ((void)fprintf(stderr, __VA_ARGS__))

#define ASSIGN_HEAP_ARRAY_POINTER(type, name, size, ...) \
    name = (type *) malloc(sizeof(type) * size); \
    if (name != NULL) { \
        memset(name, 0, sizeof(type) * size); \
    } else { \
        __VA_ARGS__ \
    } \
    do { } while(0)

#define DEFINE_HEAP_ARRAY_POINTER(type, name, size, ...) \
    type *name = (type *) malloc(sizeof(type) * size); \
    if (name != NULL) { \
        memset(name, 0, sizeof(type) * size); \
    } else { \
        __VA_ARGS__ \
    } \
    do { } while(0)

#define ASSIGN_HEAP_TYPE_POINTER(type, name, ...) \
    ASSIGN_HEAP_ARRAY_POINTER(type, name, 1, __VA_ARGS__)

#define DEFINE_HEAP_TYPE_POINTER(type, name, ...) \
    DEFINE_HEAP_ARRAY_POINTER(type, name, 1, __VA_ARGS__)

#define PARAM_IN
#define PARAM_OUT
#define PARAM_INOUT
#define OPTIONAL

#endif /* PROJECT_COMMON_H */
