#include "vb.h"
#include "internal.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>


void vb_reset_cpu(struct VB_Cpu* cpu) {
    cpu->PC = 0xFFFFFFF0;
    cpu->ECR.FECC = 0x0000;
    cpu->ECR.EICC = 0xFFF0;
    cpu->PSW.NP = 1;
    cpu->PIR = 0x00005346;
    cpu->registers[ZERO_REGISTER] = 0x00000000;
}

void vb_init(struct VB_Core* vb) {
    assert(vb);

    memset(vb, 0, sizeof(struct VB_Core));

    vb_reset_cpu(&vb->cpu);
}

static inline bool is_pow2(size_t size) {
    return (!(size & (size - 1)) && size);
}

const struct VB_RomHeader* vb_get_rom_header(const struct VB_Core* vb) {
    return vb_get_rom_header_from_data(vb->rom, vb->rom_size);
}

const struct VB_RomHeader* vb_get_rom_header_from_data(
    const uint8_t* data, const size_t size
) {
    // header should always start at the end of the rom area
    return (const struct VB_RomHeader*)(data + (size - 544));
}

void vb_get_rom_title(
    const struct VB_Core* vb, struct VB_RomTitle* title
) {
    vb_get_rom_title_from_header(vb_get_rom_header(vb), title);
}

void vb_get_rom_title_from_header(
    const struct VB_RomHeader* header, struct VB_RomTitle* title
) {
    assert(header && title);

    memset(title, 0, sizeof(struct VB_RomTitle));

    for (size_t i = 0; i < VB_ARR_SIZE(header->title); ++i) {
        title->title[i] = header->title[i];
    }
}

static void log_header(const struct VB_RomHeader* header) {
    assert(header);

    struct VB_RomTitle title;
    vb_get_rom_title_from_header(header, &title);

    vb_log("\nHEADER LOG\n");
    vb_log("\tTITLE: %s\n", title.title);
    vb_log("\n");
}

bool vb_loadrom(
    struct VB_Core* vb, const uint8_t* data, size_t size
) {
    assert(vb && data && size);

    assert(is_pow2(size));

    const struct VB_RomHeader* header = vb_get_rom_header_from_data(data, size);
    log_header(header);

    vb->rom = data;
    vb->rom_size = size;
    vb->rom_mask = size - 1;

    return true;
}

void vb_step(struct VB_Core* vb) {
    vb_cpu_run(vb);
}
