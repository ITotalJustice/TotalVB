#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../types.h"


// [READS]
uint8_t vip_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t vip_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t vip_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t vsu_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t vsu_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t vsu_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t misc_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t misc_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t misc_read_32(struct VB_Core* vb, uint32_t addr);

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

void misc_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void misc_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void misc_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

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
