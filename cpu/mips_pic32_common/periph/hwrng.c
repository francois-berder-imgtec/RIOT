/*
 * Copyright(C) 2017 Imagination Technologies Limited and/or its
 *              affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include <string.h>
#include "board.h"
#include "log.h"
#include "periph/cpuid.h"
#include "periph/hwrng.h"

#ifdef _RNG

static void wait_plen_cycles(void)
{
    unsigned int i;
    for (i = 0; i < (RNGCON & _RNGCON_PLEN_MASK); ++i)
        __asm volatile ("nop");
}

void hwrng_init(void)
{
    uint32_t id;

    cpuid_get(&id);

    /*
     * TRNG is not working on PIC32MZ2048ECG100
     * This device is used in revision A and B of the Wi-Fire board.
     */
    if ((id & 0x0FFFFFFF) == 0x0510E053) {
        LOG_WARNING("hwrng: TRNG not working on PIC32MZ2048ECG100\n");
        LOG_WARNING("hwrng: Inializing PRNG with fixed seeds\n");
        RNGNUMGEN1 = 0x090A0B0C;
        RNGNUMGEN2 = 0x0D0E0F10;
    }
    else {
        RNGCON = _RNGCON_TRNGEN_MASK;

        /*
        * Wait to have at least 64 bits before setting the 64-bit seed
        * of the pseudo random generator.
        */
        while (RNGCNT < 64) {}

        /* Load seed from the TRNG */
        RNGCON |= _RNGCON_LOAD_MASK;
        while (RNGCON & _RNGCON_LOAD_MASK) {}

        RNGCON &= ~_RNGCON_TRNGEN_MASK;
    }

    RNGPOLY1 = 0x00C00003;
    RNGPOLY2 = 0x00000000;

    RNGCON |= 42;   /* Set PLEN to 42 */
    RNGCON |= _RNGCON_CONT_MASK;
}

void hwrng_read(void *buf, unsigned int num)
{
    unsigned int i = 0;

    RNGCON |= _RNGCON_PRNGEN_MASK;

    for (i = 0; i < num >> 3; ++i) {
        wait_plen_cycles();
        memcpy(buf, (uint32_t*)&RNGNUMGEN1, 4);
        memcpy(buf + 4, (uint32_t*)&RNGNUMGEN1, 4);
        buf += 8;
    }

    for (i = 0; i < (num & 7); ++i) {
        wait_plen_cycles();
        memcpy(buf, (uint32_t*)&RNGNUMGEN1, 1);
        ++buf;
    }

    RNGCON &= ~_RNGCON_PRNGEN_MASK;
}

#endif /* _RNG */
