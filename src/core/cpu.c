#include "core/vb.h"
#include "core/internal.h"


#define REG_PC vb->cpu.PC


static void sub_execute_float(struct VB_Core* vb, uint16_t opa) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Floating-Point]
        case 0b000100:
            VB_log("[CPU] ADDF.S - Format 7\n");
            break;

        case 0b000000:
            VB_log("[CPU] CMPF.S - Format 7\n");
            break;

        case 0b000011:
            VB_log("[CPU] CVT.SW - Format 7\n");
            break;

        case 0b000010:
            VB_log("[CPU] CVT.WS - Format 7\n");
            break;

        case 0b000111:
            VB_log("[CPU] DIVF.S - Format 7\n");
            break;

        case 0b000110:
            VB_log("[CPU] MULF.S - Format 7\n");
            break;

        case 0b000101:
            VB_log("[CPU] SUBF.S - Format 7\n");
            break;

        case 0b001011:
            VB_log("[CPU] TRNC.S - Format 7\n");
            break;

    // [Nintendo Extended]
        case 0b001100:
            VB_log("[CPU] MPYHW - Format 7\n");
            break;

        case 0b001010:
            VB_log("[CPU] REV - Format 7\n");
            break;

        case 0b001000:
            VB_log("[CPU] XB - Format 7\n");
            break;

        case 0b001001:
            VB_log("[CPU] XH - Format 7\n");
            break;
    }
}

static void sub_execute_bit_strings(struct VB_Core* vb, uint16_t opa) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 11) & 0x1F) {
    // [Bitwise]
        case 0b01001:
            VB_log("[CPU] ANDBSU - Format 2\n");
            break;

        case 0b01101:
            VB_log("[CPU] ANDNBSU - Format 2\n");
            break;

        case 0b01011:
            VB_log("[CPU] MOVBSU - Format 2\n");
            break;

        case 0b01111:
            VB_log("[CPU] NOTBSU - Format 2\n");
            break;

        case 0b01000:
            VB_log("[CPU] ORBSU - Format 2\n");
            break;

        case 0b01100:
            VB_log("[CPU] ORNBSU - Format 2\n");
            break;

        case 0b01010:
            VB_log("[CPU] XORBSU - Format 2\n");
            break;

        case 0b01110:
            VB_log("[CPU] XORNBSU - Format 2\n");
            break;

    // [Search]
        case 0b00001:
            VB_log("[CPU] SCH0BSD - Format 2\n");
            break;

        case 0b00000:
            VB_log("[CPU] SCH0BSU - Format 2\n");
            break;

        case 0b00011:
            VB_log("[CPU] SCH1BSD - Format 2\n");
            break;

        case 0b00010:
            VB_log("[CPU] SCH1BSU - Format 2\n");
            break;
    }
}

static void execute_16(struct VB_Core* vb) {
    uint16_t opcode = VB_bus_read_16(vb, REG_PC);
    REG_PC += 2;

    switch ((opcode >> 10) & 0x3F) {
    // [Register Transfer]
        case 0b010000:
            VB_log("[CPU] MOV - Format 2\n");
            break;

        case 0b000000:
            VB_log("[CPU] MOV - Format 1\n");
            break;

        case 0b101000:
            VB_log("[CPU] MOVEA - Format 5\n");
            break;

        case 0b101111:
            VB_log("[CPU] MOVHI - Format 5\n");
            break;

    // [Load and Input]
        case 0b111000:
            VB_log("[CPU] IN.B - Format 6\n");
            break;

        case 0b111001:
            VB_log("[CPU] IN.H - Format 6\n");
            break;

        case 0b111011:
            VB_log("[CPU] IN.W - Format 6\n");
            break;

        case 0b110000:
            VB_log("[CPU] LD.B - Format 6\n");
            break;

        case 0b110001:
            VB_log("[CPU] LD.H - Format 6\n");
            break;

        case 0b110011:
            VB_log("[CPU] LD.W - Format 6\n");
            break;

    // [Store and Output]
        case 0b111100:
            VB_log("[CPU] OUT.B - Format 6\n");
            break;

        case 0b111101:
            VB_log("[CPU] OUT.H - Format 6\n");
            break;

        case 0b111111:
            VB_log("[CPU] OUT.W - Format 6\n");
            break;

        case 0b110100:
            VB_log("[CPU] ST.B - Format 6\n");
            break;

        case 0b110101:
            VB_log("[CPU] ST.H - Format 6\n");
            break;

        case 0b110111:
            VB_log("[CPU] ST.W - Format 6\n");
            break;

    // [Arithmetic]
        case 0b010001:
            VB_log("[CPU] ADD - Format 2\n");
            break;

        case 0b000001:
            VB_log("[CPU] ADD - Format 1\n");
            break;

        case 0b101001:
            VB_log("[CPU] ADDI - Format 5\n");
            break;

        case 0b010011:
            VB_log("[CPU] CMP - Format 1\n");
            break;

        case 0b000011:
            VB_log("[CPU] CMP - Format 1\n");
            break;

        case 0b001001:
            VB_log("[CPU] DIV - Format 1\n");
            break;

        case 0b001011:
            VB_log("[CPU] DIVU - Format 1\n");
            break;

        case 0b001000:
            VB_log("[CPU] MUL - Format 1\n");
            break;

        case 0b001010:
            VB_log("[CPU] MULU - Format 1\n");
            break;

        case 0b000010:
            VB_log("[CPU] SUB - Format 1\n");
            break;

    // [Bitwise]
        case 0b001101:
            VB_log("[CPU] AND - Format 1\n");
            break;

        case 0b101101:
            VB_log("[CPU] ANDI - Format 5\n");
            break;

        case 0b001111:
            VB_log("[CPU] NOT - Format 1\n");
            break;

        case 0b001100:
            VB_log("[CPU] OR - Format 1\n");
            break;

        case 0b101100:
            VB_log("[CPU] ORI - Format 5\n");
            break;

        case 0b010111:
            VB_log("[CPU] SAR - Format 2\n");
            break;

        case 0b000111:
            VB_log("[CPU] SAR - Format 1\n");
            break;

        case 0b010100:
            VB_log("[CPU] SHL - Format 2\n");
            break;

        case 0b000100:
            VB_log("[CPU] SHL - Format 1\n");
            break;

        case 0b010101:
            VB_log("[CPU] SHR - Format 2\n");
            break;

        case 0b000101:
            VB_log("[CPU] SHR - Format 1\n");
            break;

        case 0b001110:
            VB_log("[CPU] XOR - Format 1\n");
            break;

        case 0b101110:
            VB_log("[CPU] XORI - Format 5\n");
            break;

    // [Conditional Branch]
        case 0b100000:
        case 0b100001:
        case 0b100010:
        case 0b100011:
        case 0b100100:
        case 0b100101:
        case 0b100110:
        case 0b100111:
            VB_log("[CPU] COND-BRANCH - Format 3\n");
            break;

    // [JUMP]
        case 0b101011:
            VB_log("[CPU] JAL - Format 4\n");
            break;

        case 0b000110:
            VB_log("[CPU] JMP - Format 1\n");
            break;

        case 0b101010:
            VB_log("[CPU] JR - Format 4\n");
            break;

    // [CPU Control]
        case 0b011010:
            VB_log("[CPU] HALT - Format 2\n");
            break;

        case 0b011100:
            VB_log("[CPU] LDSR - Format 2\n");
            break;

        case 0b011001:
            VB_log("[CPU] RETI - Format 2\n");
            break;

        case 0b011101:
            VB_log("[CPU] STSR - Format 2\n");
            break;

        case 0b011000:
            VB_log("[CPU] TRAP - Format 2\n");
            break;

    // [Floating-Point] - [Nintendo - Extended]
        case 0b111110:
            sub_execute_float(vb, opcode);
            break;

    // [Bit Strings]
        case 0b011111:
            sub_execute_bit_strings(vb, opcode);
            break;

    // [CAXI]
        case 0b111010:
            VB_log("[CPU] CAXI - Format 4\n");
            break;

    // [SETF]
        case 0b010010:
            VB_log("[CPU] SETF - Format 2\n");
            break;

    // [Nintendo - Standalone]
        case 0b010110:
            VB_log("[CPU] CLI - Format 2\n");
            break;

        case 0b011110:
            VB_log("[CPU] SEI - Format 2\n");
            break;

        default:
            break;
    }
}
