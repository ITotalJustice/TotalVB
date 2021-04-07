#include "core/vb.h"
#include "core/internal.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


static void reset_cpu(struct VB_Cpu* cpu) {
    cpu->PC = 0xFFFFFFF0;
    cpu->ECR.FECC = 0x0000;
    cpu->ECR.EICC = 0xFFF0;
    // cpu->PSW = 0x00008000;
    cpu->PIR = 0x00005346;
    cpu->registers[ZERO_REGISTER] = 0x00000000;
}

void VB_init(struct VB_Core* vb) {
    assert(vb);

    memset(vb, 0, sizeof(struct VB_Core));

    reset_cpu(&vb->cpu);
}

struct VB_RomHeader* VB_get_rom_header(const struct VB_Core* vb) {
    return VB_get_rom_header_from_data(vb->rom, vb->rom_size);
}

struct VB_RomHeader* VB_get_rom_header_from_data(const uint8_t* data, const size_t size) {
    // assert(size > VB_ROM_HEADER_OFFSET && "data to small!");

    if (size <= (0x100000 / 2)) {
        return (struct VB_RomHeader*)(data + VB_ROM_HEADER_OFFSET_512kb);
    }

    if (size <= (0x100000)) {
        return (struct VB_RomHeader*)(data + VB_ROM_HEADER_OFFSET_1mb);
    }

    if (size <= 0x200000) {
        return (struct VB_RomHeader*)(data + VB_ROM_HEADER_OFFSET_2mb);
    }

    return NULL;
}

void VB_get_rom_title(const struct VB_Core* vb, struct VB_RomTitle* title) {
    VB_get_rom_title_from_header(VB_get_rom_header(vb), title);
}

void VB_get_rom_title_from_header(const struct VB_RomHeader* header, struct VB_RomTitle* title) {
    assert(header && title);

    memset(title, 0, sizeof(struct VB_RomTitle));

    for (size_t i = 0; i < VB_ARR_SIZE(header->title); ++i) {
        title->title[i] = header->title[i];
    }
}

static void log_header(const struct VB_RomHeader* header) {
    assert(header);

    struct VB_RomTitle title;
    VB_get_rom_title_from_header(header, &title);

    VB_log("\nHEADER LOG\n");
    VB_log("\tTITLE: %s\n", title.title);
    VB_log("\n");
}

bool VB_loadrom(struct VB_Core* vb, const uint8_t* data, size_t size) {
    assert(vb && data && size);

    const struct VB_RomHeader* header = VB_get_rom_header_from_data(data, size);
    log_header(header);

    vb->rom = data;
    vb->rom_size = size;

    return true;
}
