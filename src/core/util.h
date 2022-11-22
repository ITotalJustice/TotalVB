/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>


// static assert is part of c++11 and c11 (defined in assert.h)
// this is my own static_assert, just in case we are c99 bound
#if (defined(__cplusplus) && __cplusplus < 201103L) || (!defined(static_assert))
  #define static_assert(expr, msg) typedef char static_assertion[(expr) ? 1 : -1]
#endif


#if defined __has_builtin
  #define HAS_BUILTIN(x) __has_builtin(x)
#else
  #if defined(__GNUC__)
    #define HAS_BUILTIN(x) (1)
  #else
    #define HAS_BUILTIN(x) (0)
  #endif
#endif // __has_builtin


// don't use these with params that have side effects!
#define VB_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define VB_MAX(x, y) (((x) > (y)) ? (x) : (y))

// label variables unused
#define VB_UNUSED(var) ((void)(var))

// ONLY use this for C-arrays, not pointers, not structs
#define VB_ARR_SIZE(array) (sizeof(array) / sizeof(array[0]))


// force inline support
#if defined(_MSC_VER)
  #define VB_FORCE_INLINE inline __forceinline
#elif defined(__GNUC__)
  #define VB_FORCE_INLINE inline __attribute__((always_inline))
#elif defined(__clang__)
  #define VB_FORCE_INLINE inline __attribute__((always_inline))
#else
  #define VB_FORCE_INLINE inline
#endif


// usually helps improve the codegen a little bit
#if __has_builtin(__builtin_expect)
  #define VB_LIKELY(c) (__builtin_expect(c,1))
  #define VB_UNLIKELY(c) (__builtin_expect(c,0))
#else
  #define VB_LIKELY(c)
  #define VB_UNLIKELY(c)
#endif


// should only be used for functions that return a value but
// all cases inside a switch have been handled.
#if __has_builtin(__builtin_unreachable)
  #define VB_UNREACHABLE(x) __builtin_unreachable()
#elif defined (_MSC_VER)
  #define VB_UNREACHABLE(x) __assume(0)
#else
  #define VB_UNREACHABLE(x) return x
#endif


#ifdef VB_DEBUG
  #include <stdio.h>
  #include <assert.h>
  // #define vb_log(...) fprintf(stdout, __VA_ARGS__)
  #define vb_log(...)
  #define vb_log_err(...) fprintf(stderr, __VA_ARGS__)
  #define vb_log_fatal(...) do { fprintf(stderr, __VA_ARGS__); printf("count: [%zu]\n", vb->v810.step_count); assert(0); } while(0)
#else
  #define vb_log(...)
  #define vb_log_err(...)
  #define vb_log_fatal(...)
#endif // VB_DEBUG


#ifdef __cplusplus
}
#endif
