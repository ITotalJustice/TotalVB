#include "core/vb.h"
#include "core/internal.h"

#include <stdio.h>
#include <assert.h>


#define CPU vb->cpu
#define REGISTERS CPU.registers
#define REG_PC CPU.PC


// FLAGS
#define FLAG_Z CPU.PSW.Z
#define FLAG_S CPU.PSW.S
#define FLAG_OV CPU.PSW.OV
#define FLAG_CY CPU.PSW.CY


// helpers that half / word align the address when reading
#define READ8(addr) VB_bus_read_8(vb, (addr))
#define READ16(addr) VB_bus_read_16(vb, (addr) & ~(0x1))
#define READ32(addr) VB_bus_read_32(vb, (addr) & ~(0x3))


static inline bool is_bit_set(const uint8_t bit, const uint32_t value) {
    assert(bit < (sizeof(uint32_t) * 8) && "bit value out of bounds!");
    return (value & (1U << bit)) > 0;
}

static inline uint32_t get_bit_range(const uint8_t start, const uint8_t end, const uint32_t value) {
    assert(end > start && "invalid bit range!");

    return (value & (0xFFFFFFFF >> (31 - end))) >> start;
}

static inline int32_t sign_extend(const uint8_t start_size, const uint32_t value) {
    assert(start_size < 31 && "bit start size is out of bounds!");

    const uint8_t bits = 31 - start_size;
    return ((int32_t)(value << bits)) >> bits;
}


/* used for aligning the PC after modifying it. */
/* this is generic as it may need to be used for other values. */
static inline uint32_t align_16(const uint32_t v) { return v & ~(0x1); }
static inline uint32_t align_32(const uint32_t v) { return v & ~(0x3); }


/* helper for v-flag calcs, ONLY used for add / sub (mults are different!) */
static inline bool calc_v_flag_add_sub(const uint32_t a, const uint32_t b, const uint32_t r) {
    return (is_bit_set(31, a) == is_bit_set(31, b)) && (is_bit_set(31, a) != is_bit_set(31, r));
}


/* There are 6 unique formats for instruction encoding */
/* 1-6 have been implemeted, format7 is basically identical to format1. */
struct Format1 {
    uint8_t opcode; // 6-bits
    uint8_t reg2;   // 5-bits
    uint8_t reg1;   // 5-bits
};

struct Format2 {
    uint8_t opcode; // 6-bits
    uint8_t reg2;   // 5-bits
    uint8_t lo5;    // 5-bits
};

struct Format3 {
    uint8_t opcode; // 3-bits
    uint8_t cond;   // 4-bits
    int32_t disp;   // TODO: this gets sign extended (16 or 32?)
};

struct Format4 {
    uint8_t opcode; // 6-bits
    int32_t disp;   // sign extended
};

struct Format5 {
    uint8_t opcode; // 6-bits
    uint8_t reg2;   // 5-bits
    uint8_t reg1;   // 5-bits
    uint16_t imm;   // fetched
};

struct Format6 {
    uint8_t opcode; // 6-bits
    uint8_t reg2;   // 5-bits
    uint8_t reg1;   // 5-bits
    int32_t disp;   // sign extended
};


// [DEBUG]
static void log_format1(const struct VB_Core* vb, const struct Format1 f) {
    VB_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X\n\n",
        f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1]);
}

static void log_format2(const struct VB_Core* vb, const struct Format2 f) {
    VB_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\tlo5: %u\n\n",
        f.opcode, f.reg2, REGISTERS[f.reg2], f.lo5);
}

static void log_format3(const struct VB_Core* vb, const struct Format3 f) {
    VB_log("OPCODE: 0x%02X\tcond: 0x%X\tdisp: %d\n\n",
        f.opcode, f.cond, f.disp);
}

static void log_format4(const struct VB_Core* vb, const struct Format4 f) {
    VB_log("OPCODE: 0x%02X\tdisp: %d\n\n",
        f.opcode, f.disp);
}

static void log_format5(const struct VB_Core* vb, const struct Format5 f) {
    VB_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X; imm: 0x%X\n\n",
        f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1], f.imm);
}

static void log_format6(const struct VB_Core* vb, const struct Format6 f) {
    VB_log("OPCODE: 0x%02X\treg2-idx: %u\treg2: 0x%08X\treg1-idx: %u\treg1: 0x%08X\tdisp: %d\n\n",
        f.opcode, f.reg2, REGISTERS[f.reg2], f.reg1, REGISTERS[f.reg1], f.disp);
}


// [GEN]
static inline struct Format1 gen_format1(struct VB_Core* vb, uint16_t op) {
    const struct Format1 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .reg1 = get_bit_range(0, 4, op),
    };

    log_format1(vb, f);
    return f;
}

static inline struct Format2 gen_format2(struct VB_Core* vb, uint16_t op) {
    const struct Format2 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .lo5 = get_bit_range(0, 4, op),
    };

    log_format2(vb, f);
    return f;
}

static inline struct Format3 gen_format3(struct VB_Core* vb, uint16_t op) {
    const uint32_t disp_range = get_bit_range(0, 8, op);

    const struct Format3 f = {
        .opcode = get_bit_range(13, 15, op),
        .cond = get_bit_range(9, 12, op),
        .disp = sign_extend(8, disp_range),
    };

    log_format3(vb, f);
    return f;
}

static inline struct Format4 gen_format4(struct VB_Core* vb, uint16_t op) {
    const uint32_t lo_disp = READ16(REG_PC);
    const uint32_t hi_disp = get_bit_range(0, 9, op);
    const uint32_t disp = (hi_disp << 16) | lo_disp;
    REG_PC += 2;

    const struct Format4 f = {
        .opcode = get_bit_range(10, 15, op),
        .disp = sign_extend(26, disp),
    };

    log_format4(vb, f);
    return f;
}

static inline struct Format5 gen_format5(struct VB_Core* vb, uint16_t op) {
    const uint32_t imm = READ16(REG_PC);
    REG_PC += 2;

    const struct Format5 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .reg1 = get_bit_range(0, 4, op),
        .imm = imm,
    };

    log_format5(vb, f);
    return f;
}

static inline struct Format6 gen_format6(struct VB_Core* vb, uint16_t op) {
    const uint32_t disp = READ16(REG_PC);
    REG_PC += 2;

    const struct Format6 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .reg1 = get_bit_range(0, 4, op),
        .disp = sign_extend(16, disp),
    };

    log_format6(vb, f);
    return f;
}


// [Register Transfer]
static inline void MOV_imm(struct VB_Core* vb, struct Format2 f) {
    REGISTERS[f.reg2] = sign_extend(15, f.lo5);
}

static inline void MOV_reg(struct VB_Core* vb, struct Format1 f) {
    REGISTERS[f.reg2] = REGISTERS[f.reg1];
}

static inline void MOVEA(struct VB_Core* vb, struct Format5 f) {
    const int32_t imm = sign_extend(15, f.imm);
    REGISTERS[f.reg2] = REGISTERS[f.reg1] + imm;
}

static inline void MOVHI(struct VB_Core* vb, struct Format5 f) {
    const uint32_t imm = f.imm << 16;
    REGISTERS[f.reg2] = REGISTERS[f.reg1] + imm;
}


// [Arithmetic]
#define _ADD(dst, va, vb) do { \
    const uint32_t result = va + vb; \
    \
    /* update flags...*/ \
    FLAG_Z = result == 0; \
    FLAG_S = is_bit_set(31, result); \
    FLAG_OV = calc_v_flag_add_sub(va, vb, result); \
    FLAG_CY = (va + vb) > UINT32_MAX; \
    \
    dst = result; \
} while(0)

static inline void ADD_imm(struct VB_Core* vb, struct Format2 f) {
    const int32_t imm = sign_extend(5, f.lo5);
    _ADD(
        /* dst */ REGISTERS[f.reg2],
        /* va */ REGISTERS[f.reg2],
        /* vb */ imm
    );
}

static inline void ADD_reg(struct VB_Core* vb, struct Format1 f) {
    _ADD(
        /* dst */ REGISTERS[f.reg2],
        /* va */ REGISTERS[f.reg2],
        /* vb */ REGISTERS[f.reg1]
    );
}

static inline void ADDI(struct VB_Core* vb, struct Format5 f) {
    const int32_t imm = sign_extend(15, f.imm);
    _ADD(
        /* dst */ REGISTERS[f.reg2],
        /* va */ REGISTERS[f.reg1],
        /* vb */ imm
    );
}

#define _SUB(dst, va, vb) do { \
    const uint32_t result = va - vb; \
    \
    /* update flags...*/ \
    FLAG_Z = result == 0; \
    FLAG_S = is_bit_set(31, result); \
    FLAG_OV = calc_v_flag_add_sub(va, vb, result); \
    FLAG_CY = (va + vb) > UINT32_MAX; \
    \
    dst = result; \
} while(0)

static inline void CMP_imm(struct VB_Core* vb, struct Format2 f) {
    const int32_t imm = sign_extend(5, f.lo5);
    _SUB(
        /* dst unused */ f.opcode,
        /* va */ REGISTERS[f.reg2],
        /* vb */ imm
    );
}

static inline void CMP_reg(struct VB_Core* vb, struct Format1 f) {
    _SUB(
        /* dst unused */ f.opcode,
        /* va */ REGISTERS[f.reg2],
        /* vb */ REGISTERS[f.reg1]
    );
}

static inline void SUB_reg(struct VB_Core* vb, struct Format1 f) {
    _SUB(
        /* dst */ REGISTERS[f.reg2],
        /* va */ REGISTERS[f.reg2],
        /* vb */ REGISTERS[f.reg1]
    );
}


// [Conditional Branch]
static inline void BRANCH(struct VB_Core* vb, struct Format3 f) {
    switch (f.cond) {
        case 0x0: if (FLAG_OV) { goto take_branch; } break;
        case 0x1: if (FLAG_CY) { goto take_branch; } break;
        case 0x2: if (FLAG_Z) { goto take_branch; } break;
        case 0x3: if (FLAG_CY || FLAG_Z) { goto take_branch; } break;
        case 0x4: if (FLAG_S) { goto take_branch; } break;
        case 0x5: if (true) { goto take_branch; } break;
        case 0x6: if (FLAG_OV ^ FLAG_S) { goto take_branch; } break;
        case 0x7: if ((FLAG_OV ^ FLAG_S) || FLAG_Z) { goto take_branch; } break;
        case 0x8: if (!FLAG_OV) { goto take_branch; } break;
        case 0x9: if (!FLAG_CY) { goto take_branch; } break;
        case 0xA: if (!FLAG_Z) { goto take_branch; } break;
        case 0xB: if (!(FLAG_CY || FLAG_Z)) { goto take_branch; } break;
        case 0xC: if (!FLAG_S) { goto take_branch; } break;
        case 0xD: if (false) { goto take_branch; } break;
        case 0xE: if (!(FLAG_OV ^ FLAG_S)) { goto take_branch; } break;
        case 0xF: if (((FLAG_OV ^ FLAG_S) || FLAG_Z)) { goto take_branch; } break;
    }

    /* no branch taken... */
    VB_log("no jump with cond %u\n", f.cond);
    return;

take_branch:
    VB_log("jump with cond %u disp %d\n", f.cond, f.disp);
    // NOTE: i am not sure if the PC should be 2-bytes ahead or not.
    // if PC is 2-bytes ahead (after op fetch), then this loops back to
    // itself in [Jack Bros] by the disp being -2.
    // using mednafen debugger i see it's meant to jump back to ADD and
    // repeat that in a loop...
    REG_PC =  (REG_PC - 2) + f.disp;
    REG_PC = align_16(REG_PC);
}


// [Jump]
static inline void JAL(struct VB_Core* vb, struct Format4 f) {
    // this is already 4-bytes ahead
    REGISTERS[LINK_POINTER] = REG_PC;
    // we don't want to be 4-bytes ahead when we modify PC
    REG_PC = (REG_PC - 4) + f.disp;
    REG_PC = align_16(REG_PC);
    printf("REG_PC %08X; LR: %08X\n", REG_PC, REGISTERS[LINK_POINTER]);
}

static inline void JMP(struct VB_Core* vb, struct Format1 f) {
    REG_PC = REGISTERS[f.reg1];
    REG_PC = align_16(REG_PC);
}

static inline void JR(struct VB_Core* vb, struct Format4 f) {
    REG_PC += f.disp;
    REG_PC = align_16(REG_PC);
}


static void sub_execute_float(struct VB_Core* vb, uint16_t opa) {
    const uint16_t opcode = READ16(REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Floating-Point]
        case 0x04:
            VB_log("[CPU] ADDF.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x00:
            VB_log("[CPU] CMPF.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x03:
            VB_log("[CPU] CVT.SW\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] CVT.WS\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x07:
            VB_log("[CPU] DIVF.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x06:
            VB_log("[CPU] MULF.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x05:
            VB_log("[CPU] SUBF.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] TRNC.S\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Nintendo Extended]
        case 0x0C:
            VB_log("[CPU] MPYHW\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] REV\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] XB\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x09:
            VB_log("[CPU] XH\tFormat 7\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;
    }
}

static void sub_execute_bit_strings(struct VB_Core* vb, uint16_t opa) {
    const uint16_t opcode = READ16(REG_PC);
    REG_PC += 2;

    switch ((opcode >> 11) & 0x1F) {
    // [Bitwise]
        case 0x09:
            VB_log("[CPU] ANDBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0D:
            VB_log("[CPU] ANDNBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] MOVBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x1F:
            VB_log("[CPU] NOTBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] ORBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0C:
            VB_log("[CPU] ORNBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] XORBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0E:
            VB_log("[CPU] XORNBSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Search]
        case 0x01:
            VB_log("[CPU] SCH0BSD\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x00:
            VB_log("[CPU] SCH0BSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x03:
            VB_log("[CPU] SCH1BSD\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] SCH1BSU\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;
    }
}

static void execute_16(struct VB_Core* vb) {
    const uint16_t opcode = READ16(REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Register Transfer]
        case 0x10:
            VB_log("[CPU] MOV\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            MOV_imm(vb, gen_format2(vb, opcode));
            break;

        case 0x00:
            VB_log("[CPU] MOV\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            MOV_reg(vb, gen_format1(vb, opcode));
            break;

        case 0x28:
            VB_log("[CPU] MOVEA\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            MOVEA(vb, gen_format5(vb, opcode));
            break;

        case 0x2F:
            VB_log("[CPU] MOVHI\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            MOVHI(vb, gen_format5(vb, opcode));
            break;

    // [Load and Input]
        case 0x38:
            VB_log("[CPU] IN.B\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x39:
            VB_log("[CPU] IN.H\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x3B:
            VB_log("[CPU] IN.W\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x30:
            VB_log("[CPU] LD.B\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            printf("count %llu\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x31:
            VB_log("[CPU] LD.H\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x33:
            VB_log("[CPU] LD.W\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Store and Output]
        case 0x3C:
            VB_log("[CPU] OUT.B\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x3D:
            VB_log("[CPU] OUT.H\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x3F:
            VB_log("[CPU] OUT.W\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x34:
            VB_log("[CPU] ST.B\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x35:
            VB_log("[CPU] ST.H\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x37:
            VB_log("[CPU] ST.W\tFormat 6\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Arithmetic]
        case 0x11:
            VB_log("[CPU] ADD\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            ADD_imm(vb, gen_format2(vb, opcode));
            break;

        case 0x01:
            VB_log("[CPU] ADD\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            ADD_reg(vb, gen_format1(vb, opcode));
            break;

        case 0x29:
            VB_log("[CPU] ADDI\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            ADDI(vb, gen_format5(vb, opcode));
            break;

        case 0x13:
            VB_log("[CPU] CMP\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            CMP_imm(vb, gen_format2(vb, opcode));
            break;

        case 0x03:
            VB_log("[CPU] CMP\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            CMP_reg(vb, gen_format1(vb, opcode));
            break;

        case 0x09:
            VB_log("[CPU] DIV\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] DIVU\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] MUL\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] MULU\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] SUB\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            SUB_reg(vb, gen_format1(vb, opcode));
            break;

    // [Bitwise]
        case 0x0D:
            VB_log("[CPU] AND\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x2D:
            VB_log("[CPU] ANDI\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0F:
            VB_log("[CPU] NOT\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0C:
            VB_log("[CPU] OR\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x2C:
            VB_log("[CPU] ORI\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x17:
            VB_log("[CPU] SAR\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x07:
            VB_log("[CPU] SAR\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x14:
            VB_log("[CPU] SHL\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x04:
            VB_log("[CPU] SHL\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x15:
            VB_log("[CPU] SHR\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x05:
            VB_log("[CPU] SHR\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x0E:
            VB_log("[CPU] XOR\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x2E:
            VB_log("[CPU] XORI\tFormat 5\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Conditional Branch]
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
            VB_log("[CPU] COND-BRANCH\tFormat 3\tCOUNT [%llu]\n", vb->cpu.step_count);
            BRANCH(vb, gen_format3(vb, opcode));
            break;

    // [JUMP]
        case 0x2B:
            VB_log("[CPU] JAL\tFormat 4\tCOUNT [%llu]\n", vb->cpu.step_count);
            JAL(vb, gen_format4(vb, opcode));
            break;

        case 0x06:
            VB_log("[CPU] JMP\tFormat 1\tCOUNT [%llu]\n", vb->cpu.step_count);
            JMP(vb, gen_format1(vb, opcode));
            break;

        case 0x2A:
            VB_log("[CPU] JR\tFormat 4\tCOUNT [%llu]\n", vb->cpu.step_count);
            JR(vb, gen_format4(vb, opcode));
            break;

    // [CPU Control]
        case 0x1A:
            VB_log("[CPU] HALT\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x1C:
            VB_log("[CPU] LDSR\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            printf("count %llu\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x19:
            VB_log("[CPU] RETI\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x1D:
            VB_log("[CPU] STSR\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x18:
            VB_log("[CPU] TRAP\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Floating-Point] - [Nintendo - Extended]
        case 0x3E:
            sub_execute_float(vb, opcode);
            break;

    // [Bit Strings]
        case 0x1F:
            sub_execute_bit_strings(vb, opcode);
            break;

    // [CAXI]
        case 0x3A:
            VB_log("[CPU] CAXI\tFormat 4\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [SETF]
        case 0x12:
            VB_log("[CPU] SETF\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

    // [Nintendo - Standalone]
        case 0x16:
            VB_log("[CPU] CLI\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        case 0x1E:
            VB_log("[CPU] SEI\tFormat 2\tCOUNT [%llu]\n", vb->cpu.step_count);
            assert(0 && "instruction not implemented!");
            break;

        default:
            assert(0 && "UNK instruction!");
            break;
    }
}


void VB_cpu_run(struct VB_Core* vb) {
    execute_16(vb);
}
