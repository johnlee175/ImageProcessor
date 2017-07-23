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

#define PARAM_IN
#define PARAM_OUT
#define PARAM_INOUT

#endif //PROJECT_COMMON_H
