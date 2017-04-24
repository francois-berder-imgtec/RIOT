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
    uint8_t wait_state_count = 0;

    if (SYSTEM_CLOCK >= FLASH_CLOCK_SPEED)
        wait_state_count = (SYSTEM_CLOCK / FLASH_CLOCK_SPEED) - 1;
    PRECON = (wait_state_count & _PRECON_PFMWS_MASK);

    /*
     * Enable predictive prefetch for CPU instructions only.
     *
     * If the system is too slow, enabling predictive prefech cache
     * is detrimental to performance according to document 60001183B,
     * section 41.6
     */
    if (wait_state_count != 0)
        PRECON |= 1 << _PRECON_PREFEN_POSITION;
}

#endif /* _PCACHE */