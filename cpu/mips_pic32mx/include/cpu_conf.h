/*
 * Copyright(C) 2016,2017, Imagination Technologies Limited and/or its
 *                 affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @defgroup    cpu_mips_pic32mx PIC32MX
 * @ingroup     cpu
 * @{
 *
 * @file
 * @brief       CPU definitions for Microchip PIC32MX devices.
 *
 * @author      Neil Jones <neil.jones@imgtec.com>
 */

#ifndef CPU_CONF_H
#define CPU_CONF_H

#ifdef CPU_MODEL_P32MX470F512H
#include "vendor/p32mx470f512h.h"
#else
#error "No CPU headers for the defined CPU_MODEL found"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief    Configuration of default stack sizes
 *
 * printf takes a pretty tortured route through the C lib
 * then via UHI syscall exception to end up at the UART
 * driver.
 *
 * When debugging timer code we get printfs on the idle threads
 * stack which can easily blow its limits.
 *
 * Note code must be compiled at -Os with these values, using -O0
 * you'll overflow these stacks.
 *
 * NO ISR stack is in use yet, interrupt use the current running stack
 * hence the big-ish default stack size.
 * @{
 */
#ifndef THREAD_EXTRA_STACKSIZE_PRINTF
#define THREAD_EXTRA_STACKSIZE_PRINTF   (1024)
#endif

#ifndef THREAD_STACKSIZE_DEFAULT
#define THREAD_STACKSIZE_DEFAULT        (2048)
#endif

#ifndef THREAD_STACKSIZE_IDLE
#ifdef NDEBUG
#define THREAD_STACKSIZE_IDLE           (512)
#else
#define THREAD_STACKSIZE_IDLE           (512 + THREAD_EXTRA_STACKSIZE_PRINTF)
#endif
#endif
/** @} */

/**
 * @brief   Flash page configuration
 * @{
 */
#define FLASHPAGE_SIZE          (4096U)

#if defined(CPU_MODEL_P32MX470F512H)
#define FLASHPAGE_NUMOF         (128U)
#endif

/* The minimum block size which can be written is 4B. However, the erase
 * block is always FLASHPAGE_SIZE.
 */
#define FLASHPAGE_RAW_BLOCKSIZE    (4U)
/* Writing should be always 4 bytes aligned */
#define FLASHPAGE_RAW_ALIGNMENT    (4U)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* CPU_CONF_H */
/** @} */
