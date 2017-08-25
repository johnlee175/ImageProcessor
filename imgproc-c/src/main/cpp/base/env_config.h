#ifndef PROJECT_ENV_CONFIG_H
#define PROJECT_ENV_CONFIG_H

#if defined(__cplusplus)
#   define is_cpp_language
#elif defined(__OBJC__)
#   define is_objc_language
#else
#   define is_c_language
#endif

#if defined(_WIN32) || defined(_WIN64)
#   define is_windows_os
#endif

#if defined(__APPLE__) || defined(__MACH__)
#   include <TargetConditionals.h>
#   if TARGET_OS_IPHONE == 1 || TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_EMBEDDED == 1
#       define is_ios_os
#   elif TARGET_OS_MAC == 1
#       define is_macosx_os
#   else
#       define is_apple_os
#   endif
#endif

#if defined(__linux__)
#   define is_linux_os
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#define is_android_os
#endif

#if defined(__unix__) || defined(unix) || defined(__unix)
#   define is_unix_os
#endif

#if defined(__MINGW32__) || defined(__MINGW64__)
#   define is_mingw
#endif

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#   define is_cygwin
#endif

#if defined(__GNUC__)
#   define is_gcc_compiler
#endif

#if defined(__clang__)
#   define is_clang_compiler
#endif

#if defined(_MSC_VER)
#   define is_msvc_compiler
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define is_best_compiler
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

#if defined(C99)

#include <stdbool.h>
#include <math.h>
#include <stdint.h>

#if defined(is_cpp_language)
#   define _restrict_
#else /* !is_cpp_language */
#   define _restrict_ restrict
#endif /* defined(is_cpp_language) */

#if defined(is_best_compiler)
#   define _inline_ static inline __attribute__((always_inline))
#else /* !is_best_compiler */
#   define _inline_ static inline
#endif /* defined(is_best_compiler) */

#else /* !C99 */

#if !defined(is_cpp_language)
    typedef enum { false, true } bool;
#endif /* !defined(is_cpp_language) */

#if !defined(__FLT_EVAL_METHOD__)
    typedef float float_t;
    typedef double double_t;
#else /* __FLT_EVAL_METHOD__ */
#   if __FLT_EVAL_METHOD__ == 1
        typedef double float_t;
        typedef double double_t;
#   elif __FLT_EVAL_METHOD__ == 2 || __FLT_EVAL_METHOD__ == -1
        typedef long double float_t;
        typedef long double double_t;
#   else /* __FLT_EVAL_METHOD__ not in [1, 2, -1] */
        typedef float float_t;
        typedef double double_t;
#   endif /* __FLT_EVAL_METHOD__ == 1 */
#endif /* !defined(__FLT_EVAL_METHOD__) */

#if !defined(__WORDSIZE)
#   if defined(__LP64__) && __LP64__
#       define __WORDSIZE 64
#   else /* !__LP64__ */
#       define __WORDSIZE 32
#   endif /* defined(__LP64__) && __LP64__ */
#endif /* !defined(__WORDSIZE) */

/* 7.18.1.1 Exact-width integer types */
#if defined(C89)
#   define	__const		const
#   define	__signed	signed
#   define	__volatile	volatile
#else /* !C89 */
#   if !definded(is_best_compiler)
#       define	__const
#       define	__signed
#       define	__volatile
#   endif /* !definded(is_best_compiler) */
#   if !defined(NO_ANSI_KEYWORDS)
#       define	const		__const
#       define	signed		__signed
#       define	volatile	__volatile
#   endif /* !defined(NO_ANSI_KEYWORDS) */
#endif /* defined(C89) */

typedef	__signed char       int8_t;
typedef	short			    int16_t;
typedef	int			        int32_t;
typedef	long long		    int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* 7.18.2.1 Limits of exact-width integer types */
#if !defined(INT8_MAX)
#define INT8_MAX         127
#define INT16_MAX        32767
#define INT32_MAX        2147483647
#define INT64_MAX        9223372036854775807LL
#endif

#if !defined(INT8_MIN)
#define INT8_MIN          -128
#define INT16_MIN         -32768
#define INT32_MIN        (-INT32_MAX-1)
#define INT64_MIN        (-INT64_MAX-1)
#endif

#if !defined(UINT8_MAX)
#define UINT8_MAX         255
#define UINT16_MAX        65535
#define UINT32_MAX        4294967295U
#define UINT64_MAX        18446744073709551615ULL
#endif

/* 7.18.4 Macros for integer constants */
#if !defined(INT8_C)
#define INT8_C(v)    (v)
#define INT16_C(v)   (v)
#define INT32_C(v)   (v)
#define INT64_C(v)   (v ## LL)
#endif

#if !defined(UINT8_C)
#define UINT8_C(v)   (v ## U)
#define UINT16_C(v)  (v ## U)
#define UINT32_C(v)  (v ## U)
#define UINT64_C(v)  (v ## ULL)
#endif

#if defined(is_cpp_language)
#   define _restrict_
#elif defined(is_best_compiler)
#   define _restrict_ __restrict__
#elif defined(is_msvc_compiler)
#   define _restrict_ __restrict
#else /* !is_msvc_compiler && !is_best_compiler && !is_cpp_language */
#   define _restrict_
#endif /* defined(is_cpp_language) */

#if defined(is_best_compiler)
#   define _inline_ static __inline__ __attribute__((always_inline))
#elif defined(is_msvc_compiler)
#   define _inline_ static __inline
#else /* !is_msvc_compiler && !is_best_compiler */
#   define _inline_ static
#endif /* defined(is_best_compiler) */

#endif /* defined(C99) */

/*
 * Example:
 * __pack_begin struct _A {
 *   char a;
 *  int b;
 *  char c;
 * } __pack_end;
 * typedef struct _A A;
 */
#if defined(is_best_compiler)
#   define __pack_begin ;
#   define __pack_end __attribute__ ((packed))
#else /* !is_best_compiler */
#   define __pack_begin ; _Pragma("pack(push, 1)")
#   define __pack_end ; _Pragma("pack(pop)")
#endif /* defined(is_best_compiler) */

#if defined(is_best_compiler)
#       define __cdecl__ __attribute__((cdecl))
#       define __stdcall__ __attribute__((stdcall))
#else /* !is_best_compiler */
#       define __cdecl__ _cdecl
#       define __stdcall__ _stdcall
#endif /* defined(is_best_compiler) */

#if defined(is_msvc_compiler)
#   define EXPORT __declspec(dllexport)
#   define NO_EXPORT
#   define IMPORT __declspec(dllimport)
#elif defined(is_best_compiler)
#   define EXPORT __attribute__((visibility("default"))
#   define NO_EXPORT __attribute__((visibility("hidden")))
#   define IMPORT
#else /* !is_best_compiler && !is_msvc_compiler */
#   define EXPORT
#   define NO_EXPORT
#   define IMPORT
#endif /* defined(is_msvc_compiler) */

#if defined(is_msvc_compiler)
#define DEPRECATED __declspec(deprecated)
#elif defined(is_best_compiler)
#define DEPRECATED __attribute__ ((deprecated))
#else /* !is_best_compiler && !is_msvc_compiler */
#define DEPRECATED
#endif /* defined(is_msvc_compiler) */

#if defined(is_cpp_language)
#define	__BEGIN_DECLS extern "C" {
#define	__END_DECLS }
#else /* !is_cpp_language */
#define	__BEGIN_DECLS
#define	__END_DECLS
#endif /* defined(is_cpp_language) */

#if !defined(_static_cast)
#   define _static_cast(x) (x)
#endif /* !defined(_static_cast) */
#if !defined(_const_cast)
#   define _const_cast(x) (x)
#endif /* !defined(_const_cast) */
#if !defined(_reinterpret_cast)
#   define _reinterpret_cast(x) (x)
#endif /* !defined(_reinterpret_cast) */

#if defined(is_cpp_language)
#   if !defined(__static_cast)
#       define __static_cast(x, y) static_cast<x>(y)
#   endif /* !defined(__static_cast) */
#   if !defined(__const_cast)
#       define __const_cast(x, y) const_cast<x>(y)
#   endif /* !defined(__const_cast) */
#   if !defined(__reinterpret_cast)
#       define __reinterpret_cast(x, y) reinterpret_cast<x>(y)
#   endif /* !defined(__reinterpret_cast) */
#else /* !is_cpp_language */
#   if !defined(__static_cast)
#       define __static_cast(x, y) (x)y
#   endif /* !defined(__static_cast) */
#   if !defined(__const_cast)
#       define __const_cast(x, y) (x)y
#   endif /* !defined(__const_cast) */
#   if !defined(__reinterpret_cast)
#       define __reinterpret_cast(x, y) (x)y
#   endif /* !defined(__reinterpret_cast) */
#endif /* defined(is_cpp_language) */

#if !defined(__UNCONST)
#define __UNCONST(a) ((void *)(unsigned long)(const void *)(a))
#endif /* !defined(__UNCONST) */

#if !defined(is_best_compiler)
#define __attribute__(x) /*NO-THING*/
#endif

#if defined(is_best_compiler)
#define _nonnull(...) __attribute__((nonnull (__VA_ARGS__)))
#else /* !is_best_compiler */
#define _nonnull(...) /*NO-THING*/
#endif /* defined(is_best_compiler) */

#if defined(is_android_os)
#   include <android/log.h>
#   if !defined(LOGI)
#       define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "System.out", __VA_ARGS__))
#   endif /* !defined(LOGI) */
#   if !defined(LOGW)
#       define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "System.err", __VA_ARGS__))
#   endif /* !defined(LOGW) */
#else /* !is_android_os */
#   if !defined(LOGI)
#       define LOGI(...) ((void)fprintf(stdout, __VA_ARGS__))
#   endif /* !defined(LOGI) */
#   if !defined(LOGW)
#       define LOGW(...) ((void)fprintf(stderr, __VA_ARGS__))
#   endif /* !defined(LOGW) */
#endif /* defined(is_android_os) */

#define base_info_log(...) LOGI(__VA_ARGS__)
#define base_error_log(...) LOGW(__VA_ARGS__)

#endif /* PROJECT_ENV_CONFIG_H */
