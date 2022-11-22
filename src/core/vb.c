/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "vb.h"
#include "internal.h"

// #include <stdio.h>
#include <string.h>
#include <assert.h>


static bool is_pow2(size_t size) {
  return (!(size & (size - 1)) && size);
}

static void log_header(const struct VB_RomHeader* header) {
  assert(header);

  struct VB_RomTitle title;
  vb_get_rom_title_from_header(header, &title);

  vb_log("\nHEADER LOG\n");
  vb_log("\tTITLE: %s\n", title.title);
  vb_log("\tMaker code: %.*s\n", 2, header->maker_code);
  vb_log("\tGame code: %.*s\n", 4, header->game_code);
  vb_log("\tVersion: 1.%u\n", header->version);
  vb_log("\n");
}

void vb_init(struct VB_Core* vb) {
  assert(vb);
  memset(vb, 0, sizeof(struct VB_Core));
}

void vb_reset(struct VB_Core* vb) {
  vb_v810_reset(vb);
  vb_vip_reset(vb);
  vb_vsu_reset(vb);
  vb_timer_reset(vb);

  const uint8_t deadbeef[8] = { 0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF };

  for (size_t i = 0; i < VB_ARR_SIZE(vb->wram); i++) {
    vb->wram[i] = deadbeef[i & 7];
  }

  vb->link.CCR = 0x6D; // 0b01101101
  vb->link.CCSR = 0xFF; // 0b11111111
  vb->link.CDTR = 0x00; // 0b00000000
  vb->link.CDRR = 0x00; // 0b00000000

  vb->pad.SDLR = 0x00; // 0b00000000
  vb->pad.SDHR = 0x00; // 0b00000000

  vb->timer.TLR = 0xFF; // 0b11111111
  vb->timer.THR = 0xFF; // 0b11111111
  vb->timer.TCR = 0xE4; // 0b11100100

  vb->pak.WCR = 0xFC; // 0b11111100
  vb->pak.SCR = 0x4C; // 0b01001100
}

const struct VB_RomHeader* vb_get_rom_header(const struct VB_Core* vb) {
  return vb_get_rom_header_from_data(vb->rom, vb->rom_size);
}

const struct VB_RomHeader* vb_get_rom_header_from_data(
  const uint8_t* data, const size_t size
) {
  // header should always start at the end of the rom area
  // this is (0x1000 - 0xDE0)
  return (const struct VB_RomHeader*)(data + (size - 544));
}

void vb_get_rom_title(
  const struct VB_Core* vb, struct VB_RomTitle* title
) {
  vb_get_rom_title_from_header(vb_get_rom_header(vb), title);
}

void vb_get_rom_title_from_header(
  const struct VB_RomHeader* header, struct VB_RomTitle* title
) {
  assert(header && title);

  memset(title, 0, sizeof(struct VB_RomTitle));

  for (size_t i = 0; i < VB_ARR_SIZE(header->title); ++i) {
    title->title[i] = header->title[i];
  }
}

bool vb_loadrom(
  struct VB_Core* vb, const uint8_t* data, size_t size
) {
  assert(vb && data && size);
  assert(is_pow2(size) && "rom has to be a power of 2");

  const struct VB_RomHeader* header = vb_get_rom_header_from_data(data, size);
  log_header(header);

  for (size_t i = 0; i < VB_ARR_SIZE(header->reserved); i++)
  {
    assert(header->reserved[i] == 0 && "reserved bytes in header should be zero!");
  }

  vb->rom = data;
  vb->rom_size = size;
  vb->rom_mask = size - 1;

  vb_reset(vb);

  return true;
}

bool vb_savestate(
  struct VB_Core* vb, struct VB_State* state
) {
  state->meta.magic = VB_StateMeta_MAGIC;
  state->meta.version = VB_StateMeta_VERSION;
  state->meta.size = VB_StateMeta_SIZE;
  state->meta.reserved = 0;

  memcpy(&state->v810, &vb->v810, sizeof(state->v810));
  memcpy(&state->vip, &vb->vip, sizeof(state->vip));
  memcpy(&state->vsu, &vb->vsu, sizeof(state->vsu));
  memcpy(&state->timer, &vb->timer, sizeof(state->timer));
  memcpy(&state->pad, &vb->pad, sizeof(state->pad));
  memcpy(&state->link, &vb->link, sizeof(state->link));
  memcpy(&state->pak, &vb->pak, sizeof(state->pak));
  memcpy(&state->wram, &vb->wram, sizeof(state->wram));

  return true;
}

bool vb_loadstate(
  struct VB_Core* vb, const struct VB_State* state
) {
  if (state->meta.magic != VB_StateMeta_MAGIC) {
    vb_log_err("[STATE] bad magic: got: 0x%08X want: 0x%08X\n", state->meta.magic, VB_StateMeta_MAGIC);
    return false;
  }
  if (state->meta.version != VB_StateMeta_VERSION) {
    vb_log_err("[STATE] bad version: got: 0x%08X want: 0x%08X\n", state->meta.version, VB_StateMeta_VERSION);
    return false;
  }
  if (state->meta.size != VB_StateMeta_SIZE) {
    vb_log_err("[STATE] bad size: got: 0x%08X want: 0x%08X\n", state->meta.size, VB_StateMeta_SIZE);
    return false;
  }
  if (state->meta.reserved != 0) {
    vb_log_err("[STATE] bad reserved: got: 0x%08X want: 0x%08X\n", state->meta.reserved, 0);
    return false;
  }

  memcpy(&vb->v810, &state->v810, sizeof(vb->v810));
  memcpy(&vb->vip, &state->vip, sizeof(vb->vip));
  memcpy(&vb->vsu, &state->vsu, sizeof(vb->vsu));
  memcpy(&vb->timer, &state->timer, sizeof(vb->timer));
  memcpy(&vb->pad, &state->pad, sizeof(vb->pad));
  memcpy(&vb->link, &state->link, sizeof(vb->link));
  memcpy(&vb->pak, &state->pak, sizeof(vb->pak));
  memcpy(&vb->wram, &state->wram, sizeof(vb->wram));

  return true;
}

#define HZ (1000000)
#define CYCLES_PER_FRAME ((20 * HZ) / 50)

void vb_step(struct VB_Core* vb) {
  uint8_t cycles = 4;

  for (size_t i = 0; i < CYCLES_PER_FRAME; i += cycles) {
    vb_v810_run(vb);
    vb_vip_run(vb, cycles);
    vb_vsu_run(vb, cycles);
    vb->v810.step_count++;
  }
}
