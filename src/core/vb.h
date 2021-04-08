#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "core/types.h"


/* 64 MiB */
#define VB_MAX_ROM_SIZE 0x4000000


void VB_init(struct VB_Core* vb);

void VB_step(struct VB_Core* vb);

// struct VB_RomHeader
bool VB_loadrom(struct VB_Core* vb, const uint8_t* data, size_t size);

struct VB_RomHeader* VB_get_rom_header(const struct VB_Core* vb);
struct VB_RomHeader* VB_get_rom_header_from_data(const uint8_t* data, const size_t size);

void VB_get_rom_title(const struct VB_Core* vb, struct VB_RomTitle* title);
void VB_get_rom_title_from_header(const struct VB_RomHeader* header, struct VB_RomTitle* title);

#ifdef __cplusplus
}
#endif
