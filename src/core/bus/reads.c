#include "../vb.h"
#include "../internal.h"

#include <assert.h>


static inline uint8_t read_array8(
    const uint8_t* array, const uint32_t addr, const uint32_t mask
) {
    return array[addr & mask];
}

static inline uint16_t read_array16(
    const uint8_t* array, const uint32_t addr, const uint32_t mask
) {
    const uint8_t hi = array[(addr + 0) & mask];
    const uint8_t lo = array[(addr + 1) & mask];

    return (lo << 8) | hi;
}

static inline uint32_t read_array32(
    const uint8_t* array, const uint32_t addr, const uint32_t mask
) {
    const uint8_t hi_word_hi = array[(addr + 0) & mask];
    const uint8_t hi_word_lo = array[(addr + 1) & mask];
    const uint8_t lo_word_hi = array[(addr + 2) & mask];
    const uint8_t lo_word_lo = array[(addr + 3) & mask];

    return (lo_word_lo << 24) | (lo_word_hi << 16) | (hi_word_lo << 8) | hi_word_hi;
}


uint8_t vip_read_8(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint16_t vip_read_16(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint32_t vip_read_32(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}


uint8_t vsu_read_8(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint16_t vsu_read_16(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint32_t vsu_read_32(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}


uint8_t misc_read_8(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint16_t misc_read_16(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint32_t misc_read_32(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}


uint8_t game_exp_read_8(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint16_t game_exp_read_16(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint32_t game_exp_read_32(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}


uint8_t wram_read_8(struct VB_Core* vb, uint32_t addr) {
    return read_array8(vb->wram, addr, 0xFFFF);
}

uint16_t wram_read_16(struct VB_Core* vb, uint32_t addr) {
    return read_array16(vb->wram, addr, 0xFFFF);
}

uint32_t wram_read_32(struct VB_Core* vb, uint32_t addr) {
    return read_array32(vb->wram, addr, 0xFFFF);
}


uint8_t game_ram_read_8(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint16_t game_ram_read_16(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}

uint32_t game_ram_read_32(struct VB_Core* vb, uint32_t addr) {
    VB_UNUSED(vb); VB_UNUSED(addr);

    assert(0 && "non impl read!");
    return 0xFF;
}


uint8_t game_rom_read_8(struct VB_Core* vb, uint32_t addr) {
    return read_array8(vb->rom, addr, vb->rom_mask);
}

uint16_t game_rom_read_16(struct VB_Core* vb, uint32_t addr) {
    return read_array16(vb->rom, addr, vb->rom_mask);
}

uint32_t game_rom_read_32(struct VB_Core* vb, uint32_t addr) {
    return read_array32(vb->rom, addr, vb->rom_mask);
}
