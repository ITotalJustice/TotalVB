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



/*
[some notes]

- all registers are write only!
  i am not sure what the return values are of the registers, especially for
  the unused bits.

  maybe the unused bits return 1's and used bits return last written value,
  sort-of like the dmg apu (iirc).

  can't really test this as a virtual boy is £300, flashcart is £200.
  hard to jusity that when 12 games released for the thing.

- writing to waveram can only be done whilst ALL channels are disabled
  i assume it is done this way as channels have free control in which section
  of waveram they want to point to, ie, all channels could point to waveram[0]
  if so desired.

  there is also no way to query if all or a single channel is disabled.
  due to this, the programmer should always write a 1 to SSTOP which disabled
  all channels.

  this is a bit strange to me because they have to then waste more cycles
  re-enabling the channels 1 by 1, there's no "enable all" bit in SSTOP.

  this does explain why writing to SxINT resets the waveram position as well
  as other useful stuff all on a single write. so cycles are saved there.

*/

enum {
  SAMPLE_TICKS = VB_CPU_SPEED / VB_SAMPLE_RATE,
};

enum Channel {
  Channel_1 = 0,
  Channel_2 = 1,
  Channel_3 = 2,
  Channel_4 = 3,
  Channel_5 = 4,
  Channel_6 = 5,
};

// NOTE: incase i forget what this means, look at my sms psg emulation.
static const uint8_t NOISE_TAP_BIT[8] = {
  [0] = 14, // Sequence length of 32,767
  [1] = 10, // Sequence length of 1,953
  [2] = 13, // Sequence length of 254
  [3] = 4,  // Sequence length of 217
  [4] = 8,  // Sequence length of 73
  [5] = 6,  // Sequence length of 63
  [6] = 9,  // Sequence length of 42
  [7] = 11, // Sequence length of 28
};


static inline bool vsu_is_channel_enabled(struct VB_Core* vb, enum Channel channel) {
  return vb->vsu.channels[channel].SxINT.enabled;
}

// returns true if all channels are disabled.
// this is not optimised as it trashes cache
// a better impl would be to keep track of all the channels enabled
// using an int. on enabled, int++, on disable, int--, very simple.
// however this is more clear, so i'll go with this for now.
static inline bool vsu_can_access_waveram(struct VB_Core* vb) {
  for (size_t i = 0; i < VB_ARR_SIZE(vb->vsu.channels); i++) {
    if (vb->vsu.channels[i].SxINT.enabled) {
      vb_log_fatal("[VSU] trying to access waveram when channel: %zu is enabled\n", i);
      return false;
    }
  }
  return true;
}

static inline bool vsu_can_access_modram(struct VB_Core* vb) {
  return !vsu_is_channel_enabled(vb, Channel_5);
}

static inline void vsu_waveram_write(struct VB_Core* vb, uint8_t addr, uint8_t value, uint8_t index) {
  if (vsu_can_access_waveram(vb)) {
    vb->vsu.waveram[index][addr] = value;
  }
}

static inline void vsu_modram_write(struct VB_Core* vb, uint8_t addr, uint8_t value) {
  if (vsu_can_access_modram(vb)) {
    vb->vsu.modulation_ram[addr] = value;
  }
}

static void vsu_sxint_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const uint8_t interval = bit_get_range(0, 4, value);
  const bool mode = bit_is_set(5, value);
  const bool enabled = bit_is_set(7, value);

  vb->vsu.channels[channel].SxINT.interval = interval;
  vb->vsu.channels[channel].SxINT.mode = mode;
  vb->vsu.channels[channel].SxINT.enabled = enabled;

// on write, these changes happen
/*
• 	The frequency delay counter will be reset to the beginning of its current sample.
• 	The current position in PCM wave memory will be reset to the first sample.
• 	The envelope step timer will be reset to the start of its step interval.
• 	The frequency modification timer will be reset to the start of its modification interval.
• 	The current position in modulation memory will be reset to the first value.
• 	The noise generator's shift register will be reset to all 1s.
*/
  vb->vsu.channels[channel].sampling_position = 0;
  // vb->vsu.channels[channel].sample =
}

static void vsu_sxlrv_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const uint8_t right = bit_get_range(0, 4, value);
  const uint8_t left = bit_get_range(0, 4, value);

  vb->vsu.channels[channel].SxLRV.right = right;
  vb->vsu.channels[channel].SxLRV.left = left;
}

static void vsu_sxfql_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const uint8_t low = value;

  vb->vsu.channels[channel].SxFQL.low = low;
}

static void vsu_sxfqh_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const uint8_t high = bit_get_range(0, 2, value);

  vb->vsu.channels[channel].SxFQH.high = high;
}

static void vsu_sxev0_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const uint8_t interval = bit_get_range(0, 2, value);
  const bool direction = bit_is_set(3, value);
  const uint8_t reload = bit_get_range(4, 7, value);

  vb->vsu.channels[channel].SxEV0.interval = interval;
  vb->vsu.channels[channel].SxEV0.direction = direction;
  vb->vsu.channels[channel].SxEV0.reload = reload;
}

static void vsu_sxev1_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  const bool enabled = bit_is_set(0, value);
  const bool loop = bit_is_set(1, value);
  // const uint8_t ext = bit_get_range(4, 6, value);

  vb->vsu.channels[channel].SxEV1.enabled = enabled;
  vb->vsu.channels[channel].SxEV1.loop = loop;
  // vb->vsu.channels[channel].SxEV1.ext = ext;
}

static void vsu_sxram_write(struct VB_Core* vb, uint8_t value, enum Channel channel) {
  vb->vsu.SxRAM[channel].index = value;

  if (vb->vsu.SxRAM[channel].index > 4) {
    vb_log_fatal("[VSU] OOB value for S%uRAM: %u\n", channel, value);
  }
}

static void vsu_sstop_write(struct VB_Core* vb, uint8_t value) {
  const bool stop = bit_is_set(0, value);

  if (stop) {
    vb_log("[VSU] sstop written to - disabling all channels!\n");

    for (size_t i = 0; i < VB_ARR_SIZE(vb->vsu.channels); i++) {
      vb->vsu.channels[i].SxINT.enabled = false;
    }
  }
  else {
    printf("[VSU] sstop written to without stop-bit set...which is a nop\n");
  }
}

static void vsu_io_write(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  switch (addr) {
    // [channel 1]
    case 0x01000400: vsu_sxint_write(vb, value, Channel_1); printf("[VSU] S1INT write: 0x%02X\n", value); break; // S1INT Channel 1 Sound Interval Specification Register
    case 0x01000404: vsu_sxlrv_write(vb, value, Channel_1); printf("[VSU] S1LRV write: 0x%02X\n", value); break; // S1LRV Channel 1 Level Setting Register
    case 0x01000408: vsu_sxfql_write(vb, value, Channel_1); printf("[VSU] S1FQL write: 0x%02X\n", value); break; // S1FQL Channel 1 Frequency Setting Low Register
    case 0x0100040C: vsu_sxfqh_write(vb, value, Channel_1); printf("[VSU] S1FQH write: 0x%02X\n", value); break; // S1FQH Channel 1 Frequency Setting High Register
    case 0x01000410: vsu_sxev0_write(vb, value, Channel_1); printf("[VSU] S1EV0 write: 0x%02X\n", value); break; // S1EV0 Channel 1 Envelope Specification Register 0
    case 0x01000414: vsu_sxev1_write(vb, value, Channel_1); printf("[VSU] S1EV1 write: 0x%02X\n", value); break; // S1EV1 Channel 1 Envelope Specification Register 1
    case 0x01000418: vsu_sxram_write(vb, value, Channel_1); printf("[VSU] S1RAM write: 0x%02X\n", value); break; // S1RAM Channel 1 Base Address Setting Register

    // [channel 2]
    case 0x01000440: vsu_sxint_write(vb, value, Channel_2); printf("[VSU] S2INT write: 0x%02X\n", value); break; // S2INT Channel 2 Sound Interval Specification Register
    case 0x01000444: vsu_sxlrv_write(vb, value, Channel_2); printf("[VSU] S2LRV write: 0x%02X\n", value); break; // S2LRV Channel 2 Level Setting Register
    case 0x01000448: vsu_sxfql_write(vb, value, Channel_2); printf("[VSU] S2FQL write: 0x%02X\n", value); break; // S2FQL Channel 2 Frequency Setting Low Register
    case 0x0100044C: vsu_sxfqh_write(vb, value, Channel_2); printf("[VSU] S2FQH write: 0x%02X\n", value); break; // S2FQH Channel 2 Frequency Setting High Register
    case 0x01000450: vsu_sxev0_write(vb, value, Channel_2); printf("[VSU] S2EV0 write: 0x%02X\n", value); break; // S2EV0 Channel 2 Envelope Specification Register 0
    case 0x01000454: vsu_sxev1_write(vb, value, Channel_2); printf("[VSU] S2EV1 write: 0x%02X\n", value); break; // S2EV1 Channel 2 Envelope Specification Register 1
    case 0x01000458: vsu_sxram_write(vb, value, Channel_2); printf("[VSU] S2RAM write: 0x%02X\n", value); break; // S2RAM Channel 2 Base Address Setting Register

    // [channel 3]
    case 0x01000480: vsu_sxint_write(vb, value, Channel_3); printf("[VSU] S3INT write: 0x%02X\n", value); break; // S3INT Channel 3 Sound Interval Specification Register
    case 0x01000484: vsu_sxlrv_write(vb, value, Channel_3); printf("[VSU] S3LRV write: 0x%02X\n", value); break; // S3LRV Channel 3 Level Setting Register
    case 0x01000488: vsu_sxfql_write(vb, value, Channel_3); printf("[VSU] S3FQL write: 0x%02X\n", value); break; // S3FQL Channel 3 Frequency Setting Low Register
    case 0x0100048C: vsu_sxfqh_write(vb, value, Channel_3); printf("[VSU] S3FQH write: 0x%02X\n", value); break; // S3FQH Channel 3 Frequency Setting High Register
    case 0x01000490: vsu_sxev0_write(vb, value, Channel_3); printf("[VSU] S3EV0 write: 0x%02X\n", value); break; // S3EV0 Channel 3 Envelope Specification Register 0
    case 0x01000494: vsu_sxev1_write(vb, value, Channel_3); printf("[VSU] S3EV1 write: 0x%02X\n", value); break; // S3EV1 Channel 3 Envelope Specification Register 1
    case 0x01000498: vsu_sxram_write(vb, value, Channel_3); printf("[VSU] S3RAM write: 0x%02X\n", value); break; // S3RAM Channel 3 Base Address Setting Register

    // [channel 4]
    case 0x010004C0: vsu_sxint_write(vb, value, Channel_4); printf("[VSU] S4INT write: 0x%02X\n", value); break; // S4INT Channel 4 Sound Interval Specification Register
    case 0x010004C4: vsu_sxlrv_write(vb, value, Channel_4); printf("[VSU] S4LRV write: 0x%02X\n", value); break; // S4LRV Channel 4 Level Setting Register
    case 0x010004C8: vsu_sxfql_write(vb, value, Channel_4); printf("[VSU] S4FQL write: 0x%02X\n", value); break; // S4FQL Channel 4 Frequency Setting Low Register
    case 0x010004CC: vsu_sxfqh_write(vb, value, Channel_4); printf("[VSU] S4FQH write: 0x%02X\n", value); break; // S4FQH Channel 4 Frequency Setting High Register
    case 0x010004D0: vsu_sxev0_write(vb, value, Channel_4); printf("[VSU] S4EV0 write: 0x%02X\n", value); break; // S4EV0 Channel 4 Envelope Specification Register 0
    case 0x010004D4: vsu_sxev1_write(vb, value, Channel_4); printf("[VSU] S4EV1 write: 0x%02X\n", value); break; // S4EV1 Channel 4 Envelope Specification Register 1
    case 0x010004D8: vsu_sxram_write(vb, value, Channel_4); printf("[VSU] S4RAM write: 0x%02X\n", value); break; // S4RAM Channel 4 Base Address Setting Register

    // [channel 5]
    case 0x01000500: vsu_sxint_write(vb, value, Channel_5); printf("[VSU] S5INT write: 0x%02X\n", value); break; // S5INT Channel 5 Sound Interval Specification Register
    case 0x01000504: vsu_sxlrv_write(vb, value, Channel_5); printf("[VSU] S5LRV write: 0x%02X\n", value); break; // S5LRV Channel 5 Level Setting Register
    case 0x01000508: vsu_sxfql_write(vb, value, Channel_5); printf("[VSU] S5FQL write: 0x%02X\n", value); break; // S5FQL Channel 5 Frequency Setting Low Register
    case 0x0100050C: vsu_sxfqh_write(vb, value, Channel_5); printf("[VSU] S5FQH write: 0x%02X\n", value); break; // S5FQH Channel 5 Frequency Setting High Register
    case 0x01000510: vsu_sxev0_write(vb, value, Channel_5); printf("[VSU] S5EV0 write: 0x%02X\n", value); break; // S5EV0 Channel 5 Envelope Specification Register 0
    case 0x01000514: vsu_sxev1_write(vb, value, Channel_5); printf("[VSU] S5EV1 write: 0x%02X\n", value); break; // S5EV1 Channel 5 Envelope Specification Register 1
    case 0x01000518: vsu_sxram_write(vb, value, Channel_5); printf("[VSU] S5RAM write: 0x%02X\n", value); break; // S5RAM Channel 5 Base Address Setting Register
    case 0x0100051C: printf("[VSU] S5SWP write: 0x%02X\n", value); break; // S5SWP Sweep/Modulation Register

    // [channel 6]
    case 0x01000540: vsu_sxint_write(vb, value, Channel_6); printf("[VSU] S6INT write: 0x%02X\n", value); break; // S6INT Channel 6 Sound Interval Specification Register
    case 0x01000544: vsu_sxlrv_write(vb, value, Channel_6); printf("[VSU] S6LRV write: 0x%02X\n", value); break; // S6LRV Channel 6 Level Setting Register
    case 0x01000548: vsu_sxfql_write(vb, value, Channel_6); printf("[VSU] S6FQL write: 0x%02X\n", value); break; // S6FQL Channel 6 Frequency Setting Low Register
    case 0x0100054C: vsu_sxfqh_write(vb, value, Channel_6); printf("[VSU] S6FQH write: 0x%02X\n", value); break; // S6FQH Channel 6 Frequency Setting High Register
    case 0x01000550: vsu_sxev0_write(vb, value, Channel_6); printf("[VSU] S6EV0 write: 0x%02X\n", value); break; // S6EV0 Channel 6 Envelope Specification Register 0
    case 0x01000554: vsu_sxev1_write(vb, value, Channel_6); printf("[VSU] S6EV1 write: 0x%02X\n", value); break; // S6EV1 Channel 6 Envelope Specification Register 1

    // [misc]
    case 0x01000580: vsu_sstop_write(vb, value); break; // SSTOP Stop All Sound Output Register
  }
}


uint8_t vsu_read_8(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);
  vb_log_fatal("[VSU] 8-bit reads are UB\n");
  return 0xFF;
}

uint16_t vsu_read_16(struct VB_Core* vb, uint32_t addr) {
  VB_UNUSED(vb); VB_UNUSED(addr);
  vb_log_fatal("[VSU] 16-bit reads are UB\n");
  return 0xDEAD;
}

void vsu_write_8(struct VB_Core* vb, uint32_t addr, uint8_t value) {
  #define VSU_ADDR(addr) (((addr) >> 7) & 0x1F)
  #define VSU_RAM_ADDR(addr) (((addr) >> 2) & 0x1F)

  switch (VSU_ADDR(addr)) {
    case VSU_ADDR(0x0100007F): printf("[VSU] writing to waveform 1 ram: 0x%08X 0x%02X\n", addr, value); vsu_waveram_write(vb, VSU_RAM_ADDR(addr), value, 0); break;
    case VSU_ADDR(0x010000FF): printf("[VSU] writing to waveform 2 ram: 0x%08X 0x%02X\n", addr, value); vsu_waveram_write(vb, VSU_RAM_ADDR(addr), value, 1); break;
    case VSU_ADDR(0x0100017F): printf("[VSU] writing to waveform 3 ram: 0x%08X 0x%02X\n", addr, value); vsu_waveram_write(vb, VSU_RAM_ADDR(addr), value, 2); break;
    case VSU_ADDR(0x010001FF): printf("[VSU] writing to waveform 4 ram: 0x%08X 0x%02X\n", addr, value); vsu_waveram_write(vb, VSU_RAM_ADDR(addr), value, 3); break;
    case VSU_ADDR(0x0100027F): printf("[VSU] writing to waveform 5 ram: 0x%08X 0x%02X\n", addr, value); vsu_waveram_write(vb, VSU_RAM_ADDR(addr), value, 4); break;
    case VSU_ADDR(0x010002FF): printf("[VSU] writing to modulation ram: 0x%08X 0x%02X\n", addr, value); vsu_modram_write(vb, VSU_RAM_ADDR(addr), value); break;

    default: // I/O Registers
      vsu_io_write(vb, addr, value);
      break;
  }

  #undef VSU_ADDR
  #undef VSU_RAM_ADDR
}

void vsu_write_16(struct VB_Core* vb, uint32_t addr, uint16_t value) {
  VB_UNUSED(vb); VB_UNUSED(addr); VB_UNUSED(value);

  assert(!(addr & 0x1) && "unaligned addr in vsu_write_16!");
  vb_log_fatal("[VSU] 16-bit writes are UB\n");
}

// void vsu_sample(struct VB_Core* vb) {

// }

void vb_vsu_run(struct VB_Core* vb, uint8_t cycles) {

}

void vb_vsu_reset(struct VB_Core* vb) {
  memset(&vb->vsu, 0, sizeof(vb->vsu));
}
