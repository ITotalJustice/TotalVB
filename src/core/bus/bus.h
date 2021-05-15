#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"


enum {
  VIP_BEG           = 0x0000, // VIP - Virtual Image Processor
  VIP_END           = 0x0FFF, // VIP - Virtual Image Processor
  VSU_BEG           = 0x1000, // VSU - Virtual Sound Unit
  VSU_END           = 0x1FFF, // VSU - Virtual Sound Unit
  MISC_BEG          = 0x2000, // Miscellaneous Hardware
  MISC_END          = 0x2FFF, // Miscellaneous Hardware
  UNMAP_BEG         = 0x3000, // Unmapped
  UNMAP_END         = 0x3FFF, // Unmapped
  PAK_EXPANSION_BEG = 0x4000, // Game Pak Expansion
  PAK_EXPANSION_END = 0x4FFF, // Game Pak Expansion
  WRAM_BEG          = 0x5000, // WRAM
  WRAM_END          = 0x5FFF, // WRAM
  PAK_RAM_BEG       = 0x6000, // Game Pak RAM
  PAK_RAM_END       = 0x6FFF, // Game Pak RAM
  PAK_ROM_BEG       = 0x7000, // Game Pak ROM
  PAK_ROM_END       = 0x7FFF, // Game Pak ROM
  // 0x8000 - 0xFFFF Mirroring of memory map
};

// All IO ports are to be accessed as 8-bit values, but
// are spaced 4-bytes apart each, allowing for any access.
enum {
  IO_CCR   = 0x2000, // Link  Communication Control Register
  IO_CCSR  = 0x2004, // Link  COMCNT Control Register
  IO_CDTR  = 0x2008, // Link  Transmitted Data Register
  IO_CDRR  = 0x200C, // Link  Received Data Register
  IO_SDLR  = 0x2010, // Game Pad  Serial Data Low Register
  IO_SDHR  = 0x2014, // Game Pad  Serial Data High Register
  IO_TLR   = 0x2018, // Timer Timer Counter Low Register
  IO_THR   = 0x201C, // Timer Timer Counter High Register
  IO_TCR   = 0x2020, // Timer Timer Control Register
  IO_WCR   = 0x2024, // Game Pak  Wait Control Register
  IO_SCR   = 0x2028, // Game Pad  Serial Control Register
  // 0x2040 - 0x2FFF Mirroring of hardware component memory map
};


// [READS]
uint8_t vip_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t vip_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t vip_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t vsu_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t vsu_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t vsu_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t io_read(struct VB_Core* vb, uint32_t addr);

uint8_t game_exp_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t game_exp_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t game_exp_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t wram_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t wram_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t wram_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t game_ram_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t game_ram_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t game_ram_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t game_rom_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t game_rom_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t game_rom_read_32(struct VB_Core* vb, uint32_t addr);


// [WRITES]
void vip_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void vip_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void vip_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void vsu_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void vsu_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void vsu_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void io_write(struct VB_Core* vb, uint32_t addr, uint8_t value);

void game_exp_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void game_exp_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void game_exp_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void wram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void wram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void wram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void game_ram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void game_ram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void game_ram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void game_rom_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void game_rom_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void game_rom_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);


#ifdef __cplusplus
}
#endif
