/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>


// static assert is part of c++11 and c11 (defined in assert.h)
// this is my own static_assert, just in case we are c99 bound
#if (defined(__cplusplus) && __cplusplus < 201103L) || (!defined(static_assert))
  #if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
    #define static_assert _Static_assert
  #else
    #define static_assert(expr, msg) typedef char static_assertion[(expr) ? 1 : -1]
  #endif
#endif

// these functions are reasonabily optimised
// better codegen was genertaed for gcc when using `== (1U << bit)` in the
// bit_is_set function for whatever reason. clang gets it right either way.

// better codegen also for using bitfields for psw when writing a u32 to it.
// i have not yet looked at the overall asm when accessing psw flags as
// bitfields compared to normal, i imagine its more asm generated for the former.
// SEE: https://godbolt.org/z/769nd61E5
// SEE: https://godbolt.org/z/aW18Kr1fn

#define HELLO_WORLD
#ifndef HELLO_WORLD

static inline bool bit_is_set(const uint8_t bit, const uint32_t value) {
  assert(bit < (sizeof(uint32_t) * 8) && "bit value out of bounds!");
  return (value & (1U << bit)) == (1U << bit);
}

static inline uint32_t bit_get_range(const uint8_t start, const uint8_t end, const uint32_t value) {
  assert(end > start && "invalid bit range!");

  return (value & (0xFFFFFFFF >> (31 - end))) >> start;
}

static inline int32_t bit_sign_extend(const uint8_t start_size, const uint32_t value) {
  assert(start_size < 31 && "bit start size is out of bounds!");

  const uint8_t bits = 31 - start_size;
  return ((int32_t)(value << bits)) >> bits;
}

#else // NDEBUG

#define bit_is_set(bit, value) \
  (!!((value) & (1U << (bit))))

#define bit_get_range(start, end, value) \
  (((value) & (0xFFFFFFFF >> (31U - (end)))) >> (start))

#define bit_sign_extend(start_size, value) \
  (((int32_t)(((uint32_t)value) << (31U - (start_size)))) >> (31U - (start_size)))

static_assert(
  bit_is_set(0, 0x1) == true &&
  bit_is_set(1, 0x0) == false &&
  bit_is_set(1, 0x2) == true &&
  bit_is_set(0, 0x2) == false,
  "bit_is_set is broken!"
);

static_assert(
  bit_get_range(3, 5, 0x38) == 0x07 &&
  bit_get_range(0, 2, 0x02) == 0x02 &&
  bit_get_range(1, 5, 0x3E) == 0x1F &&
  bit_get_range(4, 5, 0x30) == 0x03,
  "bit_get_range is broken!"
);

static_assert(
  (uint32_t)(int32_t)(int8_t)0xFF == 0xFFFFFFFF,
  "sign extending via cast is broken"
);

static_assert(
  (uint32_t)(((int32_t)(0xFFU << 24)) >> 24) == 0xFFFFFFFF,
  "sign extending via asr is broken"
);

static_assert(
  // ensure that sign extending does work!
  (int8_t)0xFF == -1 &&
  (int32_t)(int8_t)0xFF == (int32_t)0xFFFFFFFF &&

  // same as above but using bit_sign_extend
  bit_sign_extend(7, 0xFF) == -1 &&
  bit_sign_extend(7, 0xFF) == (int32_t)0xFFFFFFFF &&

  // simple 24-bit asr
  bit_sign_extend(23, 0x00CFFFFF) == (int32_t)0xFFCFFFFF &&
  // set the sign-bit to bit 1, then asr 31-bits
  bit_sign_extend(0, 0x00000001) == (int32_t)0xFFFFFFFF &&
  // this is used in thumb ldr halword sign
  bit_sign_extend(15, 0x00E0FFFF) == (int32_t)0xFFFFFFFF &&
  // same as above but no sign
  bit_sign_extend(15, 0x00E07FFF) == (int32_t)0x00007FFF,
  "bit_sign_extend is broken!"
);

#endif // NDEBUG

#ifdef __cplusplus
}
#endif
