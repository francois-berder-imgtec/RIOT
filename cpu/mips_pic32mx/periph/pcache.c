/*
 * Copyright(C) 2017 Imagination Technologies Limited and/or its
 *              affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include "board.h"
#include "periph/pcache.h"

#ifdef _PCACHE

#define FLASH_CLOCK_SPEED     (30000000)        /* 30 MHz */

void pcache_init(void)
{
    uint8_t wait_state_count = (SYSTEM_CLOCK / FLASH_CLOCK_SPEED) - 1;
    CHECON = (wait_state_count & _CHECON_PFMWS_MASK);

    if(wait_state_count != 0) {
        CHECON |= (0b11 << _CHECON_PREFEN_POSITION);    /* Enable predictive prefetch for all regions */
        CHECON |= (0b01 << _CHECON_DCSZ_POSITION);      /* Data cache size of 1 line */
    }
}

#endif /* _PCACHE */