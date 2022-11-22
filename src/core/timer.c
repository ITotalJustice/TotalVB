/**
 * Copyright 2022 TotalJustice.
 * SPDX-License-Identifier: MIT
 */

#include "vb.h"
#include "internal.h"
#include "bit.h"

// #include <stdio.h>
#include <assert.h>
#include <string.h>


void vb_timer_run(struct VB_Core* vb, uint8_t cycles) {

}

void vb_timer_reset(struct VB_Core* vb) {
  memset(&vb->timer, 0, sizeof(vb->timer));
}
