/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "vb.h"
#include "internal.h"

#include <string.h>
#include <assert.h>


enum MemoryMap {
  VIP_BEG           = 0x00000000, // VIP - Virtual Image Processor
  VIP_END           = 0x00FFFFFF, // VIP - Virtual Image Processor
  VSU_BEG           = 0x01000000, // VSU - Virtual Sound Unit
  VSU_END           = 0x01FFFFFF, // VSU - Virtual Sound Unit
  MISC_BEG          = 0x02000000, // Miscellaneous Hardware
  MISC_END          = 0x02FFFFFF, // Miscellaneous Hardware
  UNMAP_BEG         = 0x03000000, // Unmapped
  UNMAP_END         = 0x03FFFFFF, // Unmapped
  PAK_EXPANSION_BEG = 0x04000000, // Game Pak Expansion
  PAK_EXPANSION_END = 0x04FFFFFF, // Game Pak Expansion
  WRAM_BEG          = 0x05000000, // WRAM
  WRAM_END          = 0x05FFFFFF, // WRAM
  PAK_RAM_BEG       = 0x06000000, // Game Pak RAM
  PAK_RAM_END       = 0x06FFFFFF, // Game Pak RAM
  PAK_ROM_BEG       = 0x07000000, // Game Pak ROM
  PAK_ROM_END       = 0x07FFFFFF, // Game Pak ROM
  // 0x08000000 - 0xFFFFFFFF Mirroring of memory map
};

// All IO ports are to be accessed as 8-bit values, but
// are spaced 4-bytes apart each, allowing for any access.
enum HwIO {
  IO_CCR   = 0x02000000, // Link  Communication Control Register
  IO_CCSR  = 0x02000004, // Link  COMCNT Control Register
  IO_CDTR  = 0x02000008, // Link  Transmitted Data Register
  IO_CDRR  = 0x0200000C, // Link  Received Data Register
  IO_SDLR  = 0x02000010, // Game Pad  Serial Data Low Register
  IO_SDHR  = 0x02000014, // Game Pad  Serial Data High Register
  IO_TLR   = 0x02000018, // Timer Timer Counter Low Register
  IO_THR   = 0x0200001C, // Timer Timer Counter High Register
  IO_TCR   = 0x02000020, // Timer Timer Control Register
  IO_WCR   = 0x02000024, // Game Pak  Wait Control Register
  IO_SCR   = 0x02000028, // Game Pad  Serial Control Register
  // 0x02000040 - 0x02FFFFFF Mirroring of hardware component memory map
};


// NOTE: these r/w arrays assume little endian host
// use bswap if porting to big endian platform!
static inline uint8_t read_array8(
  const uint8_t* array, const uint32_t addr, const uint32_t mask
) {
  uint8_t value;
  memcpy(&value, array + (addr & mask), sizeof(value));
  return value;
}

static inline uint16_t read_array16(
  const uint8_t* array, const uint32_t addr, const uint32_t mask
) {
  uint16_t value;
  memcpy(&value, array + (addr & mask), sizeof(value));
  return value;
}

static inline void write_array8(
  uint8_t* array, const uint32_t addr, const uint8_t value, const uint32_t mask
) {
  memcpy(array + (addr & mask), &value, sizeof(value));
}

static inline void write_array16(
  uint8_t* array, const uint32_t addr, const uint16_t value, const uint32_t mask
) {
  memcpy(array + (addr & mask), &value, sizeof(value));
}


static uint8_t io_read(struct VB_Core* vb, uint32_t addr) {
  #define IO_ADDR(a) ((a >> 2) & 0xF)

  static const uint8_t rmasks[] = {
    [IO_ADDR(IO_CCR)]   = 0xFF,
    [IO_ADDR(IO_CCSR)]  = 0xFF,
    [IO_ADDR(IO_CDTR)]  = 0xFF,
    [IO_ADDR(IO_CDRR)]  = 0xFF,
    [IO_ADDR(IO_SDLR)]  = 0xFF,
    [IO_ADDR(IO_SDHR)]  = 0xFF,
    [IO_ADDR(IO_TLR)]   = 0xFF,
    [IO_ADDR(IO_THR)]   = 0xFF,
    [IO_ADDR(IO_TCR)]   = 0xFF,
    [IO_ADDR(IO_WCR)]   = 0xFF,
    [IO_ADDR(IO_SCR)]   = 0xFF,
  };

  static const uint8_t omasks[] = {
    [IO_ADDR(IO_CCR)]   = 0x69, // 0b01101001
    [IO_ADDR(IO_CCSR)]  = 0x60, // 0b01100000
    [IO_ADDR(IO_CDTR)]  = 0x00,
    [IO_ADDR(IO_CDRR)]  = 0x00,
    [IO_ADDR(IO_SDLR)]  = 0x00,
    [IO_ADDR(IO_SDHR)]  = 0x00,
    [IO_ADDR(IO_TLR)]   = 0x00,
    [IO_ADDR(IO_THR)]   = 0x00,
    [IO_ADDR(IO_TCR)]   = 0xE0, // 0b11100000
    [IO_ADDR(IO_WCR)]   = 0xFC, // 0b11111100
    [IO_ADDR(IO_SCR)]   = 0x48, // 0b01001000
  };

  const uint8_t mask = rmasks[IO_ADDR(addr)];
  const uint8_t or_mask = omasks[IO_ADDR(addr)];

  switch (IO_ADDR(addr)) {
    case IO_ADDR(IO_CCR):
      vb_log_fatal("[IO] read CCR Link Communication Control Registern\n");
      // return (vb->io.CCR & mask) | or_mask;

    case IO_ADDR(IO_CCSR):
      vb_log_fatal("[IO] read CCSR Link COMCNT Control Register\n");
      // return (vb->io.CCSR & mask) | or_mask;

    case IO_ADDR(IO_CDTR):
      vb_log_fatal("[IO] read CDTR Link Transmitted Data Register\n");
      // return (vb->io.CDTR & mask) | or_mask;

    case IO_ADDR(IO_CDRR):
      vb_log_fatal("[IO] read CDRR Link Received Data Register\n");
      // return (vb->io.CDRR & mask) | or_mask;

    case IO_ADDR(IO_SDLR):
      vb_log_fatal("[IO] read SDLR Game Pad Serial Data Low Register\n");
      // return (vb->io.SDLR & mask) | or_mask;

    case IO_ADDR(IO_SDHR):
      vb_log_fatal("[IO] read SDHR Game Pad Serial Data High Register\n");
      // return (vb->io.SDHR & mask) | or_mask;

    case IO_ADDR(IO_TLR):
      vb_log_fatal("[IO] read TLR Timer Counter Low Register\n");
      // return (vb->io.TLR & mask) | or_mask;

    case IO_ADDR(IO_THR):
      vb_log_fatal("[IO] read THR Timer Counter High Register\n");
      // return (vb->io.THR & mask) | or_mask;

    case IO_ADDR(IO_TCR):
      vb_log_fatal("[IO] read TCR Timer Control Register\n");
      // return (vb->io.TCR & mask) | or_mask;

    case IO_ADDR(IO_WCR):
      vb_log_fatal("[IO] read WCR Game Pak Wait Control Register\n");
      // return (vb->io.WCR & mask) | or_mask;

    case IO_ADDR(IO_SCR):
      vb_log_fatal("[IO] read SCR Game Pad Serial Control Register\n");
      // return (vb->io.SCR & mask) | or_mask;
  }

  #undef IO_ADDR

  return 0xFF;
}

static void io_write(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  #define IO_ADDR(addr) ((addr >> 2) & 0xF)

  switch (IO_ADDR(addr)) {
    case IO_ADDR(IO_CCR):
      printf("[IO] write CCR Link Communication Control Registe addr: 0x%08Xrn value: 0x%02X\n", addr, value);
      // vb->io.CCR = value;
      break;

    case IO_ADDR(IO_CCSR):
      printf("[IO] write CCSR Link COMCNT Control Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.CCSR = value;
      break;

    case IO_ADDR(IO_CDTR):
      vb_log_fatal("[IO] write CDTR Link Transmitted Data Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.CDTR = value;
      break;

    case IO_ADDR(IO_CDRR):
      vb_log_fatal("[IO] write CDRR Link Received Data Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.CDRR = value;
      break;

    case IO_ADDR(IO_SDLR):
      vb_log_fatal("[IO] write SDLR Game Pad Serial Data Low Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.SDLR = value;
      break;

    case IO_ADDR(IO_SDHR):
      vb_log_fatal("[IO] write SDHR Game Pad Serial Data High Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.SDHR = value;
      break;

    case IO_ADDR(IO_TLR):
      printf("[IO] write TLR Timer Counter Low Register addr: 0x%08X value: 0x%02X\n", addr, value);
      vb->timer.TLR = value;
      break;

    case IO_ADDR(IO_THR):
      printf("[IO] write THR Timer Counter High Register addr: 0x%08X value: 0x%02X\n", addr, value);
      vb->timer.THR = value;
      break;

    case IO_ADDR(IO_TCR):
      printf("[IO] write TCR Timer Control Register addr: 0x%08X value: 0x%02X\n", addr, value);
      vb->timer.TCR = value;
      break;

    case IO_ADDR(IO_WCR):
      printf("[IO] write WCR Game Pak Wait Control Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.WCR = value;
      break;

    case IO_ADDR(IO_SCR):
      printf("[IO] write SCR Game Pad Serial Control Register addr: 0x%08X value: 0x%02X\n", addr, value);
      // vb->io.SCR = value;
      break;
  }

  #undef IO_ADDR
}


static uint8_t game_exp_read_8(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);

  assert(!"non impl read!");
  return 0xFF;
}

static uint16_t game_exp_read_16(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);

  assert(!"non impl read!");
  return 0xFF;
}

static void game_exp_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

static void game_exp_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}


static uint8_t wram_read_8(struct VB_Core* vb, uint32_t addr) {
  return read_array8(vb->wram, addr, 0xFFFF);
}

static uint16_t wram_read_16(struct VB_Core* vb, uint32_t addr) {
  assert(!(addr & 0x1) && "unaligned addr in wram_read_16!");
  return read_array16(vb->wram, addr, 0xFFFF);
}

static void wram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  write_array8(vb->wram, addr, value, 0xFFFF);
}

static void wram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  assert(!(addr & 0x1) && "unaligned addr in wram_write_16!");
  write_array16(vb->wram, addr, value, 0xFFFF);
}


static uint8_t game_ram_read_8(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);

  assert(!"non impl read!");
  return 0xFF;
}

static uint16_t game_ram_read_16(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);

  assert(!"non impl read!");
  return 0xFF;
}


static void game_ram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

static void game_ram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}


static uint8_t game_rom_read_8(struct VB_Core* vb, uint32_t addr) {
  return read_array8(vb->rom, addr, vb->rom_mask);
}

static uint16_t game_rom_read_16(struct VB_Core* vb, uint32_t addr) {
  assert(!(addr & 0x1) && "unaligned addr in game_rom_read_16!");
  return read_array16(vb->rom, addr, vb->rom_mask);
}

static void game_rom_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

static void game_rom_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}


#define BUS_ADDR(addr) (((addr) >> 24) & 0x7)

uint8_t vb_bus_read_8(struct VB_Core* vb, uint32_t addr) {
  switch (BUS_ADDR(addr)) {
    case BUS_ADDR(0x00000000): return vip_read_8(vb, addr);
    case BUS_ADDR(0x01000000): return vsu_read_8(vb, addr);
    case BUS_ADDR(0x02000000): return io_read(vb, addr);
    case BUS_ADDR(0x04000000): return game_exp_read_8(vb, addr);
    case BUS_ADDR(0x05000000): return wram_read_8(vb, addr);
    case BUS_ADDR(0x06000000): return game_ram_read_8(vb, addr);
    case BUS_ADDR(0x07000000): return game_rom_read_8(vb, addr);
    default:
      vb_log_err("[ERROR] reading from unmapped region: 0x%08X\n", addr);
      return 0x0; /* unammped */
  }
}

uint16_t vb_bus_read_16(struct VB_Core* vb, uint32_t addr) {
  switch (BUS_ADDR(addr)) {
    case BUS_ADDR(0x00000000): return vip_read_16(vb, addr);
    case BUS_ADDR(0x01000000): return vsu_read_16(vb, addr);
    case BUS_ADDR(0x02000000): return io_read(vb, addr);
    case BUS_ADDR(0x04000000): return game_exp_read_16(vb, addr);
    case BUS_ADDR(0x05000000): return wram_read_16(vb, addr);
    case BUS_ADDR(0x06000000): return game_ram_read_16(vb, addr);
    case BUS_ADDR(0x07000000): return game_rom_read_16(vb, addr);
    default:
      vb_log_err("[ERROR] reading from unmapped region: 0x%08X\n", addr);
      return 0x0; /* unammped */
  }
}

uint32_t vb_bus_read_32(struct VB_Core* vb, uint32_t addr) {
  const uint32_t lo = vb_bus_read_16(vb, addr + 0);
  const uint32_t hi = vb_bus_read_16(vb, addr + 2);

  return (hi << 16) | lo;
}


void vb_bus_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  switch (BUS_ADDR(addr)) {
    case BUS_ADDR(0x00000000): vip_write_8(vb, addr, value); break;
    case BUS_ADDR(0x01000000): vsu_write_8(vb, addr, value); break;
    case BUS_ADDR(0x02000000): io_write(vb, addr, value); break;
    case BUS_ADDR(0x04000000): game_exp_write_8(vb, addr, value); break;
    case BUS_ADDR(0x05000000): wram_write_8(vb, addr, value); break;
    case BUS_ADDR(0x06000000): game_ram_write_8(vb, addr, value); break;
    case BUS_ADDR(0x07000000): game_rom_write_8(vb, addr, value); break;
    default:
      vb_log_err("[ERROR] writing to unmapped region: 0x%08X\n", addr);
      break;
  }
}

void vb_bus_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  switch (BUS_ADDR(addr)) {
    case BUS_ADDR(0x00000000): vip_write_16(vb, addr, value); break;
    case BUS_ADDR(0x01000000): vsu_write_16(vb, addr, value); break;
    case BUS_ADDR(0x02000000): io_write(vb, addr, value); break;
    case BUS_ADDR(0x04000000): game_exp_write_16(vb, addr, value); break;
    case BUS_ADDR(0x05000000): wram_write_16(vb, addr, value); break;
    case BUS_ADDR(0x06000000): game_ram_write_16(vb, addr, value); break;
    case BUS_ADDR(0x07000000): game_rom_write_16(vb, addr, value); break;
    default:
      vb_log_err("[ERROR] writing to unmapped region: 0x%08X\n", addr);
      break;
  }
}

void vb_bus_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {
  vb_bus_write_16(vb, addr + 0, value >> 0);
  vb_bus_write_16(vb, addr + 2, value >> 16);
}

#undef BUS_ADDR
