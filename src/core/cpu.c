#include "core/vb.h"
#include "core/internal.h"


#define REG_PC vb->cpu.PC


static void sub_execute_float(struct VB_Core* vb, uint16_t opa) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Floating-Point]
        case 0x04:
            VB_log("[CPU] ADDF.S - Format 7\n");
            break;

        case 0x00:
            VB_log("[CPU] CMPF.S - Format 7\n");
            break;

        case 0x03:
            VB_log("[CPU] CVT.SW - Format 7\n");
            break;

        case 0x02:
            VB_log("[CPU] CVT.WS - Format 7\n");
            break;

        case 0x07:
            VB_log("[CPU] DIVF.S - Format 7\n");
            break;

        case 0x06:
            VB_log("[CPU] MULF.S - Format 7\n");
            break;

        case 0x05:
            VB_log("[CPU] SUBF.S - Format 7\n");
            break;

        case 0x0B:
            VB_log("[CPU] TRNC.S - Format 7\n");
            break;

    // [Nintendo Extended]
        case 0x0C:
            VB_log("[CPU] MPYHW - Format 7\n");
            break;

        case 0x0A:
            VB_log("[CPU] REV - Format 7\n");
            break;

        case 0x08:
            VB_log("[CPU] XB - Format 7\n");
            break;

        case 0x09:
            VB_log("[CPU] XH - Format 7\n");
            break;
    }
}

static void sub_execute_bit_strings(struct VB_Core* vb, uint16_t opa) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 11) & 0x1F) {
    // [Bitwise]
        case 0x09:
            VB_log("[CPU] ANDBSU - Format 2\n");
            break;

        case 0x0D:
            VB_log("[CPU] ANDNBSU - Format 2\n");
            break;

        case 0x0B:
            VB_log("[CPU] MOVBSU - Format 2\n");
            break;

        case 0x1F:
            VB_log("[CPU] NOTBSU - Format 2\n");
            break;

        case 0x08:
            VB_log("[CPU] ORBSU - Format 2\n");
            break;

        case 0x0C:
            VB_log("[CPU] ORNBSU - Format 2\n");
            break;

        case 0x0A:
            VB_log("[CPU] XORBSU - Format 2\n");
            break;

        case 0x0E:
            VB_log("[CPU] XORNBSU - Format 2\n");
            break;

    // [Search]
        case 0x01:
            VB_log("[CPU] SCH0BSD - Format 2\n");
            break;

        case 0x00:
            VB_log("[CPU] SCH0BSU - Format 2\n");
            break;

        case 0x03:
            VB_log("[CPU] SCH1BSD - Format 2\n");
            break;

        case 0x02:
            VB_log("[CPU] SCH1BSU - Format 2\n");
            break;
    }
}

static void execute_16(struct VB_Core* vb) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Register Transfer]
        case 0x10:
            VB_log("[CPU] MOV - Format 2\n");
            break;

        case 0x00:
            VB_log("[CPU] MOV - Format 1\n");
            break;

        case 0x28:
            VB_log("[CPU] MOVEA - Format 5\n");
            break;

        case 0x2F:
            VB_log("[CPU] MOVHI - Format 5\n");
            break;

    // [Load and Input]
        case 0x38:
            VB_log("[CPU] IN.B - Format 6\n");
            break;

        case 0x39:
            VB_log("[CPU] IN.H - Format 6\n");
            break;

        case 0x3B:
            VB_log("[CPU] IN.W - Format 6\n");
            break;

        case 0x30:
            VB_log("[CPU] LD.B - Format 6\n");
            break;

        case 0x31:
            VB_log("[CPU] LD.H - Format 6\n");
            break;

        case 0x33:
            VB_log("[CPU] LD.W - Format 6\n");
            break;

    // [Store and Output]
        case 0x3C:
            VB_log("[CPU] OUT.B - Format 6\n");
            break;

        case 0x3D:
            VB_log("[CPU] OUT.H - Format 6\n");
            break;

        case 0x3F:
            VB_log("[CPU] OUT.W - Format 6\n");
            break;

        case 0x34:
            VB_log("[CPU] ST.B - Format 6\n");
            break;

        case 0x35:
            VB_log("[CPU] ST.H - Format 6\n");
            break;

        case 0x37:
            VB_log("[CPU] ST.W - Format 6\n");
            break;

    // [Arithmetic]
        case 0x11:
            VB_log("[CPU] ADD - Format 2\n");
            break;

        case 0x01:
            VB_log("[CPU] ADD - Format 1\n");
            break;

        case 0x29:
            VB_log("[CPU] ADDI - Format 5\n");
            break;

        case 0x13:
            VB_log("[CPU] CMP - Format 1\n");
            break;

        case 0x03:
            VB_log("[CPU] CMP - Format 1\n");
            break;

        case 0x09:
            VB_log("[CPU] DIV - Format 1\n");
            break;

        case 0x0B:
            VB_log("[CPU] DIVU - Format 1\n");
            break;

        case 0x08:
            VB_log("[CPU] MUL - Format 1\n");
            break;

        case 0x0A:
            VB_log("[CPU] MULU - Format 1\n");
            break;

        case 0x02:
            VB_log("[CPU] SUB - Format 1\n");
            break;

    // [Bitwise]
        case 0x0D:
            VB_log("[CPU] AND - Format 1\n");
            break;

        case 0x2D:
            VB_log("[CPU] ANDI - Format 5\n");
            break;

        case 0x0F:
            VB_log("[CPU] NOT - Format 1\n");
            break;

        case 0x0C:
            VB_log("[CPU] OR - Format 1\n");
            break;

        case 0x2C:
            VB_log("[CPU] ORI - Format 5\n");
            break;

        case 0x17:
            VB_log("[CPU] SAR - Format 2\n");
            break;

        case 0x07:
            VB_log("[CPU] SAR - Format 1\n");
            break;

        case 0x14:
            VB_log("[CPU] SHL - Format 2\n");
            break;

        case 0x04:
            VB_log("[CPU] SHL - Format 1\n");
            break;

        case 0x15:
            VB_log("[CPU] SHR - Format 2\n");
            break;

        case 0x05:
            VB_log("[CPU] SHR - Format 1\n");
            break;

        case 0x0E:
            VB_log("[CPU] XOR - Format 1\n");
            break;

        case 0x2E:
            VB_log("[CPU] XORI - Format 5\n");
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
            break;

    // [JUMP]
        case 0x2B:
            VB_log("[CPU] JAL - Format 4\n");
            break;

        case 0x06:
            VB_log("[CPU] JMP - Format 1\n");
            break;

        case 0x2A:
            VB_log("[CPU] JR - Format 4\n");
            break;

    // [CPU Control]
        case 0x1A:
            VB_log("[CPU] HALT - Format 2\n");
            break;

        case 0x1C:
            VB_log("[CPU] LDSR - Format 2\n");
            break;

        case 0x19:
            VB_log("[CPU] RETI - Format 2\n");
            break;

        case 0x1D:
            VB_log("[CPU] STSR - Format 2\n");
            break;

        case 0x18:
            VB_log("[CPU] TRAP - Format 2\n");
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
            break;

    // [SETF]
        case 0x12:
            VB_log("[CPU] SETF - Format 2\n");
            break;

    // [Nintendo - Standalone]
        case 0x16:
            VB_log("[CPU] CLI - Format 2\n");
            break;

        case 0x1E:
            VB_log("[CPU] SEI - Format 2\n");
            break;

        default:
            break;
    }
}
