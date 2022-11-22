/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "vb.h"
#include "internal.h"
#include "bit.h"

// #include <stdio.h>
#include <assert.h>
#include <string.h>


static void vip_log_region(struct VB_Core* vb, uint32_t addr) {
  return;
  addr &= 0x7FFFF; // the entire region is mirrored

  if (addr <= 0x00005FFF) {
    // Left frame buffer 0
    vb_log("[VIP] 16-bit read from Left frame buffer 0: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00006000 && addr <= 0x00007FFF) {
    // Character table 0
    printf("[VIP] 16-bit read from Character table 0: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00008000 && addr <= 0x0000DFFF) {
    // Left frame buffer 1
    printf("[VIP] 16-bit read from Left frame buffer 1: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0000E000 && addr <= 0x0000FFFF) {
    // Character table 1
    printf("[VIP] 16-bit read from Character table 1: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00010000 && addr <= 0x00015FFF) {
    // Right frame buffer 0
    printf("[VIP] 16-bit read from Right frame buffer 0: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00016000 && addr <= 0x00017FFF) {
    // Character table 2
    printf("[VIP] 16-bit read from Character table 2: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00018000 && addr <= 0x0001DFFF) {
    // Right frame buffer 1
    printf("[VIP] 16-bit read from Right frame buffer 1: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0001E000 && addr <= 0x0001FFFF) {
    // Character table 3
    printf("[VIP] 16-bit read from Character table 3: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00020000 && addr <= 0x0003D7FF) {
    // Background maps and world parameters
    printf("[VIP] 16-bit read from Background maps and world parameters: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0003D800 && addr <= 0x0003DBFF) {
    // World attributes
    printf("[VIP] 16-bit read from World attributes: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0003DC00 && addr <= 0x0003DDFF) {
    // Left column table
    printf("[VIP] 16-bit read from Left column table: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0003DE00 && addr <= 0x0003DFFF) {
    // Right column table
    printf("[VIP] 16-bit read from Right column table: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0003E000 && addr <= 0x0003FFFF) {
    // Object attributes
    printf("[VIP] 16-bit read from Object attributes: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00040000 && addr <= 0x0005DFFF) {
    // Unmapped
    printf("[VIP] 16-bit read from Unmapped: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0005E000 && addr <= 0x0005FFFF) {
    // I/O Registers
    printf("[VIP] 16-bit read from I/O Registers: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00060000 && addr <= 0x00077FFF) {
    // Unmapped
    vb_log_fatal("[VIP] 16-bit read from Unmapped: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x00078000 && addr <= 0x00079FFF) {
    // Mirror of character table 0
    vb_log_fatal("[VIP] 16-bit read from Mirror of character table 0: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0007A000 && addr <= 0x0007BFFF) {
    // Mirror of character table 1
    vb_log_fatal("[VIP] 16-bit read from Mirror of character table 1: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0007C000 && addr <= 0x0007DFFF) {
    // Mirror of character table 2
    vb_log_fatal("[VIP] 16-bit read from Mirror of character table 2: addr: 0x%08X\n", addr);
  }
  else if (addr >= 0x0007E000 && addr <= 0x0007FFFF) {
    // Mirror of character table 3
    vb_log_fatal("[VIP] 16-bit read from Mirror of character table 3: addr: 0x%08X\n", addr);
  }
}

static inline uint16_t* vip_get_character_table(struct VB_Core* vb, uint8_t num) {
  assert(num <= 3 && "bruh what are you doing");

  enum { table_start_addr     = 0x6000 };
  enum { space_between_tables = 0x8000 };

  const uint32_t offset = table_start_addr + (space_between_tables * num);
  return vb->vip.vram + (offset >> 1);
}

static uint16_t vip_VER_read(struct VB_Core* vb) {
  return vb->vip.VER;
}

static uint16_t vip_io_read_16(struct VB_Core* vb, uint32_t addr) {
  assert(!(addr & 0x1) && "unaligned addr in vip_io_write_16!");

  switch (addr) {
    case 0x0005F800: vb_log_fatal("[VIP] read from INTPND Interrupt Pending\n"); break; // INTPND Interrupt Pending
    case 0x0005F802: vb_log_fatal("[VIP] read from INTENB Interrupt Enable\n"); break; // INTENB Interrupt Enable
    case 0x0005F804: vb_log_fatal("[VIP] read from INTCLR Interrupt Clear\n"); break; // INTCLR Interrupt Clear
    case 0x0005F820: printf("[VIP] read from DPSTTS Display Control Read Register\n"); return 0xFFFF; break; // DPSTTS Display Control Read Register
    case 0x0005F822: vb_log_fatal("[VIP] read from DPCTRL Display Control Write Register\n"); break; // DPCTRL Display Control Write Register
    case 0x0005F824: vb_log_fatal("[VIP] read from BRTA Brightness Control Register A\n"); break; // BRTA Brightness Control Register A
    case 0x0005F826: vb_log_fatal("[VIP] read from BRTB Brightness Control Register B\n"); break; // BRTB Brightness Control Register B
    case 0x0005F828: vb_log_fatal("[VIP] read from BRTC Brightness Control Register C\n"); break; // BRTC Brightness Control Register C
    case 0x0005F82A: vb_log_fatal("[VIP] read from REST Rest Control Register\n"); break; // REST Rest Control Register
    case 0x0005F82E: vb_log_fatal("[VIP] read from FRMCYC Game Frame Control Register\n"); break; // FRMCYC Game Frame Control Register
    case 0x0005F830: vb_log_fatal("[VIP] read from CTA Column Table Read Start Address\n"); break; // CTA Column Table Read Start Address
    case 0x0005F840: printf("[VIP] read from XPSTTS Drawing Control Read Register\n"); break; // XPSTTS Drawing Control Read Register
    case 0x0005F842: vb_log_fatal("[VIP] read from XPCTRL Drawing Control Write Register\n"); break; // XPCTRL Drawing Control Write Register
    case 0x0005F844: vb_log_fatal("[VIP] read from VER VIP Version Register\n"); return vip_VER_read(vb); break; // VER VIP Version Register
    case 0x0005F848: vb_log_fatal("[VIP] read from SPT0 OBJ Control Register 0\n"); break; // SPT0 OBJ Control Register 0
    case 0x0005F84A: vb_log_fatal("[VIP] read from SPT1 OBJ Control Register 1\n"); break; // SPT1 OBJ Control Register 1
    case 0x0005F84C: vb_log_fatal("[VIP] read from SPT2 OBJ Control Register 2\n"); break; // SPT2 OBJ Control Register 2
    case 0x0005F84E: vb_log_fatal("[VIP] read from SPT3 OBJ Control Register 3\n"); break; // SPT3 OBJ Control Register 3
    case 0x0005F860: vb_log_fatal("[VIP] read from GPLT0 BG Palette Control Register 0\n"); break; // GPLT0 BG Palette Control Register 0
    case 0x0005F862: vb_log_fatal("[VIP] read from GPLT1 BG Palette Control Register 1\n"); break; // GPLT1 BG Palette Control Register 1
    case 0x0005F864: vb_log_fatal("[VIP] read from GPLT2 BG Palette Control Register 2\n"); break; // GPLT2 BG Palette Control Register 2
    case 0x0005F866: vb_log_fatal("[VIP] read from GPLT3 BG Palette Control Register 3\n"); break; // GPLT3 BG Palette Control Register 3
    case 0x0005F868: vb_log_fatal("[VIP] read from JPLT0 OBJ Palette Control Register 0\n"); break; // JPLT0 OBJ Palette Control Register 0
    case 0x0005F86A: vb_log_fatal("[VIP] read from JPLT1 OBJ Palette Control Register 1\n"); break; // JPLT1 OBJ Palette Control Register 1
    case 0x0005F86C: vb_log_fatal("[VIP] read from JPLT2 OBJ Palette Control Register 2\n"); break; // JPLT2 OBJ Palette Control Register 2
    case 0x0005F86E: vb_log_fatal("[VIP] read from JPLT3 OBJ Palette Control Register 3\n"); break; // JPLT3 OBJ Palette Control Register 3
    case 0x0005F870: vb_log_fatal("[VIP] read from BKCOL BG Color Palette Control Register\n"); break; // BKCOL BG Color Palette Control Register
    default:
        vb_log_fatal("[VIP] invalid register read: 0x%08X\n", addr);
        return 0xCAFE;
  }

  return 0x0;
}

static void vip_io_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  assert(!(addr & 0x1) && "unaligned addr in vip_io_write_16!");

  switch (addr) {
    case 0x0005F800: vb_log_fatal("[VIP] write to INTPND Interrupt Pending: addr: 0x%08X value: 0x%04X\n", addr, value); break; // INTPND Interrupt Pending
    case 0x0005F802: printf("[VIP] write to INTENB Interrupt Enable: addr: 0x%08X value: 0x%04X\n", addr, value); break; // INTENB Interrupt Enable
    case 0x0005F804: printf("[VIP] write to INTCLR Interrupt Clear: addr: 0x%08X value: 0x%04X\n", addr, value); break; // INTCLR Interrupt Clear
    case 0x0005F820: vb_log_fatal("[VIP] write to DPSTTS Display Control Read Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // DPSTTS Display Control Read Register
    case 0x0005F822: printf("[VIP] write to DPCTRL Display Control Write Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // DPCTRL Display Control Write Register
    case 0x0005F824: printf("[VIP] write to BRTA Brightness Control Register A: addr: 0x%08X value: 0x%04X\n", addr, value); break; // BRTA Brightness Control Register A
    case 0x0005F826: printf("[VIP] write to BRTB Brightness Control Register B: addr: 0x%08X value: 0x%04X\n", addr, value); break; // BRTB Brightness Control Register B
    case 0x0005F828: printf("[VIP] write to BRTC Brightness Control Register C: addr: 0x%08X value: 0x%04X\n", addr, value); break; // BRTC Brightness Control Register C
    case 0x0005F82A: printf("[VIP] write to REST Rest Control Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // REST Rest Control Register
    case 0x0005F82E: printf("[VIP] write to FRMCYC Game Frame Control Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // FRMCYC Game Frame Control Register
    case 0x0005F830: printf("[VIP] write to CTA Column Table Read Start Address: addr: 0x%08X value: 0x%04X\n", addr, value); break; // CTA Column Table Read Start Address
    case 0x0005F840: printf("[VIP] write to XPSTTS Drawing Control Read Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // XPSTTS Drawing Control Read Register
    case 0x0005F842: printf("[VIP] write to XPCTRL Drawing Control Write Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // XPCTRL Drawing Control Write Register
    case 0x0005F844: printf("[VIP] write to VER VIP Version Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // VER VIP Version Register
    case 0x0005F848: printf("[VIP] write to SPT0 OBJ Control Register 0: addr: 0x%08X value: 0x%04X\n", addr, value); break; // SPT0 OBJ Control Register 0
    case 0x0005F84A: printf("[VIP] write to SPT1 OBJ Control Register 1: addr: 0x%08X value: 0x%04X\n", addr, value); break; // SPT1 OBJ Control Register 1
    case 0x0005F84C: printf("[VIP] write to SPT2 OBJ Control Register 2: addr: 0x%08X value: 0x%04X\n", addr, value); break; // SPT2 OBJ Control Register 2
    case 0x0005F84E: printf("[VIP] write to SPT3 OBJ Control Register 3: addr: 0x%08X value: 0x%04X\n", addr, value); break; // SPT3 OBJ Control Register 3
    case 0x0005F860: printf("[VIP] write to GPLT0 BG Palette Control Register 0: addr: 0x%08X value: 0x%04X\n", addr, value); break; // GPLT0 BG Palette Control Register 0
    case 0x0005F862: printf("[VIP] write to GPLT1 BG Palette Control Register 1: addr: 0x%08X value: 0x%04X\n", addr, value); break; // GPLT1 BG Palette Control Register 1
    case 0x0005F864: printf("[VIP] write to GPLT2 BG Palette Control Register 2: addr: 0x%08X value: 0x%04X\n", addr, value); break; // GPLT2 BG Palette Control Register 2
    case 0x0005F866: printf("[VIP] write to GPLT3 BG Palette Control Register 3: addr: 0x%08X value: 0x%04X\n", addr, value); break; // GPLT3 BG Palette Control Register 3
    case 0x0005F868: printf("[VIP] write to JPLT0 OBJ Palette Control Register 0: addr: 0x%08X value: 0x%04X\n", addr, value); break; // JPLT0 OBJ Palette Control Register 0
    case 0x0005F86A: printf("[VIP] write to JPLT1 OBJ Palette Control Register 1: addr: 0x%08X value: 0x%04X\n", addr, value); break; // JPLT1 OBJ Palette Control Register 1
    case 0x0005F86C: printf("[VIP] write to JPLT2 OBJ Palette Control Register 2: addr: 0x%08X value: 0x%04X\n", addr, value); break; // JPLT2 OBJ Palette Control Register 2
    case 0x0005F86E: printf("[VIP] write to JPLT3 OBJ Palette Control Register 3: addr: 0x%08X value: 0x%04X\n", addr, value); break; // JPLT3 OBJ Palette Control Register 3
    case 0x0005F870: printf("[VIP] write to BKCOL BG Color Palette Control Register: addr: 0x%08X value: 0x%04X\n", addr, value); break; // BKCOL BG Color Palette Control Register
    default:
        vb_log_fatal("[VIP] invalid register write: 0x%08X value: 0x%04X\n", addr, value);
        break;
  }
}

uint16_t vip_read_16(struct VB_Core* vb, uint32_t addr) {
  assert(!(addr & 0x1) && "unaligned addr in vip_read_16!");

  vip_log_region(vb, addr);

  addr &= 0x7FFFF; // the entire region is mirrored

  switch ((addr >> 17) & 0x3) {
    case 0:
      return vb->vip.vram[addr >> 1];

    case 1:
      return vb->vip.dram[(addr & 0x1FFFF) >> 1];

    case 2:
      return vip_io_read_16(vb, addr);

    case 3:
      if (addr >= 0x00078000) {
        const uint8_t num = (addr >> 13) & 0x3;
        const uint16_t* character_table = vip_get_character_table(vb, num);
        return character_table[(addr & 0x1FFF) >> 1];
      }
      else {
        return 0xCAFE; // unused
      }
  }

  __builtin_unreachable();
}

uint8_t vip_read_8(struct VB_Core* vb, uint32_t addr) {
  const uint16_t value = vip_read_16(vb, addr & ~0x1);

  // do we want to lo or hi byte?
  if (addr & 1) {
    return value >> 8; // the hi byte
  }
  else {
    return value & 0xFF; // the lo byte
  }
}

void vip_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  assert(!(addr & 0x1) && "unaligned addr in vip_write_16!");

  vip_log_region(vb, addr);

  addr &= 0x7FFFF; // the entire region is mirrored

  switch ((addr >> 17) & 0x3) {
    case 0:
      vb->vip.vram[addr >> 1] = value;
      break;

    case 1:
      vb->vip.dram[(addr & 0x1FFFF) >> 1] = value;
      break;

    case 2:
      vip_io_write_16(vb, addr, value);
      break;

    case 3:
      if (addr >= 0x00078000) {
        const uint8_t num = (addr >> 13) & 0x3;
        uint16_t* character_table = vip_get_character_table(vb, num);
        character_table[(addr & 0x1FFF) >> 1] = value;
      }
      break;
  }
}

void vip_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  if (addr >= 0x0005E000 && addr <= 0x0005FFFF) {
    vb_log_fatal("[VIP] 8-bit write to I/O Registers: addr: 0x%08X value: 0x%02X\n", addr, value);
  }
  else {
    uint16_t new_value = vip_read_16(vb, addr & ~0x1);

    if (addr & 0x1) {
      new_value &= 0x00FF;
      new_value |= value << 8;
    }
    else {
      new_value &= 0xFF00;
      new_value |= value;
    }

    vip_write_16(vb, addr & ~0x1, new_value);
  }
}

void vb_vip_run(struct VB_Core* vb, uint8_t cycles) {

}

void vb_vip_reset(struct VB_Core* vb) {
  memset(&vb->vip, 0, sizeof(vb->vip));

  vb->vip.VER = 2; // fixed version of 2, not sure if anything need this.

  const uint16_t deadbeef[4] = { 0xDE, 0xAD, 0xBE, 0xEF };

  for (size_t i = 0; i < VB_ARR_SIZE(vb->vip.vram); i++) {
    vb->vip.vram[i] = deadbeef[i & 3];
  }

  for (size_t i = 0; i < VB_ARR_SIZE(vb->vip.dram); i++) {
    vb->vip.dram[i] = deadbeef[i & 3];
  }
}
