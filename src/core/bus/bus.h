#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "core/types.h"


// [READS]
uint8_t VB_vip_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_vip_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_vip_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_vsu_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_vsu_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_vsu_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_misc_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_misc_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_misc_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_game_exp_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_game_exp_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_game_exp_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_wram_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_wram_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_wram_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_game_ram_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_game_ram_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_game_ram_read_32(struct VB_Core* vb, uint32_t addr);

uint8_t VB_game_rom_read_8(struct VB_Core* vb, uint32_t addr);
uint16_t VB_game_rom_read_16(struct VB_Core* vb, uint32_t addr);
uint32_t VB_game_rom_read_32(struct VB_Core* vb, uint32_t addr);


// [WRITES]
void VB_vip_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_vip_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_vip_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_vsu_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_vsu_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_vsu_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_misc_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_misc_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_misc_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_game_exp_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_game_exp_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_game_exp_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_wram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_wram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_wram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_game_ram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_game_ram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_game_ram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);

void VB_game_rom_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value);
void VB_game_rom_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value);
void VB_game_rom_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value);


#ifdef __cplusplus
}
#endif
