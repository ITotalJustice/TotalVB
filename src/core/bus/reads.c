#include "../vb.h"
#include "../internal.h"
#include "bus.h"

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


uint8_t io_read(struct VB_Core* vb, uint32_t addr) {
  #define A(a) ((a >> 2) & 0xF)

  static const uint8_t rmasks[0xF] = {
    [A(IO_CCR)]   = 0xFF,
    [A(IO_CCSR)]  = 0xFF,
    [A(IO_CDTR)]  = 0xFF,
    [A(IO_CDRR)]  = 0xFF,
    [A(IO_SDLR)]  = 0xFF,
    [A(IO_SDHR)]  = 0xFF,
    [A(IO_TLR)]   = 0xFF,
    [A(IO_THR)]   = 0xFF,
    [A(IO_TCR)]   = 0xFF,
    [A(IO_WCR)]   = 0xFF,
    [A(IO_SCR)]   = 0xFF,
  };

  static const uint8_t omasks[0xF] = {
    [A(IO_CCR)]   = 0x69, // 0b01101001
    [A(IO_CCSR)]  = 0x60, // 0b01100000
    [A(IO_CDTR)]  = 0x00,
    [A(IO_CDRR)]  = 0x00,
    [A(IO_SDLR)]  = 0x00,
    [A(IO_SDHR)]  = 0x00,
    [A(IO_TLR)]   = 0x00,
    [A(IO_THR)]   = 0x00,
    [A(IO_TCR)]   = 0xE0, // 0b11100000
    [A(IO_WCR)]   = 0xFC, // 0b11111100
    [A(IO_SCR)]   = 0x48, // 0b01001000
  };

  const uint8_t mask = rmasks[A(addr)];
  const uint8_t or_mask = omasks[A(addr)];

  switch (A(addr)) {
    case A(IO_CCR): 
      return (vb->io.CCR & mask) | or_mask;
    
    case A(IO_CCSR):
      return (vb->io.CCSR & mask) | or_mask;
    
    case A(IO_CDTR):
      return (vb->io.CDTR & mask) | or_mask;
    
    case A(IO_CDRR):
      return (vb->io.CDRR & mask) | or_mask;
    
    case A(IO_SDLR):
      return (vb->io.SDLR & mask) | or_mask;
    
    case A(IO_SDHR):
      return (vb->io.SDHR & mask) | or_mask;
    
    case A(IO_TLR): 
      return (vb->io.TLR & mask) | or_mask;
    
    case A(IO_THR): 
      return (vb->io.THR & mask) | or_mask;
    
    case A(IO_TCR): 
      return (vb->io.TCR & mask) | or_mask;
    
    case A(IO_WCR): 
      return (vb->io.WCR & mask) | or_mask;
    
    case A(IO_SCR): 
      return (vb->io.SCR & mask) | or_mask;
  }

  #undef A

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
