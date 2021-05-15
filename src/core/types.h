#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


struct VB_RomHeader {
  char title[14];
  uint8_t reserved[4];
  char maker_code[2];
  char game_code[4];
  uint8_t version;
};

struct VB_RomTitle {
  char title[15]; /* this is NULL terminated */
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
  // TRAP instruction (vector < 16) = 0xFFA0,
  // TRAP instruction (vector ≥ 16) = 0xFFB0,
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

// NOTE: the system register set is actually another 32 32-bit registers
// however 8-23 are reserved.
// if instructions use the registers by indexing them as an array
// then i will just use an array, like with the program register set.

// NOTE: there are 4 data types:
// - ints
// - uints
// - bit string(?)
// - floats (32-bit)

// NOTE: bit strings have a variable length.
// - A: this is the addr of... TODO!

// NOTE: word data must be aligned, so the lower 2-bits are zero
// in case those 2-bits are non-zero, they are forced when data is accessed

struct PSW {
  uint32_t I : 4;   /* Interrupt Level */
  uint32_t NP : 1;  /* NMI Pending */
  uint32_t EP : 1;  /* Exception Pending */
  uint32_t AE : 1;  /* Address Trap Enable */
  uint32_t ID : 1;  /* Interrupt Disable */
  uint32_t FRO : 1; /* Floating Reserved Operand */
  uint32_t FIV : 1; /* Floating Invalid */
  uint32_t FZD : 1; /* Floating Zero Divide */
  uint32_t FOV : 1; /* Floating Overflow */
  uint32_t FUD : 1; /* Floating Underflow */
  uint32_t FPR : 1; /* Floating Precision */
  uint32_t CY : 1;  /* Carry */
  uint32_t OV : 1;  /* Overflow */
  uint32_t S : 1;   /* Sign */
  uint32_t Z : 1;   /* Zero */
};

struct TKCW {
  uint32_t OTM : 1; /* Operand Trap Mask */
  uint32_t FIT : 1; /* Floating Invalid Operation Trap Enable */
  uint32_t FZT : 1; /* Floating-Zero Divide Trap Enable */
  uint32_t FVT : 1; /* Floating-Overflow Trap Enable */
  uint32_t FUT : 1; /* Floating-Underflow Trap Enable */
  uint32_t FPI : 1; /* Floating-Precision Trap Enable */
  uint32_t RDI : 1; /* Floating Rounding Control Bit for Integer Conversion */
  uint32_t RD : 2;  /* Floating Rounding Control */
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
  uint32_t EIPC;      /* Exception / Interrupt PC */
  uint32_t EIPSW;     /* Exception / Interrupt PSW */
  uint32_t FEPC;      /* Fatal Error PC */
  uint32_t FEPSW;     /* Fatal Error PSW */
  struct ECR ECR;     /* Exception Cause Register */
  struct PSW PSW;     /* Program Status Word */
  uint32_t PIR;       /* Processor ID Register */
  struct TKCW TKCW;   /* Task Control Word */
  uint32_t CHCW;      /* Cache Control Word */
  uint32_t ADTRE;     /* Address Trap Register */

  uint16_t cycles;    /* Cycles elapsed after an instruction */

  size_t step_count;  /* Debugging... */
};

struct VB_Vip {
  uint8_t chars[0x800];
};

struct VB_IO {
  // [LINK]
  uint8_t CCR;  // Communication Control Register
  uint8_t CCSR; // COMCNT Control Register
  uint8_t CDTR; // Transmitted Data Register
  uint8_t CDRR; // Received Data Register
  // [PAD]
  uint8_t SDLR; // Serial Data Low Register
  uint8_t SDHR; // Serial Data High Register
  // [TIMER]
  uint8_t TLR;  // Timer Counter Low Register
  uint8_t THR;  // Timer Counter High Register
  uint8_t TCR;  // Timer Control Register
  // [PAK]
  uint8_t WCR;  // Wait Control Register
  uint8_t SCR;  // Serial Control Register
};

struct VB_Core {
  struct VB_Cpu cpu;
  struct VB_Vip vip;
  struct VB_IO io;

  // 64 KiB
  uint8_t wram[0x10000];

  const uint8_t* rom;
  size_t rom_size;
  uint32_t rom_mask;
};


#ifdef __cplusplus
}
#endif
