#include "core/vb.h"
#include "core/internal.h"


static inline void write_array8(uint8_t* array, const uint32_t addr, const uint8_t value, const uint32_t mask) {
    array[addr & mask] = value;
}

static inline void write_array16(uint8_t* array, const uint32_t addr, const uint16_t value, const uint32_t mask) {
    array[(addr + 0) & mask] = (value >> 0) & 0xFF;
    array[(addr + 1) & mask] = (value >> 8) & 0xFF;
}

static inline void write_array32(uint8_t* array, const uint32_t addr, const uint32_t value, const uint32_t mask) {
    array[(addr + 0) & mask] = (value >> 0) & 0xFF;
    array[(addr + 1) & mask] = (value >> 8) & 0xFF;
    array[(addr + 2) & mask] = (value >> 16) & 0xFF;
    array[(addr + 3) & mask] = (value >> 24) & 0xFF;
}


void VB_vip_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {

}

void VB_vip_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {

}

void VB_vip_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {

}


void VB_vsu_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {

}

void VB_vsu_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {

}

void VB_vsu_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {

}


void VB_misc_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {

}

void VB_misc_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {

}

void VB_misc_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {

}


void VB_game_exp_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {

}

void VB_game_exp_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {

}

void VB_game_exp_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {

}


void VB_wram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
    write_array8(vb->wram, addr, value, 0xFFFF);
}

void VB_wram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
    write_array16(vb->wram, addr, value, 0xFFFF);
}

void VB_wram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {
    write_array32(vb->wram, addr, value, 0xFFFF);
}


void VB_game_ram_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {

}

void VB_game_ram_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {

}

void VB_game_ram_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {

}


void VB_game_rom_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
    VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

void VB_game_rom_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
    VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

void VB_game_rom_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {
    VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);
}

