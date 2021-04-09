#include "core/vb.h"
#include "core/internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static struct VB_Core CORE = {0};

static uint8_t ROM_DATA[VB_MAX_ROM_SIZE];


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

#define STEP_COUNT 8

    for (CORE.cpu.step_count = 0; CORE.cpu.step_count < STEP_COUNT; ++CORE.cpu.step_count) {
        VB_step(&CORE);
    }

    return 0;
}
