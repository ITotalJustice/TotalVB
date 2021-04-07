#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "core/types.h"


// used mainly in debugging when i want to quickly silence
// the compiler about unsed vars.
#define VB_UNUSED(var) ((void)(var))

// ONLY use this for C-arrays, not pointers, not structs
#define VB_ARR_SIZE(array) (sizeof(array) / sizeof(array[0]))


#ifdef VB_DEBUG
#include <stdio.h>
#include <assert.h>
#define VB_log(...) fprintf(stdout, __VA_ARGS__)
#define VB_log_err(...) fprintf(stderr, __VA_ARGS__)
#else
#define VB_log(...)
#define VB_log_err(...)
#endif // VB_DEBUG


void VB_mmio_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);


#ifdef __cplusplus
}
#endif