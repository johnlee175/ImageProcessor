#ifndef PROJECT_ENV_CONFIG_H
#define PROJECT_ENV_CONFIG_H

#if defined(__cplusplus)
#define is_cpp_language
#elif defined(__OBJC__)
#define is_objc_language
#else
#define is_c_language
#endif

#if defined(_WIN32) || defined(_WIN64)
#define is_windows_os
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define is_macosx_os
#endif

#ifdef __linux__
#define is_linux_os
#endif

#if defined(__unix__) || defined(unix)
#define is_unix_os
#endif

#ifdef __GNUC__
#define is_gcc_compiler
#endif

#ifdef __clang__
#define is_clang_compiler
#endif

#ifdef _MSC_VER
#define is_msvc_compiler
#endif

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

#include <stdbool.h>
#include <stdint.h>
#define __restrict__ restrict
#define __inline__ static inline

#else /* !C99 */

typedef enum { false, true } bool;

#if __LP64__
#define __WORDSIZE 64
#else /* !__LP64__ */
#define __WORDSIZE 32
#endif /* __LP64__ */

/* 7.18.1.1 Exact-width integer types */
#ifdef C89
    #define	__const		const
    #define	__signed	signed
    #define	__volatile	volatile
#else	/* !C89 */

#ifndef __GNUC__
#define	__const
#define	__signed
#define	__volatile
#endif	/* __GNUC__ */

#ifndef	NO_ANSI_KEYWORDS
#define	const		__const
#define	signed		__signed
#define	volatile	__volatile
#endif /* NO_ANSI_KEYWORDS */

#endif /* C89 */

typedef	__signed char       int8_t;
typedef	short			    int16_t;
typedef	int			        int32_t;
typedef	long long		    int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* 7.18.2.1 Limits of exact-width integer types */
#define INT8_MAX         127
#define INT16_MAX        32767
#define INT32_MAX        2147483647
#define INT64_MAX        9223372036854775807LL

#define INT8_MIN          -128
#define INT16_MIN         -32768
#define INT32_MIN        (-INT32_MAX-1)
#define INT64_MIN        (-INT64_MAX-1)

#define UINT8_MAX         255
#define UINT16_MAX        65535
#define UINT32_MAX        4294967295U
#define UINT64_MAX        18446744073709551615ULL

/* 7.18.4 Macros for integer constants */
#define INT8_C(v)    (v)
#define INT16_C(v)   (v)
#define INT32_C(v)   (v)
#define INT64_C(v)   (v ## LL)

#define UINT8_C(v)   (v ## U)
#define UINT16_C(v)  (v ## U)
#define UINT32_C(v)  (v ## U)
#define UINT64_C(v)  (v ## ULL)

#define __restrict__
#define __inline__ static

#endif /* C99 */

/*
 * Example:
 * __pack_begin struct _A {
 *   char a;
 *  int b;
 *  char c;
 * } __pack_end;
 * typedef struct _A A;
 */
#ifdef __GNUC__
#define __pack_begin ;
#define __pack_end __attribute__ ((__packed__))
#else /* !__GNUC__ */
#define __pack_begin ; _Pragma("pack(push, 1)")
#define __pack_end ; _Pragma("pack(pop)")
#endif /* __GNUC__ */

#endif //PROJECT_ENV_CONFIG_H
