/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "core/vb.h"
#include "core/internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static struct VB_Core CORE = {0};

static uint8_t ROM_DATA[VB_MAX_ROM_SIZE];


static bool read_file(const char* path, uint8_t* out_buf, size_t* out_size) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    return false;
  }

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (size <= 0) {
    fclose(f);
    return false;
  }

  fread(out_buf, 1, size, f);
  *out_size = (size_t)size;
  fclose(f);

  return true;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("missing args\n");
    return 1;
  }

  vb_init(&CORE);

  size_t rom_size = 0;

  if (!read_file(argv[1], ROM_DATA, &rom_size)) {
    printf("failed to read file!\n");
    return 1;
  }

  if (!vb_loadrom(&CORE, ROM_DATA, rom_size)) {
    printf("failed to load rom!\n");
    return 1;
  }

  #define HZ (1000000)
  #define CYCLES_PER_FRAME ((20 * HZ) / 60) / 4
  #define STEP_COUNT CYCLES_PER_FRAME

  // 1000000
  // 3579545
  // 1769515
  // 4303630

  // 20000000 (per frame)

  for (int frame_count = 0; ;frame_count++)
  {
    vb_step(&CORE);
    if ((frame_count % 60) == 0) {
      printf("\nframe count: %d\n\n", frame_count);
    }
  }

  return 0;
}
