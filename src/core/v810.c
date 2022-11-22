/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "vb.h"
#include "internal.h"
#include "bit.h"

// #include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

// https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html
#ifdef __GNUC__
  #define USE_BUILTIN 1
#else
  #define USE_BUILTIN 0
#endif


enum BranchCondition {
  BV = 0,   // Overflow	if OV = 1
  BC = 1,   // Carry, Lower	if CY = 1	Unsigned
  BE = 2,   // Equal, Zero	if Z = 1
  BNH = 3,  // Not higher	if (CY OR Z) = 1	Unsigned
  BN = 4,   // Negative	if S = 1
  BR = 5,   // Always	if true
  BLT = 6,  // Less than	if (OV XOR S) = 1	Signed
  BLE = 7,  // Less than or equal	if ((OV XOR S) OR Z) = 1	Signed
  BNV = 8,  // Not overflow	if OV = 0
  BNC = 9,  // Not carry, Not lower	if CY = 0	Unsigned
  BNE = 10, // Not equal, Not zero	if Z = 0
  BH = 11,  // Higher	if (CY OR Z) = 0	Unsigned
  BP = 12,  // Positive	if S = 0
  NOP = 13, // Not always	if false	Never branches
  BGE = 14, // Greater than or equal	if (OV XOR S) = 0	Signed
  BGT = 15, // Greater than	if ((OV XOR S) OR Z) = 0	Signed
};

#define CPU vb->v810
#define REGISTERS CPU.registers
#define REG_PC CPU.PC


// FLAGS
#define FLAG_Z CPU.PSW.Z
#define FLAG_S CPU.PSW.S
#define FLAG_OV CPU.PSW.OV
#define FLAG_CY CPU.PSW.CY


/* used for aligning the PC after modifying it. */
/* this is generic as it may need to be used for other values. */
static uint32_t align_16(const uint32_t v) { return v & ~(0x1); }
static uint32_t align_32(const uint32_t v) { return v & ~(0x3); }


// helpers that half / word align the address when reading
#define READ8(addr) vb_bus_read_8(vb, (addr))
#define READ16(addr) vb_bus_read_16(vb, align_16(addr))
#define READ32(addr) vb_bus_read_32(vb, align_32(addr))

#define WRITE8(addr, value) vb_bus_write_8(vb, (addr), value)
#define WRITE16(addr, value) vb_bus_write_16(vb, align_16(addr), value)
#define WRITE32(addr, value) vb_bus_write_32(vb, align_32(addr), value)


/* helper for v-flag calcs, ONLY used for add / sub (mults are different!) */
static inline bool calc_v_flag_add_sub(
  const uint32_t a, const uint32_t b, const uint32_t r
) {
  return (bit_is_set(31, a) == bit_is_set(31, b)) && (bit_is_set(31, a) != bit_is_set(31, r));
}


/* There are 6 unique formats for instruction encoding */
/* 1-6 have been implemeted, format7 is basically identical to format1. */
struct Format1 {
  uint8_t reg1;   // 5-bits
  uint8_t reg2;   // 5-bits
};

struct Format2 {
  uint8_t lo5;    // 5-bits
  uint8_t reg2;   // 5-bits
};

struct Format3 {
  uint8_t cond;   // 4-bits
  int32_t disp;   // TODO: this gets sign extended (16 or 32?)
};

struct Format4 {
  int32_t disp;   // sign extended
};

struct Format5 {
  uint8_t reg1;   // 5-bits
  uint8_t reg2;   // 5-bits
  uint16_t imm;   // fetched
};

struct Format6 {
  uint8_t reg1;   // 5-bits
  uint8_t reg2;   // 5-bits
  int32_t disp;   // TODO: this gets sign extended (16 or 32?)
};

struct Format7 {
  uint8_t reg1;   // 5-bits
  uint8_t reg2;   // 5-bits
  uint8_t subop;  // 6-bits
};

// [DEBUG]
#if 0
static void log_format1(const struct VB_Core* vb, const struct Format1 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X\n\n",
    f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1]);
}

static void log_format2(const struct VB_Core* vb, const struct Format2 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\tlo5: %u\n\n",
    f.opcode, f.reg2, REGISTERS[f.reg2], f.lo5);
}

static void log_format3(const struct VB_Core* vb, const struct Format3 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\tcond: 0x%X\tdisp: %d\n\n",
    f.opcode, f.cond, f.disp);
}

static void log_format4(const struct VB_Core* vb, const struct Format4 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\tdisp: %d\n\n",
    f.opcode, f.disp);
}

static void log_format5(const struct VB_Core* vb, const struct Format5 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X; imm: 0x%X\n\n",
    f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1], f.imm);
}

static void log_format6(const struct VB_Core* vb, const struct Format6 f) {
  VB_UNUSED(vb);

  vb_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X\tdisp: %d\n\n",
    f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1], f.disp);
}
#else
#define log_format1(vb, f)
#define log_format2(vb, f)
#define log_format3(vb, f)
#define log_format4(vb, f)
#define log_format5(vb, f)
#define log_format6(vb, f)
#define log_format7(vb, f)
#endif


// [GEN]
static inline struct Format1 gen_format1(struct VB_Core* vb, uint16_t op) {
  const struct Format1 f = {
    .reg1 = bit_get_range(0, 4, op),
    .reg2 = bit_get_range(5, 9, op),
  };

  log_format1(vb, f);
  return f;
}

static inline struct Format2 gen_format2(struct VB_Core* vb, uint16_t op) {
  const struct Format2 f = {
    .lo5 = bit_get_range(0, 4, op),
    .reg2 = bit_get_range(5, 9, op),
  };

  log_format2(vb, f);
  return f;
}

static inline struct Format3 gen_format3(struct VB_Core* vb, uint16_t op) {
  const uint32_t disp_range = bit_get_range(0, 8, op);

  const struct Format3 f = {
    .cond = bit_get_range(9, 12, op),
    .disp = bit_sign_extend(8, disp_range),
  };

  log_format3(vb, f);
  return f;
}

static inline struct Format4 gen_format4(struct VB_Core* vb, uint16_t op) {
  const uint16_t lo_disp = READ16(REG_PC);
  const uint32_t hi_disp = bit_get_range(0, 9, op);
  const uint32_t disp = (hi_disp << 16) | lo_disp;
  REG_PC += 2;

  const struct Format4 f = {
    .disp = bit_sign_extend(25, disp),
  };

  log_format4(vb, f);
  return f;
}

static inline struct Format5 gen_format5(struct VB_Core* vb, uint16_t op) {
  const uint16_t imm = READ16(REG_PC);
  REG_PC += 2;

  const struct Format5 f = {
    .reg1 = bit_get_range(0, 4, op),
    .reg2 = bit_get_range(5, 9, op),
    .imm = imm,
  };

  log_format5(vb, f);
  return f;
}

static inline struct Format6 gen_format6(struct VB_Core* vb, uint16_t op) {
  const uint16_t disp = READ16(REG_PC);
  REG_PC += 2;

  const struct Format6 f = {
    .reg1 = bit_get_range(0, 4, op),
    .reg2 = bit_get_range(5, 9, op),
    .disp = bit_sign_extend(15, disp),
  };

  return f;
}

static inline struct Format7 gen_format7(struct VB_Core* vb, uint16_t op) {
  // note: i am unsure if this is correct.
  // format7 is listed as fetching a 32bit value, but most
  // of that value is RFU.
  // this fetch would mean the pc advances by 4...
  const uint16_t next_op = READ16(REG_PC);
  REG_PC += 2;

  const struct Format7 f = {
    .reg1 = bit_get_range(0, 4, op),
    .reg2 = bit_get_range(5, 9, op),
    // .subop = bit_get_range(5, 9, next_op),
    .subop = (next_op >> 10) & 0x3F,
  };

  log_format7(vb, f);
  return f;
}

// [Register Transfer]
static inline void MOV_imm(struct VB_Core* vb, struct Format2 f) {
  REGISTERS[f.reg2] = bit_sign_extend(4, f.lo5);
}

static inline void MOV_reg(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = REGISTERS[f.reg1];
}

static inline void MOVEA(struct VB_Core* vb, struct Format5 f) {
  // const int32_t imm = bit_sign_extend(15, f.imm);
  const int32_t imm = (int32_t)(int16_t)f.imm;
  REGISTERS[f.reg2] = REGISTERS[f.reg1] + imm;
}

static inline void MOVHI(struct VB_Core* vb, struct Format5 f) {
  const uint32_t imm = (uint32_t)f.imm << 16;
  REGISTERS[f.reg2] = REGISTERS[f.reg1] + imm;
}


// [Load and Input]
static inline void IN_B(struct VB_Core* vb, struct Format6 f) {
  // read byte from port
  REGISTERS[f.reg2] = READ8(REGISTERS[f.reg1] + f.disp);
}

static inline void IN_H(struct VB_Core* vb, struct Format6 f) {
  // read halfword from port
  REGISTERS[f.reg2] = READ16(REGISTERS[f.reg1] + f.disp);
}

static inline void IN_W(struct VB_Core* vb, struct Format6 f) {
  // read word from port
  REGISTERS[f.reg2] = READ32(REGISTERS[f.reg1] + f.disp);
}

static inline void LD_B(struct VB_Core* vb, struct Format6 f) {
  // read byte from port and sign_extend
  // const uint8_t value = READ8(REGISTERS[f.reg1] + f.disp);
  // const int32_t extended = bit_sign_extend(8-1, value);
  // REGISTERS[f.reg2] = extended;

  REGISTERS[f.reg2] = (int32_t)(int8_t)READ8(REGISTERS[f.reg1] + f.disp);
}

static inline void LD_H(struct VB_Core* vb, struct Format6 f) {
  // read halfword from port and sign_extend
  // const uint16_t value = READ16(REGISTERS[f.reg1] + f.disp);
  // const int32_t extended = bit_sign_extend(16-1, value);
  // REGISTERS[f.reg2] = extended;

  REGISTERS[f.reg2] = (int32_t)(int16_t)READ16(REGISTERS[f.reg1] + f.disp);
}

static inline void LD_W(struct VB_Core* vb, struct Format6 f) {
  // read word from port (same as IN_W)
  REGISTERS[f.reg2] = READ32(REGISTERS[f.reg1] + f.disp);
}


// [Store and Output]
static inline void OUT_B(struct VB_Core* vb, struct Format6 f) {
  // store byte
  WRITE8(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}

static inline void OUT_H(struct VB_Core* vb, struct Format6 f) {
  // store halfword
  WRITE16(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}

static inline void OUT_W(struct VB_Core* vb, struct Format6 f) {
  // store word
  WRITE32(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}

static inline void ST_B(struct VB_Core* vb, struct Format6 f) {
  // store byte
  WRITE8(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}

static inline void ST_H(struct VB_Core* vb, struct Format6 f) {
  // store halfword
  WRITE16(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}

static inline void ST_W(struct VB_Core* vb, struct Format6 f) {
  // store word
  WRITE32(REGISTERS[f.reg1] + f.disp, REGISTERS[f.reg2]);
}


// [Arithmetic]
static inline uint32_t add_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  uint32_t result;

  #if USE_BUILTIN && HAS_BUILTIN(__builtin_add_overflow)
    FLAG_CY = __builtin_add_overflow(a, b, &result);
  #else
    result = a + b;
    FLAG_CY = (uint64_t)a + (uint64_t)b > UINT32_MAX;
  #endif

  FLAG_Z = result == 0;
  FLAG_S = bit_is_set(31, result);
  FLAG_OV = calc_v_flag_add_sub(a, b, result);

  return result;
}

static inline void ADD_imm(struct VB_Core* vb, struct Format2 f) {
  const int32_t imm = bit_sign_extend(4, f.lo5);
  REGISTERS[f.reg2] = add_internal(vb, REGISTERS[f.reg2], imm);
}

static inline void ADD_reg(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = add_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void ADDI(struct VB_Core* vb, struct Format5 f) {
  const int32_t imm = bit_sign_extend(15, f.imm);
  REGISTERS[f.reg2] = add_internal(vb, REGISTERS[f.reg2], imm);
}

static inline uint32_t sub_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  uint32_t result;

  #if USE_BUILTIN && HAS_BUILTIN(__builtin_sub_overflow)
    FLAG_CY = __builtin_sub_overflow(a, b, &result);
  #else
    result = a - b;
    FLAG_CY = b > a;
  #endif

  FLAG_Z = result == 0;
  FLAG_S = bit_is_set(31, result);
  FLAG_OV = calc_v_flag_add_sub(a, ~b, result);

  return result;
}

static inline void CMP_imm(struct VB_Core* vb, struct Format2 f) {
  const uint32_t imm = bit_sign_extend(4, f.lo5);
  sub_internal(vb, REGISTERS[f.reg2], imm);
}

static inline void CMP_reg(struct VB_Core* vb, struct Format1 f) {
  sub_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void SUB_reg(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = sub_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void MUL(struct VB_Core* vb, struct Format1 f) {
  // signed mul
  assert(0);
  const int64_t result = (int64_t)(int32_t)REGISTERS[f.reg2] * (int64_t)(int32_t)REGISTERS[f.reg1];

  FLAG_Z = result == 0;
  FLAG_S = result < 0;
  FLAG_OV = (uint64_t)result != (uint32_t)result;

  REGISTERS[30] = result >> 32; // upper half
  REGISTERS[f.reg2] = result; // lower half
}

static inline void MULU(struct VB_Core* vb, struct Format1 f) {
  // unsigned mul
  assert(0);
  const uint64_t result = (uint64_t)REGISTERS[f.reg2] * (uint64_t)REGISTERS[f.reg1];

  FLAG_Z = result == 0;
  FLAG_S = (result >> 63) & 1;
  FLAG_OV = (uint64_t)result != (uint32_t)result;

  REGISTERS[30] = result >> 32; // upper half
  REGISTERS[f.reg2] = result; // lower half
}


// [Bitwise]
static inline void set_bitwise_flags(struct VB_Core* vb, uint32_t result) {
  FLAG_Z = result == 0;
  FLAG_S = bit_is_set(31, result);
  FLAG_OV = 0;
}

static inline uint32_t and_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint32_t result = a & b;
  set_bitwise_flags(vb, result);
  return result;
}

static inline void AND(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = and_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void ANDI(struct VB_Core* vb, struct Format5 f) {
  REGISTERS[f.reg2] = and_internal(vb, REGISTERS[f.reg1], f.imm);
}

static inline void NOT(struct VB_Core* vb, struct Format1 f) {
  const uint32_t result = ~REGISTERS[f.reg1];
  set_bitwise_flags(vb, result);
  REGISTERS[f.reg2] = result;
}

static inline uint32_t or_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint32_t result = a | b;
  set_bitwise_flags(vb, result);
  return result;
}

static inline void OR(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = or_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void ORI(struct VB_Core* vb, struct Format5 f) {
  REGISTERS[f.reg2] = or_internal(vb, REGISTERS[f.reg1], f.imm);
}

static inline uint32_t sar_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint8_t shift_v = (b & 0x1F);
  const uint32_t result = (int32_t)a >> shift_v;

  set_bitwise_flags(vb, result);
  FLAG_CY = shift_v == 0 ? 0 : bit_is_set(shift_v - 1, result);

  return result;
}

static inline void SAR(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = sar_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void SARI(struct VB_Core* vb, struct Format2 f) {
  REGISTERS[f.reg2] = sar_internal(vb, REGISTERS[f.reg2], f.lo5);
}

static inline uint32_t shl_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint8_t shift_v = (b & 0x1F);
  const uint32_t result = a << shift_v;

  set_bitwise_flags(vb, result);
  FLAG_CY = shift_v == 0 ? 0 : bit_is_set(32 - shift_v, result);

  return result;
}

static inline void SHL(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = shl_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void SHLI(struct VB_Core* vb, struct Format2 f) {
  REGISTERS[f.reg2] = shl_internal(vb, REGISTERS[f.reg2], f.lo5);
}

static inline uint32_t shr_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint8_t shift_v = (b & 0x1F);
  const uint32_t result = a >> shift_v;

  set_bitwise_flags(vb, result);
  FLAG_CY = shift_v == 0 ? 0 : bit_is_set(shift_v - 1, result);

  return result;
}

static inline void SHR(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = shr_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void SHRI(struct VB_Core* vb, struct Format2 f) {
  REGISTERS[f.reg2] = shr_internal(vb, REGISTERS[f.reg2], f.lo5);
}

static inline uint32_t xor_internal(struct VB_Core* vb, uint32_t a, uint32_t b) {
  const uint32_t result = a ^ b;
  set_bitwise_flags(vb, result);
  return result;
}

static inline void XOR(struct VB_Core* vb, struct Format1 f) {
  REGISTERS[f.reg2] = xor_internal(vb, REGISTERS[f.reg2], REGISTERS[f.reg1]);
}

static inline void XORI(struct VB_Core* vb, struct Format5 f) {
  REGISTERS[f.reg2] = xor_internal(vb, REGISTERS[f.reg1], f.imm);
}


// [CPU Control]
static inline void Bcond(struct VB_Core* vb, struct Format3 f) {
  #define GOTO_IF(c) if (c) { goto take_branch; } break;

  switch (f.cond & 15) {
    case BV: GOTO_IF(FLAG_OV);
    case BC: GOTO_IF(FLAG_CY);
    case BE: GOTO_IF(FLAG_Z);
    case BNH: GOTO_IF(FLAG_CY || FLAG_Z);
    case BN: GOTO_IF(FLAG_S);
    case BR: GOTO_IF(true);
    case BLT: GOTO_IF(FLAG_OV != FLAG_S);
    case BLE: GOTO_IF((FLAG_OV != FLAG_S) || FLAG_Z);
    case BNV: GOTO_IF(!FLAG_OV);
    case BNC: GOTO_IF(!FLAG_CY);
    case BNE: GOTO_IF(!FLAG_Z);
    case BH: GOTO_IF(!(FLAG_CY || FLAG_Z));
    case BP: GOTO_IF(!FLAG_S);
    case NOP: GOTO_IF(false);
    case BGE: GOTO_IF(!(FLAG_OV != FLAG_S));
    case BGT: GOTO_IF(!((FLAG_OV != FLAG_S) || FLAG_Z));
  }

  #undef GOTO_IF

  /* no branch taken... */
  vb_log("no jump with cond %u\n", f.cond);
  return;

take_branch:
  vb_log("jump with cond %u disp %d\n", f.cond, f.disp);
  // NOTE: the disp is applied to the original pc, before incrementing
  // after opcode fetch.
  // An example of this is in [Jack Bros] where is jumps with disp = -2.
  // the result of this should be the instruction before the Bcond instruction.
  // likely being a simple [while (FLAG) { value++; }] loop.
  REG_PC =  (REG_PC - 2) + f.disp;
  REG_PC = align_16(REG_PC);
}

static inline void HALT(struct VB_Core* vb, struct Format2 f) {
  VB_UNUSED(f);
  assert(!"halt called!");
  CPU.halted = true;
}

static inline void JAL(struct VB_Core* vb, struct Format4 f) {
  // PC is already 4-bytes ahead due to opcode fetch and format4 disp fetch
  REGISTERS[LINK_POINTER] = REG_PC;
  REG_PC = (REG_PC - 4) + f.disp;
  REG_PC = align_16(REG_PC);
  vb_log("REG_PC %08X; LR: %08X\n", REG_PC, REGISTERS[LINK_POINTER]);
}

static inline void JMP(struct VB_Core* vb, struct Format1 f) {
  REG_PC = REGISTERS[f.reg1];
  REG_PC = align_16(REG_PC);
}

static inline void JR(struct VB_Core* vb, struct Format4 f) {
  // PC is already 4-bytes ahead due to opcode fetch and format4 disp fetch
  REG_PC = (REG_PC - 4) + f.disp;
  REG_PC = align_16(REG_PC);
}

static inline void LDSR(struct VB_Core* vb, struct Format2 f) {
  const uint32_t value = REGISTERS[f.reg2];

  switch (f.lo5 & 31) {
    case ADTRE:
      vb_log_fatal("[ADTRE] unimpl write: 0x%08X\n", value);
      break;

    case CHCW:
      printf("[CHCW] unimpl write: 0x%08X\n", value);
      break;

    case ECR:
      assert(!"[ECR] cannot be modified by LDSR");
      break;

    case EIPC:
      vb_log_fatal("[EIPC] unimpl write: 0x%08X\n", value);
      break;

    case EIPSW:
      vb_log_fatal("[EIPSW] unimpl write: 0x%08X\n", value);
      break;

    case FEPC:
      vb_log_fatal("[FEPC] unimpl write: 0x%08X\n", value);
      break;

    case FEPSW:
      vb_log_fatal("[FEPSW] unimpl write: 0x%08X\n", value);
      break;

    case PIR:
      vb_log_fatal("[PIR] unimpl write: 0x%08X\n", value);
      break;

    case PSW:
      CPU.PSW.Z = bit_is_set(0, value);
      CPU.PSW.S = bit_is_set(1, value);
      CPU.PSW.OV = bit_is_set(2, value);
      CPU.PSW.CY = bit_is_set(3, value);
      CPU.PSW.FPR = bit_is_set(4, value);
      CPU.PSW.FUD = bit_is_set(5, value);
      CPU.PSW.FOV = bit_is_set(6, value);
      CPU.PSW.FZD = bit_is_set(7, value);
      CPU.PSW.FIV = bit_is_set(8, value);
      CPU.PSW.FRO = bit_is_set(9, value);
      CPU.PSW.ID = bit_is_set(12, value);
      CPU.PSW.AE = bit_is_set(13, value);
      CPU.PSW.EP = bit_is_set(14, value);
      CPU.PSW.NP = bit_is_set(15, value);
      CPU.PSW.I = bit_get_range(16, 19, value);
      printf("[PSW] unimpl write: 0x%08X\n", value);
      break;

    case TKCW:
      vb_log_fatal("[TKCW] unimpl write: 0x%08X\n", value);
      break;

    case ABS:
      // the case is needed else compiler warns about value being unsigned.
      CPU.ABS = abs((int32_t)value);
      break;

    default:
      vb_log_fatal("[unk] unimpl write: 0x%08X index: %u\n", value, f.lo5);
      break;
  }
}

static inline void STSR(struct VB_Core* vb, struct Format2 f) {
  uint32_t result = 0;

  switch (f.lo5 & 31) {
    case ADTRE:
      vb_log_fatal("[ADTRE] unimpl read\n");
      break;

    case CHCW:
      vb_log_fatal("[CHCW] unimpl read\n");
      break;

    case ECR:
      assert(!"[ECR] cannot be modified by LDSR");
      break;

    case EIPC:
      vb_log_fatal("[EIPC] unimpl read\n");
      break;

    case EIPSW:
      vb_log_fatal("[EIPSW] unimpl read\n");
      break;

    case FEPC:
      vb_log_fatal("[FEPC] unimpl read\n");
      break;

    case FEPSW:
      vb_log_fatal("[FEPSW] unimpl read\n");
      break;

    case PIR:
      vb_log_fatal("[PIR] unimpl read\n");
      break;

    case PSW:
      result |= CPU.PSW.Z << 0;
      result |= CPU.PSW.S << 1;
      result |= CPU.PSW.OV << 2;
      result |= CPU.PSW.CY << 3;
      result |= CPU.PSW.FPR << 4;
      result |= CPU.PSW.FUD << 5;
      result |= CPU.PSW.FOV << 6;
      result |= CPU.PSW.FZD << 7;
      result |= CPU.PSW.FIV << 8;
      result |= CPU.PSW.FRO << 9;
      result |= CPU.PSW.ID << 12;
      result |= CPU.PSW.AE << 13;
      result |= CPU.PSW.EP << 14;
      result |= CPU.PSW.NP << 15;
      result |= CPU.PSW.I << 16;
      printf("[PSW] unimpl read\n");
      break;

    case TKCW:
      vb_log_fatal("[TKCW] unimpl read\n");
      break;

    case ABS:
      // the case is needed else compiler warns about value being unsigned.
      result = CPU.ABS;
      break;

    default:
      vb_log_fatal("[unk] unimpl read, index: %u\n", f.lo5);
      break;
  }

  REGISTERS[f.reg2] = result;
}


// [Nintendo - Standalone]
static inline void CLI(struct VB_Core* vb) {
  CPU.PSW.ID = false;
  // cycles 12; // why is this instruction so slow???
  // assert(0);
}

static inline void SEI(struct VB_Core* vb) {
  CPU.PSW.ID = true;
  // cycles 12; // why is this instruction so slow???
  // assert(0);
}


// [Nintendo - Extended]
static inline void MPYHW(struct VB_Core* vb, struct Format7 f) {
  const int32_t a = REGISTERS[f.reg2];
  const int32_t b = bit_sign_extend(16, REGISTERS[f.reg1]);
  REGISTERS[f.reg2] = a * b;
}

static inline void REV(struct VB_Core* vb, struct Format7 f) {
  const uint32_t value = REGISTERS[f.reg2];
  uint32_t result = 0;

  #if USE_BUILTIN && HAS_BUILTIN(__builtin_bitreverse32)
    result = __builtin_bitreverse32(value);
  #else
    for (size_t i = 0; i < 32; i++) {
      result |= bit_is_set(31 - i, value) << i;
    }
  #endif

  REGISTERS[f.reg2] = result;
}

static inline void XB(struct VB_Core* vb, struct Format7 f) {
  const uint32_t reg2 = REGISTERS[f.reg2];
  REGISTERS[f.reg2] = (reg2 & 0xFFFF0000) | ((reg2 << 8) & 0xFF00) | ((reg2 >> 8) & 0x00FF);
}

static inline void XH(struct VB_Core* vb, struct Format7 f) {
  const uint32_t reg2 = REGISTERS[f.reg2];
  REGISTERS[f.reg2] = (reg2 >> 16) | (reg2 << 16);
}


// [Floating-Point]
static inline void set_float_flags(struct VB_Core* vb, float result) {
  FLAG_Z = fpclassify(result) == FP_ZERO;
  FLAG_S = signbit(result);
  FLAG_OV = 0;
  FLAG_CY = signbit(result); // ???

  // FLT_MAX
  // FLT_MIN
  // isnan
  //

  // FPR =
  // FUD =
  // FOV =
  // FRO =
}

// Add Floating Short 	reg2 = reg2 + reg1
static inline void ADDF_S(struct VB_Core* vb, struct Format7 f) {
  const float a = REGISTERS[f.reg2];
  const float b = REGISTERS[f.reg1];
  const float result = a + b;

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Compare Floating Short 	(discard) = reg2 - reg1
static inline void CMPF_S(struct VB_Core* vb, struct Format7 f) {
  const float a = REGISTERS[f.reg2];
  const float b = REGISTERS[f.reg1];
  const float result = a - b;

  set_float_flags(vb, result);
}

// Convert Short Floating to Word Integer 	reg2 = (word) round(reg1)
static inline void CVT_SW(struct VB_Core* vb, struct Format7 f) {
  const float result = roundf(REGISTERS[f.reg1]);

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Convert Word Integer to Short Floating 	reg2 = (float) reg1
static inline void CVT_WS(struct VB_Core* vb, struct Format7 f) {
  const float result = (float)REGISTERS[f.reg1];

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Divide Floating Short 	reg2 = reg2 / reg1
static inline void DIVF_S(struct VB_Core* vb, struct Format7 f) {
  const float a = REGISTERS[f.reg2];
  const float b = REGISTERS[f.reg1];
  assert((a > 0.0F || a < 0.0F) && (b > 0.0F || b < 0.0F));
  const float result = a / b;

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Multiply Floating Short 	reg2 = reg2 * reg1
static inline void MULF_S(struct VB_Core* vb, struct Format7 f) {
  const float a = REGISTERS[f.reg2];
  const float b = REGISTERS[f.reg1];
  const float result = a * b;

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Subtract Floating Short 	reg2 = reg2 - reg1
static inline void SUBF_S(struct VB_Core* vb, struct Format7 f) {
  const float a = REGISTERS[f.reg2];
  const float b = REGISTERS[f.reg1];
  const float result = a - b;

  set_float_flags(vb, result);

  REGISTERS[f.reg2] = result;
}

// Truncate Short Floating to Word Integer 	reg2 = (word) truncate(reg1)
static inline void TRNC_S(struct VB_Core* vb, struct Format7 f) {
  const float result = truncf(REGISTERS[f.reg1]);

  FLAG_Z = fpclassify(result) == FP_ZERO;
  FLAG_S = signbit(result);
  FLAG_OV = 0;

  REGISTERS[f.reg2] = result;
}

static void sub_execute_float(struct VB_Core* vb, struct Format7 f) {
  switch (f.subop & 0x3F) {
  // [Floating-Point]
    case 0x04:
      vb_log("[CPU] ADDF.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      ADDF_S(vb, f);
      break;

    case 0x00:
      vb_log("[CPU] CMPF.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      CMPF_S(vb, f);
      break;

    case 0x03:
      vb_log("[CPU] CVT.SW\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      CVT_SW(vb, f);
      break;

    case 0x02:
      vb_log("[CPU] CVT.WS\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      CVT_WS(vb, f);
      break;

    case 0x07:
      vb_log("[CPU] DIVF.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      DIVF_S(vb, f);
      break;

    case 0x06:
      vb_log("[CPU] MULF.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      MULF_S(vb, f);
      break;

    case 0x05:
      vb_log("[CPU] SUBF.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      SUBF_S(vb, f);
      break;

    case 0x0B:
      vb_log("[CPU] TRNC.S\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      TRNC_S(vb, f);
      break;

  // [Nintendo Extended]
    case 0x0C:
      vb_log("[CPU] MPYHW\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      MPYHW(vb, f);
      break;

    case 0x0A:
      vb_log("[CPU] REV\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      REV(vb, f);
      break;

    case 0x08:
      vb_log("[CPU] XB\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      XB(vb, f);
      break;

    case 0x09:
      vb_log("[CPU] XH\tFormat 7\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      XH(vb, f);
      break;

    default:
      assert(0);
      break;
  }
}

static void sub_execute_bit_strings(struct VB_Core* vb, uint16_t opa) {
  VB_UNUSED(opa);

  const uint16_t opcode = READ16(REG_PC);
  REG_PC += 2;

  switch ((opcode >> 11) & 0x1F) {
  // [Bitwise]
    case 0x09:
      vb_log("[CPU] ANDBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0D:
      vb_log("[CPU] ANDNBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0B:
      vb_log("[CPU] MOVBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x1F:
      vb_log("[CPU] NOTBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x08:
      vb_log("[CPU] ORBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0C:
      vb_log("[CPU] ORNBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0A:
      vb_log("[CPU] XORBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0E:
      vb_log("[CPU] XORNBSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

  // [Search]
    case 0x01:
      vb_log("[CPU] SCH0BSD\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x00:
      vb_log("[CPU] SCH0BSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x03:
      vb_log("[CPU] SCH1BSD\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x02:
      vb_log("[CPU] SCH1BSU\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    default:
      assert(0);
      break;
  }
}

static void execute(struct VB_Core* vb) {
  const uint16_t opcode = READ16(REG_PC);
  REG_PC += 2;

  switch ((opcode >> 10) & 0x3F) {
  // [Register Transfer]
    case 0x10: // 0b010000
      vb_log("[CPU] MOV\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      MOV_imm(vb, gen_format2(vb, opcode));
      break;

    case 0x00: // 0b000000
      vb_log("[CPU] MOV\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      MOV_reg(vb, gen_format1(vb, opcode));
      break;

    case 0x28: // 0b101000
      vb_log("[CPU] MOVEA\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      MOVEA(vb, gen_format5(vb, opcode));
      break;

    case 0x2F: // 0b101111
      vb_log("[CPU] MOVHI\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      MOVHI(vb, gen_format5(vb, opcode));
      break;

  // [Load and Input]
    case 0x38: // 0b111000
      vb_log("[CPU] IN.B\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      IN_B(vb, gen_format6(vb, opcode));
      break;

    case 0x39: // 0b111001
      vb_log("[CPU] IN.H\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      IN_H(vb, gen_format6(vb, opcode));
      break;

    case 0x3B: // 0b111011
      vb_log("[CPU] IN.W\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      IN_W(vb, gen_format6(vb, opcode));
      break;

    case 0x30: // 0b110000
      vb_log("[CPU] LD.B\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      LD_B(vb, gen_format6(vb, opcode));
      break;

    case 0x31: // 0b110001
      vb_log("[CPU] LD.H\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      LD_H(vb, gen_format6(vb, opcode));
      break;

    case 0x33: // 0b110011
      vb_log("[CPU] LD.W\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      LD_W(vb, gen_format6(vb, opcode));
      break;

  // [Store and Output]
    case 0x3C: // 0b111100
      vb_log("[CPU] OUT.B\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      OUT_B(vb, gen_format6(vb, opcode));
      break;

    case 0x3D: // 0b111101
      vb_log("[CPU] OUT.H\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      OUT_H(vb, gen_format6(vb, opcode));
      break;

    case 0x3F: // 0b111111
      vb_log("[CPU] OUT.W\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      OUT_W(vb, gen_format6(vb, opcode));
      break;

    case 0x34: // 0b110100
      vb_log("[CPU] ST.B\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      ST_B(vb, gen_format6(vb, opcode));
      break;

    case 0x35: // 0b110101
      vb_log("[CPU] ST.H\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      ST_H(vb, gen_format6(vb, opcode));
      break;

    case 0x37: // 0b110111
      vb_log("[CPU] ST.W\tFormat 6\tCOUNT [%zu]\n", vb->v810.step_count);
      ST_W(vb, gen_format6(vb, opcode));
      break;

  // [Arithmetic]
    case 0x11: // 0b010001
      vb_log("[CPU] ADD\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      ADD_imm(vb, gen_format2(vb, opcode));
      break;

    case 0x01: // 0b000001
      vb_log("[CPU] ADD\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      ADD_reg(vb, gen_format1(vb, opcode));
      break;

    case 0x29: // 0b101001
      vb_log("[CPU] ADDI\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      ADDI(vb, gen_format5(vb, opcode));
      break;

    case 0x13: // 0b010011
      vb_log("[CPU] CMP\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      CMP_imm(vb, gen_format2(vb, opcode));
      break;

    case 0x03: // 0b000011
      vb_log("[CPU] CMP\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      CMP_reg(vb, gen_format1(vb, opcode));
      break;

    case 0x09: // 0b001001
      vb_log("[CPU] DIV\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x0B: // 0b001011
      vb_log("[CPU] DIVU\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x08: // 0b001000
      vb_log("[CPU] MUL\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      MUL(vb, gen_format1(vb, opcode));
      break;

    case 0x0A: // 0b001010
      vb_log("[CPU] MULU\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      MULU(vb, gen_format1(vb, opcode));
      break;

    case 0x02: // 0b000010
      vb_log("[CPU] SUB\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      SUB_reg(vb, gen_format1(vb, opcode));
      break;

  // [Bitwise]
    case 0x0D:
      vb_log("[CPU] AND\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      AND(vb, gen_format1(vb, opcode));
      break;

    case 0x2D:
      vb_log("[CPU] ANDI\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      ANDI(vb, gen_format5(vb, opcode));
      break;

    case 0x0F:
      vb_log("[CPU] NOT\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      NOT(vb, gen_format1(vb, opcode));
      break;

    case 0x0C:
      vb_log("[CPU] OR\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      OR(vb, gen_format1(vb, opcode));
      break;

    case 0x2C:
      vb_log("[CPU] ORI\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      ORI(vb, gen_format5(vb, opcode));
      break;

    case 0x17:
      vb_log("[CPU] SAR\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      SARI(vb, gen_format2(vb, opcode));
      break;

    case 0x07:
      vb_log("[CPU] SAR\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      SAR(vb, gen_format1(vb, opcode));
      break;

    case 0x14:
      vb_log("[CPU] SHL\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      SHLI(vb, gen_format2(vb, opcode));
      break;

    case 0x04:
      vb_log("[CPU] SHL\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      SHL(vb, gen_format1(vb, opcode));
      break;

    case 0x15:
      vb_log("[CPU] SHR\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      SHRI(vb, gen_format2(vb, opcode));
      break;

    case 0x05:
      vb_log("[CPU] SHR\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      SHR(vb, gen_format1(vb, opcode));
      break;

    case 0x0E:
      vb_log("[CPU] XOR\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      XOR(vb, gen_format1(vb, opcode));
      break;

    case 0x2E:
      vb_log("[CPU] XORI\tFormat 5\tCOUNT [%zu]\n", vb->v810.step_count);
      XORI(vb, gen_format5(vb, opcode));
      break;

  // [CPU Control]
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
      vb_log("[CPU] Bcond\tFormat 3\tCOUNT [%zu]\n", vb->v810.step_count);
      Bcond(vb, gen_format3(vb, opcode));
      break;

    case 0x1A: // 0b011010:
      vb_log("[CPU] HALT\tFormat 3\tCOUNT [%zu]\n", vb->v810.step_count);
      HALT(vb, gen_format2(vb, opcode));
      break;

    case 0x2B:
      vb_log("[CPU] JAL\tFormat 4\tCOUNT [%zu]\n", vb->v810.step_count);
      JAL(vb, gen_format4(vb, opcode));
      break;

    case 0x06:
      vb_log("[CPU] JMP\tFormat 1\tCOUNT [%zu]\n", vb->v810.step_count);
      JMP(vb, gen_format1(vb, opcode));
      break;

    case 0x2A:
      vb_log("[CPU] JR\tFormat 4\tCOUNT [%zu]\n", vb->v810.step_count);
      JR(vb, gen_format4(vb, opcode));
      break;

    case 0x1C: // 0b011100
      vb_log("[CPU] LDSR\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      LDSR(vb, gen_format2(vb, opcode));
      break;

    case 0x19:
      vb_log("[CPU] RETI\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

    case 0x1D:
      vb_log("[CPU] STSR\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      // assert(!"instruction not implemented!");
      STSR(vb, gen_format2(vb, opcode));
      break;

    case 0x18:
      vb_log("[CPU] TRAP\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

  // [Floating-Point] - [Nintendo - Extended]
    case 0x3E:
      sub_execute_float(vb, gen_format7(vb, opcode));
      break;

  // [Bit Strings]
    case 0x1F:
      sub_execute_bit_strings(vb, opcode);
      break;

  // [CAXI]
    case 0x3A:
      vb_log("[CPU] CAXI\tFormat 4\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

  // [SETF]
    case 0x12:
      vb_log("[CPU] SETF\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      assert(!"instruction not implemented!");
      break;

  // [Nintendo - Standalone]
    case 0x16:
      vb_log("[CPU] CLI\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      // assert(!"instruction not implemented!");
      CLI(vb);
      break;

    case 0x1E:
      vb_log("[CPU] SEI\tFormat 2\tCOUNT [%zu]\n", vb->v810.step_count);
      // assert(!"instruction not implemented!");
      SEI(vb);
      break;

    default:
      assert(!"UNK instruction!");
      break;
  }
}


void vb_v810_run(struct VB_Core* vb) {
  REGISTERS[ZERO_REGISTER] = 0; // forced to zero
  execute(vb);

  if (CPU.PSW.ID) {
    // vb_log_fatal("[CPU] ID is set!\tCOUNT [%zu]\n", vb->v810.step_count);
  }
}

void vb_v810_reset(struct VB_Core* vb) {
  memset(&vb->v810, 0, sizeof(vb->v810));

  vb->v810.PC = 0xFFFFFFF0;
  vb->v810.ECR.FECC = 0x0000;
  vb->v810.ECR.EICC = 0xFFF0;
  vb->v810.PSW.NP = 1;
  vb->v810.PIR = 0x00005346;
  vb->v810.registers[ZERO_REGISTER] = 0x00000000;
}
