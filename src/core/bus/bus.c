#include "../vb.h"
#include "../internal.h"
#include "bus.h"

#include <assert.h>


/*
0x00000000 - 0x00FFFFFF   VIP - Virtual Image Processor
0x01000000 - 0x01FFFFFF   VSU - Virtual Sound Unit
0x02000000 - 0x02FFFFFF   Miscellaneous Hardware
0x03000000 - 0x03FFFFFF   Unmapped
0x04000000 - 0x04FFFFFF   Game Pak Expansion
0x05000000 - 0x05FFFFFF   WRAM
0x06000000 - 0x06FFFFFF   Game Pak RAM
0x07000000 - 0x07FFFFFF   Game Pak ROM
0x08000000 - 0xFFFFFFFF   Mirroring of memory map
*/


uint8_t vb_bus_read_8(struct VB_Core* vb, uint32_t addr) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: return vip_read_8(vb, addr);
    case 0x1: return vsu_read_8(vb, addr);
    case 0x2: return misc_read_8(vb, addr);
    case 0x4: return game_exp_read_8(vb, addr);
    case 0x5: return wram_read_8(vb, addr);
    case 0x6: return game_ram_read_8(vb, addr);
    case 0x7: return game_rom_read_8(vb, addr);
    default: return 0x0; /* unammped */
  }
}

uint16_t vb_bus_read_16(struct VB_Core* vb, uint32_t addr) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: return vip_read_16(vb, addr);
    case 0x1: return vsu_read_16(vb, addr);
    case 0x2: return misc_read_16(vb, addr);
    case 0x4: return game_exp_read_16(vb, addr);
    case 0x5: return wram_read_16(vb, addr);
    case 0x6: return game_ram_read_16(vb, addr);
    case 0x7: return game_rom_read_16(vb, addr);
    default: return 0x0; /* unammped */
  }
}

uint32_t vb_bus_read_32(struct VB_Core* vb, uint32_t addr) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: return vip_read_32(vb, addr);
    case 0x1: return vsu_read_32(vb, addr);
    case 0x2: return misc_read_32(vb, addr);
    case 0x4: return game_exp_read_32(vb, addr);
    case 0x5: return wram_read_32(vb, addr);
    case 0x6: return game_ram_read_32(vb, addr);
    case 0x7: return game_rom_read_32(vb, addr);
    default: return 0x0; /* unammped */
  }
}


void vb_bus_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: vip_write_8(vb, addr, value); break;
    case 0x1: vsu_write_8(vb, addr, value); break;
    case 0x2: misc_write_8(vb, addr, value); break;
    case 0x4: game_exp_write_8(vb, addr, value); break;
    case 0x5: wram_write_8(vb, addr, value); break;
    case 0x6: game_ram_write_8(vb, addr, value); break;
    case 0x7: game_rom_write_8(vb, addr, value); break;
  }
}

void vb_bus_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: vip_write_16(vb, addr, value); break;
    case 0x1: vsu_write_16(vb, addr, value); break;
    case 0x2: misc_write_16(vb, addr, value); break;
    case 0x4: game_exp_write_16(vb, addr, value); break;
    case 0x5: wram_write_16(vb, addr, value); break;
    case 0x6: game_ram_write_16(vb, addr, value); break;
    case 0x7: game_rom_write_16(vb, addr, value); break;
  }
}

void vb_bus_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {
  switch ((addr >> 24) & 0x7) {
    case 0x0: vip_write_32(vb, addr, value); break;
    case 0x1: vsu_write_32(vb, addr, value); break;
    case 0x2: misc_write_32(vb, addr, value); break;
    case 0x4: game_exp_write_32(vb, addr, value); break;
    case 0x5: wram_write_32(vb, addr, value); break;
    case 0x6: game_ram_write_32(vb, addr, value); break;
    case 0x7: game_rom_write_32(vb, addr, value); break;
  }
}
