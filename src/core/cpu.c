#include "core/vb.h"
#include "core/internal.h"


#define CPU vb->cpu

#define REGISTERS CPU.registers
#define REG_PC CPU.PC


// helpers that half / word align the address when reading
#define READ8(addr) VB_bus_read_8(vb, (addr))
#define READ16(addr) VB_bus_read_16(vb, (addr) & ~(0x1))
#define READ32(addr) VB_bus_read_32(vb, (addr) & ~(0x3))


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

static inline struct Format1 gen_format1(struct VB_Core* vb, uint16_t op) {
    const struct Format1 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .reg1 = get_bit_range(0, 4, op),
    };

    return f;
}

static inline struct Format2 gen_format2(struct VB_Core* vb, uint16_t op) {
    const struct Format2 f = {
        .opcode = get_bit_range(10, 15, op),
        .reg2 = get_bit_range(5, 9, op),
        .lo5 = get_bit_range(0, 4, op),
    };

    return f;
}

static inline struct Format3 gen_format3(struct VB_Core* vb, uint16_t op) {
    const uint32_t disp_range = get_bit_range(0, 8, op);

    const struct Format3 f = {
        .opcode = get_bit_range(13, 15, op),
        .cond = get_bit_range(9, 12, op),
        .disp = sign_extend(8, disp_range),
    };

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

// [Jump]
static inline void JMP(struct VB_Core* vb, struct Format1 f) {
    REG_PC = REGISTERS[f.reg1];
    REG_PC = align_16(REG_PC);
}

static inline void JR(struct VB_Core* vb, struct Format4 f) {
    VB_log("PC: 0x%08X DISP: %d R: 0x%08X\n", REG_PC, f.disp, REG_PC + f.disp);
    REG_PC += f.disp;
    REG_PC = align_16(REG_PC);
}


static void sub_execute_float(struct VB_Core* vb, uint16_t opa) {
    const uint16_t opcode = READ16(REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Floating-Point]
        case 0x04:
            VB_log("[CPU] ADDF.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x00:
            VB_log("[CPU] CMPF.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x03:
            VB_log("[CPU] CVT.SW - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] CVT.WS - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x07:
            VB_log("[CPU] DIVF.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x06:
            VB_log("[CPU] MULF.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x05:
            VB_log("[CPU] SUBF.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] TRNC.S - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Nintendo Extended]
        case 0x0C:
            VB_log("[CPU] MPYHW - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] REV - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] XB - Format 7\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x09:
            VB_log("[CPU] XH - Format 7\n");
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
            VB_log("[CPU] ANDBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0D:
            VB_log("[CPU] ANDNBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] MOVBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x1F:
            VB_log("[CPU] NOTBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] ORBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0C:
            VB_log("[CPU] ORNBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] XORBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0E:
            VB_log("[CPU] XORNBSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Search]
        case 0x01:
            VB_log("[CPU] SCH0BSD - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x00:
            VB_log("[CPU] SCH0BSU - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x03:
            VB_log("[CPU] SCH1BSD - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] SCH1BSU - Format 2\n");
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
            VB_log("[CPU] MOV - Format 2\n");
            MOV_imm(vb, gen_format2(vb, opcode));
            break;

        case 0x00:
            VB_log("[CPU] MOV - Format 1\n");
            MOV_reg(vb, gen_format1(vb, opcode));
            break;

        case 0x28:
            VB_log("[CPU] MOVEA - Format 5\n");
            MOVEA(vb, gen_format5(vb, opcode));
            break;

        case 0x2F:
            VB_log("[CPU] MOVHI - Format 5\n");
            MOVHI(vb, gen_format5(vb, opcode));
            break;

    // [Load and Input]
        case 0x38:
            VB_log("[CPU] IN.B - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x39:
            VB_log("[CPU] IN.H - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x3B:
            VB_log("[CPU] IN.W - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x30:
            VB_log("[CPU] LD.B - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x31:
            VB_log("[CPU] LD.H - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x33:
            VB_log("[CPU] LD.W - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Store and Output]
        case 0x3C:
            VB_log("[CPU] OUT.B - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x3D:
            VB_log("[CPU] OUT.H - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x3F:
            VB_log("[CPU] OUT.W - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x34:
            VB_log("[CPU] ST.B - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x35:
            VB_log("[CPU] ST.H - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x37:
            VB_log("[CPU] ST.W - Format 6\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Arithmetic]
        case 0x11:
            VB_log("[CPU] ADD - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x01:
            VB_log("[CPU] ADD - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x29:
            VB_log("[CPU] ADDI - Format 5\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x13:
            VB_log("[CPU] CMP - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x03:
            VB_log("[CPU] CMP - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x09:
            VB_log("[CPU] DIV - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0B:
            VB_log("[CPU] DIVU - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x08:
            VB_log("[CPU] MUL - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0A:
            VB_log("[CPU] MULU - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x02:
            VB_log("[CPU] SUB - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Bitwise]
        case 0x0D:
            VB_log("[CPU] AND - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x2D:
            VB_log("[CPU] ANDI - Format 5\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0F:
            VB_log("[CPU] NOT - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0C:
            VB_log("[CPU] OR - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x2C:
            VB_log("[CPU] ORI - Format 5\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x17:
            VB_log("[CPU] SAR - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x07:
            VB_log("[CPU] SAR - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x14:
            VB_log("[CPU] SHL - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x04:
            VB_log("[CPU] SHL - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x15:
            VB_log("[CPU] SHR - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x05:
            VB_log("[CPU] SHR - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x0E:
            VB_log("[CPU] XOR - Format 1\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x2E:
            VB_log("[CPU] XORI - Format 5\n");
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
            VB_log("[CPU] COND-BRANCH - Format 3\n");
            assert(0 && "instruction not implemented!");
            break;

    // [JUMP]
        case 0x2B:
            VB_log("[CPU] JAL - Format 4\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x06:
            VB_log("[CPU] JMP - Format 1\n");
            JMP(vb, gen_format1(vb, opcode));
            break;

        case 0x2A:
            VB_log("[CPU] JR - Format 4\n");
            JR(vb, gen_format4(vb, opcode));
            break;

    // [CPU Control]
        case 0x1A:
            VB_log("[CPU] HALT - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x1C:
            VB_log("[CPU] LDSR - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x19:
            VB_log("[CPU] RETI - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x1D:
            VB_log("[CPU] STSR - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x18:
            VB_log("[CPU] TRAP - Format 2\n");
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
            VB_log("[CPU] CAXI - Format 4\n");
            assert(0 && "instruction not implemented!");
            break;

    // [SETF]
        case 0x12:
            VB_log("[CPU] SETF - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

    // [Nintendo - Standalone]
        case 0x16:
            VB_log("[CPU] CLI - Format 2\n");
            assert(0 && "instruction not implemented!");
            break;

        case 0x1E:
            VB_log("[CPU] SEI - Format 2\n");
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
