#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


// used mainly in debugging when i want to quickly silence
// the compiler about unsed vars.
#define VB_UNUSED(var) ((void)(var))

// ONLY use this for C-arrays, not pointers, not structs
#define VB_ARR_SIZE(array) (sizeof(array) / sizeof(array[0]))


#ifdef VB_DEBUG
#include <stdio.h>
#include <assert.h>
#define vb_log(...) fprintf(stdout, __VA_ARGS__)
#define vb_log_err(...) fprintf(stderr, __VA_ARGS__)
#else
#define vb_log(...)
#define vb_log_err(...)
#endif // VB_DEBUG


void vb_cpu_run(struct VB_Core* vb);


uint8_t vb_bus_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t vb_bus_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t vb_bus_read_32(struct VB_Core* vb, uint32_t addr);

void vb_bus_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void vb_bus_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void vb_bus_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

#ifdef __cplusplus
}
#endif