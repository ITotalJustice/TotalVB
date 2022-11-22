/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


void vb_init(struct VB_Core* vb);
void vb_reset(struct VB_Core* vb);
void vb_step(struct VB_Core* vb);

bool vb_loadrom(
  struct VB_Core* vb, const uint8_t* data, size_t size
);

bool vb_savestate(
  struct VB_Core* vb, struct VB_State* state
);

bool vb_loadstate(
  struct VB_Core* vb, const struct VB_State* state
);

const struct VB_RomHeader* vb_get_rom_header(
  const struct VB_Core* vb
);

const struct VB_RomHeader* vb_get_rom_header_from_data(
  const uint8_t* data, const size_t size
);

void vb_get_rom_title(
  const struct VB_Core* vb, struct VB_RomTitle* title
);

void vb_get_rom_title_from_header(
  const struct VB_RomHeader* header, struct VB_RomTitle* title
);

#ifdef __cplusplus
}
#endif
