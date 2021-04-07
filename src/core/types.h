#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>


enum { VB_ROM_HEADER_OFFSET_0 = 0x0FFDE0 };
enum { VB_ROM_HEADER_OFFSET_1 = 0x1FFDE0 };

struct VB_RomHeader {
    char title[13];
    uint8_t reserved[4];
    char maker_code[2];
    char game_code[4];
    uint8_t version;
};

struct VB_RomTitle {
    char title[14]; /* this is NULL terminated */
};

#define GAME_PAD_INTERRUPT  0xFFFFFE00
#define TIMER_ZERO_INTERRUPT  0xFFFFFE10
#define GAME_PAK_INTERRUPT  0xFFFFFE20
#define COMMUNICATION_INTERRUPT  0xFFFFFE30
#define VIP_INTERRUPT  0xFFFFFE40
#define FLOATINT_POINT_EXCEPTION  0xFFFFFF60
#define ZERO_DIVISION_EXCEPTION  0xFFFFFF80
#define ILLEGAL_OPCODE_EXCEPTION  0xFFFFFF90
// #define TRAP instruction (vector < 16)  0xFFFFFFA0
// #define TRAP instruction (vector ≥ 16)  0xFFFFFFB0
#define ADDRESS_TRAP  0xFFFFFFC0
#define DUPLEXED_EXCEPTION  0xFFFFFFD0
#define RESET  0xFFFFFFF0


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
    uint32_t I : 4;     /* Interrupt Level */
    uint32_t NP : 1;    /* NMI Pending */
    uint32_t EP : 1;    /* Exception Pending */
    uint32_t AE : 1;    /* Address Trap Enable */
    uint32_t ID : 1;    /* Interrupt Disable */
    uint32_t FRO : 1;   /* Floating Reserved Operand */
    uint32_t FIV : 1;   /* Floating Invalid */
    uint32_t FZD : 1;   /* Floating Zero Divide */
    uint32_t FOV : 1;   /* Floating Overflow */
    uint32_t FUD : 1;   /* Floating Underflow */
    uint32_t FPR : 1;   /* Floating Precision */
    uint32_t CY : 1;    /* Carry */
    uint32_t OV : 1;    /* Overflow */
    uint32_t S : 1;     /* Sign */
    uint32_t Z : 1;     /* Zero */
};

struct TKCW {
    uint32_t OTM : 1;   /* Operand Trap Mask */
    uint32_t FIT : 1;   /* Floating Invalid Operation Trap Enable */
    uint32_t FZT : 1;   /* Floating-Zero Divide Trap Enable */
    uint32_t FVT : 1;   /* Floating-Overflow Trap Enable */
    uint32_t FUT : 1;   /* Floating-Underflow Trap Enable */
    uint32_t FPI : 1;   /* Floating-Precision Trap Enable */
    uint32_t RDI : 1;   /* Floating Rounding Control Bit for Integer Conversion */
    uint32_t RD : 2;    /* Floating Rounding Control */
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
};

struct VB_Core {
    struct VB_Cpu cpu;

    // 64 KiB
    uint8_t wram[0x10000];

    const uint8_t* rom;
    size_t rom_size;
};


#ifdef __cplusplus
}
#endif
