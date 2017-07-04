#ifndef PROJECT_BASE_H
#define PROJECT_BASE_H

#define PROJECT_PACKAGE_NAME com_johnsoft_alg

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>

#if defined(__STDC__)
#   define C89
#   if defined(__STDC_VERSION__)
#       define C90
#       if (__STDC_VERSION__ >= 199901L)
#           define C99
#           if (__STDC_VERSION__ >= 201112L)
#               define C11
#           endif
#       endif
#   endif
#endif

#ifdef C99
#define __restrict__ restrict
#define __inline__ static inline
#else
#define __restrict__
#define __inline__ static
#endif

#endif //PROJECT_BASE_H
