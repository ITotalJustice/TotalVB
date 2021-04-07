#include "core/vb.h"
#include "core/internal.h"

#include <assert.h>


/*
0x00000000 - 0x00FFFFFF 	VIP - Virtual Image Processor
0x01000000 - 0x01FFFFFF 	VSU - Virtual Sound Unit
0x02000000 - 0x02FFFFFF 	Miscellaneous Hardware
0x03000000 - 0x03FFFFFF 	Unmapped
0x04000000 - 0x04FFFFFF 	Game Pak Expansion
0x05000000 - 0x05FFFFFF 	WRAM
0x06000000 - 0x06FFFFFF 	Game Pak RAM
0x07000000 - 0x07FFFFFF 	Game Pak ROM
0x08000000 - 0xFFFFFFFF 	Mirroring of memory map
*/


void VB_mmio_write_32(struct VB_Core* vb, uint32_t addr, uint32_t value) {
    switch ((addr >> 24) & 0x7) {
        case 0x0: /* VIP - Virtual Image Processor */
            VB_log("[MMIO-W32] VIP - Virtual Image Processor: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x1: /* VSU - Virtual Sound Unit */
            VB_log("[MMIO-W32] VSU - Virtual Sound Unit: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x2: /* Miscellaneous Hardware */
            VB_log("[MMIO-W32] Miscellaneous Hardware: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x3: /* Unmapped */
            VB_log("[MMIO-W32] Unmapped: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x4: /* Game Pak Expansion */
            VB_log("[MMIO-W32] Game Pak Expansion: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x5: /* WRAM */
            VB_log("[MMIO-W32] WRAM: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x6: /* Game Pak RAM */
            VB_log("[MMIO-W32] Game Pak RAM: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;

        case 0x7: /* Game Pak ROM */
            VB_log("[MMIO-W32] Game Pak ROM: addr: 0x%08X value: 0x%08X\n", addr, value);
            break;
    }
}
