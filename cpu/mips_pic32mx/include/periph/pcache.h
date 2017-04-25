/*
 * Copyright(C) 2017, Imagination Technologies Limited and/or its
 *                 affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#ifndef PREFETCH_H_
#define PREFETCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PIC32MX has a small prefetch cache
 *
 * This function configures the prefetch cache and the program
 * flash memory access time.
 */
void pcache_init(void);

#ifdef __cplusplus
}
#endif

#endif