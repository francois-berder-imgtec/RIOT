/*
 * Copyright (C) 2020 Francois Berder
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     cpu_mips_pic32_common
 * @ingroup     drivers_periph_wdt
 *
 * @brief
 *
 * @{
 *
 * @file        wdt.c
 * @brief       Independent Watchdog timer for PIC32 platforms
 *
 * @author      Francois Berder <fberder@outlook.fr>
 */

#include <assert.h>
#include <stdio.h>

#include "periph_cpu.h"
#include "periph/wdt.h"

#define WDTCLRKEY   (0x5743)

#ifdef __cplusplus
extern "C" {
#endif

void wdt_start(void)
{
    WDTCONSET = _WDTCON_ON_MASK;
}

void wdt_stop(void)
{
    WDTCONCLR = _WDTCON_ON_MASK;
}

void wdt_kick(void)
{
#if defined (CPU_FAM_PIC32MX)
    WDTCONSET = _WDTCON_WDTCLR_MASK;
#elif defined(CPU_FAM_PIC32MZ)
    *((volatile uint16_t*)&WDTCON + 1) = WDTCLRKEY;
#endif
}

void wdt_setup_reboot(uint32_t min_time, uint32_t max_time)
{
    (void) min_time;
    (void) max_time;

    printf("Warning: Ignoring wdt min_time/max_time. Prescaler set in configurations bits\r\n");
}

#ifdef __cplusplus
}
#endif
