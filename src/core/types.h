/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


enum {
  VB_SCREEN_WIDTH = 384,
  VB_SCREEN_HEIGHT = 224,

  VB_MAX_COMMERCIAL_ROM_SIZE = 1024 * 1024 * 2, // 2 MiB
  VB_MAX_ROM_SIZE = 1024 * 1024 * 16, // 16 MiB

  VB_FPS = 50,
  VB_CPU_SPEED = 20000000, // 20Mhz
  VB_CYCLES_PER_FRAME = VB_CPU_SPEED / VB_FPS,

  VB_SAMPLE_RATE = 41700, // 41.7 KHz (is this really right?)
};

enum VB_ColourShade {
  VB_ColourShade_0, // black
  VB_ColourShade_1, // dim red
  VB_ColourShade_2, // bright red
  VB_ColourShade_3, // brightest red
};

enum VB_ExceptionHandle {
  GAME_PAD_INTERRUPT        = 0xFE00,
  TIMER_ZERO_INTERRUPT      = 0xFE10,
  GAME_PAK_INTERRUPT        = 0xFE20,
  COMMUNICATION_INTERRUPT   = 0xFE30,
  VIP_INTERRUPT             = 0xFE40,
  FLOATINT_POINT_EXCEPTION  = 0xFF60,
  ZERO_DIVISION_EXCEPTION   = 0xFF80,
  ILLEGAL_OPCODE_EXCEPTION  = 0xFF90,
  TRAP_INSTRUCTION_SMALL    = 0xFFA0, // (vector < 16)
  TRAP_INSTRUCTION_BIG      = 0xFFB0, // (vector ≥ 16)
  ADDRESS_TRAP              = 0xFFC0,
  DUPLEXED_EXCEPTION        = 0xFFD0,
  RESET                     = 0xFFF0,
};

enum VB_RegisterType {
  ZERO_REGISTER = 0,
  RESERVED_FOR_ADDRESS_GENERATION = 1,
  HANDLER_STACK_POINTER = 2,
  STACK_POINTER = 3,
  GLOBAL_POINTER = 4,
  TEXT_POINTER = 5,

  STRING_DESTINATION_BIT_OFFSET = 26,
  STRING_SOURCE_BIT_OFFSET = 27,
  STRING_LENGTH = 28,
  STRING_DESTINATION = 29,
  STRING_SOURCE = 30,
  LINK_POINTER = 31,
};

enum VB_SysRegisterType {
  EIPC  = 0,  // Exception/Interrupt PC 	Stores the value to restore to PC when an exception finishes processing.
  EIPSW = 1,  // Exception/Interrupt PSW 	Stores the value to restore to PSW when an exception finishes processing.
  FEPC  = 2,  // Fatal Error PC 	Stores the value to restore to PC when a duplexed exception finishes processing.
  FEPSW = 3,  // Fatal Error PSW 	Stores the value to restore to PSW when a duplexed exception finishes processing.
  PIR   = 6,  // Processor ID Register 	Indicates to the program what kind of processor it's being run on.
  PSW   = 5,  // Program Status Word 	Contains status flags and the interrupt masking level.
  TKCW  = 7,  // Task Control Word 	Specifies the behavior of floating-point instructions.
  ECR   = 4,  // Exception Cause Register 	Stores values indicating the source of exceptions or interrupts.
  ADTRE = 25, // Address Trap Register for Execution 	Configures the execution address for the hardware breakpoint.
  CHCW  = 24, // Cache Control Word 	Configures the instruction cache.
  UNK29 = 29, // [note] This system register has unknown significance.
  UNK30 = 30, // [note] This system register has unknown significance.
  ABS   = 31, // Calculates the absolute value of the number written into it.
};


struct VB_RomHeader {
  char title[20];
  uint8_t reserved[4];
  char maker_code[2];
  char game_code[4];
  uint8_t version;
};

struct VB_RomTitle {
  char title[21]; /* this is NULL terminated */
};

// TODO: the psw is ordered based on access frequency, ie, flags at top
// need to do this for the rest of the structs.
struct PSW {
  bool Z;     /* Zero */
  bool S;     /* Sign */
  bool OV;    /* Overflow */
  bool CY;    /* Carry */
  bool FPR;   /* Floating Precision */
  bool FUD;   /* Floating Underflow */
  bool FOV;   /* Floating Overflow */
  bool FZD;   /* Floating Zero Divide */
  bool FIV;   /* Floating Invalid */
  bool FRO;   /* Floating Reserved Operand */
  bool ID;    /* Interrupt Disable */
  bool AE;    /* Address Trap Enable */
  bool EP;    /* Exception Pending */
  bool NP;    /* NMI Pending */
  uint8_t I;  /* Interrupt Level */
};

struct TKCW {
  bool OTM;   /* Operand Trap Mask */
  bool FIT;   /* Floating Invalid Operation Trap Enable */
  bool FZT;   /* Floating-Zero Divide Trap Enable */
  bool FVT;   /* Floating-Overflow Trap Enable */
  bool FUT;   /* Floating-Underflow Trap Enable */
  bool FPI;   /* Floating-Precision Trap Enable */
  bool RDI;   /* Floating Rounding Control Bit for Integer Conversion */
  uint8_t RD; /* Floating Rounding Control */
};

struct ECR {
  uint16_t FECC;  /* Fatal Error Cause Code */
  uint16_t EICC;  /* Exception/Interrupt Cause Code */
};

struct VB_Cpu {
  /* Program register sets */
  uint32_t registers[32];

  uint32_t PC; /* Program Counter (bit-0 always fixed to zero) */

  /* System register sets */
  uint32_t EIPC;    /* Exception / Interrupt PC */
  uint32_t EIPSW;   /* Exception / Interrupt PSW */
  uint32_t FEPC;    /* Fatal Error PC */
  uint32_t FEPSW;   /* Fatal Error PSW */
  struct ECR ECR;   /* Exception Cause Register */
  struct PSW PSW;   /* Program Status Word */
  uint32_t PIR;     /* Processor ID Register */
  struct TKCW TKCW; /* Task Control Word */
  uint32_t CHCW;    /* Cache Control Word */
  uint32_t ADTRE;   /* Address Trap Register */
  uint32_t ABS;     /* Keeps the abs of the value written to it */

  uint16_t cycles;  /* Cycles elapsed after an instruction */

  bool halted; /* set to true when the cpu is halted */

  size_t step_count;  /* Debugging... */
};

// [Common VIP Terms]
// characters = tiles
// objects = sprites
// worlds = windows
struct VB_Vip {
  // x2 left frame buffers, x2 right frame buffers, tiles
  uint16_t vram[1024 * 128 / 2];
  // backgrounds, coloumn table windows, oam
  uint16_t dram[1024 * 128 / 2];

  uint16_t INTPND;  // Interrupt Pending
  uint16_t INTENB;  // Interrupt Enable
  uint16_t INTCLR;  // Interrupt Clear
  uint16_t DPSTTS;  // Display Control Read Register
  uint16_t DPCTRL;  // Display Control Write Register
  uint16_t BRTA;    // Brightness Control Register A
  uint16_t BRTB;    // Brightness Control Register B
  uint16_t BRTC;    // Brightness Control Register C
  uint16_t REST;    // Rest Control Register
  uint16_t FRMCYC;  // Game Frame Control Register
  uint16_t CTA;     // Column Table Read Start Address
  uint16_t XPSTTS;  // Drawing Control Read Register
  uint16_t XPCTRL;  // Drawing Control Write Register
  uint16_t VER;     // VIP Version Register
  uint16_t SPT0;    // OBJ Control Register 0
  uint16_t SPT1;    // OBJ Control Register 1
  uint16_t SPT2;    // OBJ Control Register 2
  uint16_t SPT3;    // OBJ Control Register 3
  uint16_t GPLT0;   // BG Palette Control Register 0
  uint16_t GPLT1;   // BG Palette Control Register 1
  uint16_t GPLT2;   // BG Palette Control Register 2
  uint16_t GPLT3;   // BG Palette Control Register 3
  uint16_t JPLT0;   // OBJ Palette Control Register 0
  uint16_t JPLT1;   // OBJ Palette Control Register 1
  uint16_t JPLT2;   // OBJ Palette Control Register 2
  uint16_t JPLT3;   // OBJ Palette Control Register 3
  uint16_t BKCOL;   // BG Color Palette Control Register

  // characters are also known as tiles
  // uint16_t characters[2048];

  // objects are also known as sprites
  // uint16_t objects[1024];

  // not sure if this value is right yet...
  // uint16_t background_maps[14 * 4 * 4]; // 224

  // worlds are also known as windows
  // there's 32 worlds
};

// has 6 channels, 5 pcm samples, 1 noise
// channel all: len, stereo, envelope, freq control (timer)
// channel 1-4: pcm
// channel 5: pcm, sweep
// channel 6: noise
// output: 10-bit stereo at 41700 Hz
struct VB_Vsu {
  // samples are 6-bit unsigned.
  // pcm memory can only be written to when channel is disabled (inactive)
  // any writes whilst channel is enabled are ignored.
  // there is space for 32 samples, but are
  uint8_t waveram[5][32];

  // all writes whilst channel 5 is enabled are ignored
  int8_t modulation_ram[32];

  // these are values in common with all channels
  struct {
    // when SxINT is written to, the channels sampling position
    // in waveram is reset!

    // Sound Interval Specification Register
    struct SxINT {
      uint8_t interval;
      bool mode; // 1=disable channel after [inerval], 0=loop
      bool enabled; // is the channel enabled
    } SxINT;

    // Level Setting Register
    struct SxLRV {
      uint8_t right;
      uint8_t left;
    } SxLRV;

    // Frequency Setting Low Register
    struct SxFQL {
      uint8_t low; // lower 8-bits
    } SxFQL;

    // Frequency Setting High Register
    struct SxFQH {
      uint8_t high; // upper 3-bits
    } SxFQH;

    // Envelope Specification Register 0
    struct SxEV0 {
      uint8_t interval; // time until envelope is next ticked
      bool direction; // 1=increase / 0=decrease
      uint8_t reload; // inital and reload value of envelope
    } SxEV0;

    // Envelope Specification Register 1
    struct SxEV1 {
      bool enabled; // is the envelope enabled
      bool loop; // does the envelope repeat
    } SxEV1;

    uint8_t sampling_position; // the position in waveram
    uint8_t sample; // current sample
  } channels[6];

  // Base Address Setting Register
  struct SxRAM {
    // this indexs into waveform_ram for that channel
    // all 8-bits are used!
    // this means that a value > 4 effectively stops output from
    // that channel as there are no samples to be fetched.
    // this does not disable the channel however!
    uint8_t index;
  } SxRAM[5];

  struct S5SWP {
    uint8_t S5SWP; // Sweep/Modulation Register
  } S5SWP;

  // struct {
  // } channel_6_noise;
};

struct VB_Timer {
  uint8_t TLR;  // Timer Counter Low Register
  uint8_t THR;  // Timer Counter High Register
  uint8_t TCR;  // Timer Control Register

  // struct TCR {
  //   bool enabled;
  //   bool status;
  //   bool interrupt;
  //   bool interval;
  // } TCR;

  uint16_t counter;
};

struct VB_Pad {
  uint8_t SDLR; // Serial Data Low Register
  uint8_t SDHR; // Serial Data High Register
};

struct VB_Link {
  uint8_t CCR;  // Communication Control Register
  uint8_t CCSR; // COMCNT Control Register
  uint8_t CDTR; // Transmitted Data Register
  uint8_t CDRR; // Received Data Register
};

struct VB_Pak {
  uint8_t WCR;  // Wait Control Register
  uint8_t SCR;  // Serial Control Register
};

struct VB_Core {
  struct VB_Cpu v810;
  struct VB_Vip vip;
  struct VB_Vsu vsu;
  struct VB_Timer timer;
  struct VB_Pad pad;
  struct VB_Link link;
  struct VB_Pak pak;

  // i think wram should be uint16_t for faster access
  // same for rom, but that might cause issues if not halfword aligned.
  uint8_t wram[1024 * 64]; // 64 KiB

  const uint8_t* rom;
  size_t rom_size;
  uint32_t rom_mask; // unused (remove?)

  uint16_t* pixels; // todo: support custom width
  uint32_t stride;
  // uint8_t bpp;
};

struct VB_State {
  struct {
    uint32_t magic;
    uint32_t version;
    uint32_t size; // size of state
    uint32_t reserved;
  } meta;

  struct VB_Cpu v810;
  struct VB_Vip vip;
  struct VB_Vsu vsu;
  struct VB_Timer timer;
  struct VB_Pad pad;
  struct VB_Link link;
  struct VB_Pak pak;

  uint8_t wram[1024 * 64]; // 64 KiB
};

enum VB_StateMeta {
  VB_StateMeta_MAGIC = 0x52454431, // RED1
  VB_StateMeta_VERSION = 1,
  VB_StateMeta_SIZE = sizeof(struct VB_State),
};

#ifdef __cplusplus
}
#endif
