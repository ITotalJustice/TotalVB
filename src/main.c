#include "core/vb.h"
// for testing!
#include "core/internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct VB_Core CORE = {0};

// 4-MB (i think most games are 2MB)
static uint8_t ROM_DATA[0x400000];

static bool read_file(const char* path, uint8_t* out_buf, size_t* out_size) {
	FILE* f = fopen(path, "rb");
	if (!f) {
		return false;
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (size <= 0) {
		return false;
	}

	fread(out_buf, 1, size, f);
	*out_size = (size_t)size;
	fclose(f);

	return true;
}

static void mmio_test() {
    const uint32_t addrs[] = {
        0x00FFFFFF, // VIP - Virtual Image Processor
        0x01FFFFFF, // VSU - Virtual Sound Unit
        0x02FFFFFF, // Miscellaneous Hardware
        0x03FFFFFF, // Unmapped
        0x04FFFFFF, // Game Pak Expansion
        0x05FFFFFF, // WRAM
        0x06FFFFFF, // Game Pak RAM
        0x07FFFFFF, // Game Pak ROM

        0x08000000, // Mirroring of memory map
        0x09000000, // Mirroring of memory map
        0x0A000000, // Mirroring of memory map
        0x0B000000, // Mirroring of memory map
        0x0C000000, // Mirroring of memory map
        0x0D000000, // Mirroring of memory map
        0x0E000000, // Mirroring of memory map
        0x0F000000, // Mirroring of memory map
        0x10000000, // Mirroring of memory map
        0x11000000, // Mirroring of memory map
        0xFFFFFFFF, // Mirroring of memory map
    };

    for (size_t i = 0; i < VB_ARR_SIZE(addrs); ++i) {
        VB_mmio_write_32(&CORE, addrs[i], 0x00);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("missing args\n");
        return -1;
    }

    VB_init(&CORE);

    size_t rom_size = 0;

    if (!read_file(argv[1], ROM_DATA, &rom_size)) {
        printf("failed to read file!\n");
        return -1;
    }

    if (!VB_loadrom(&CORE, ROM_DATA, rom_size)) {
        printf("failed to load rom!\n");
        return -1;
    }

    // this passes
    // mmio_test();

    return 0;
}
