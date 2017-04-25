/*
 * Copyright(C) 2017 Imagination Technologies Limited and/or its
 *              affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include <stddef.h>
#include "board.h"
#include "periph/flashpage.h"

#define VIRT_TO_PHY_ADDR(ADDRESS)   (((unsigned int)ADDRESS) & 0x1FFFFFFF)
#define ROW_SIZE                (2048)
#define ROW_PER_PAGE            (8)
#define ROW_PROGRAM_OP          (0b0011)
#define ERASE_PAGE_OP           (0b0100)

static void nvm_init(void)
{
    int int_status;

    asm volatile("di %0" : "=r"(int_status));

    NVMKEY = 0x0;
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;
    NVMCON |= _NVMCON_WR_MASK;

    /* Restore Interrupts */
    if(int_status & 0x00000001)
        asm volatile("ei");
}

void flashpage_write(int page, void *data)
{
    const uint32_t page_base_addr = page * FLASHPAGE_SIZE;

    if (data == NULL) {
        NVMADDR = VIRT_TO_PHY_ADDR(page_base_addr);
        NVMCON |= ERASE_PAGE_OP << _NVMCON_NVMOP_POSITION;
        NVMCON |= _NVMCON_WREN_MASK;
        nvm_init();
        while(NVMCON & _NVMCON_WR_MASK) {}
        NVMCON &= ~_NVMCON_WREN_MASK;
    }
    else {
        uint32_t offset = 0;
        uint8_t r;
        for (r = 0; r < FLASHPAGE_SIZE / ROW_SIZE; ++r) {
            NVMADDR = VIRT_TO_PHY_ADDR(page_base_addr + offset);
            NVMSRCADDR = VIRT_TO_PHY_ADDR(data);
            data += ROW_SIZE;
            offset += ROW_SIZE;

            NVMCON |= ROW_PROGRAM_OP << _NVMCON_NVMOP_POSITION;
            NVMCON |= _NVMCON_WREN_MASK;

            while(NVMCON & _NVMCON_WR_MASK) {}

            NVMCON &= ~_NVMCON_WREN_MASK;

            /* Check if an error occured */
            if(NVMCON & (_NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK))
                return;
        }
    }
}

void flashpage_read(int page, void *data)
{
    const uint32_t page_base_addr = page * FLASHPAGE_SIZE;
    uint32_t *buffer = (uint32_t*)data;
    uint32_t offset = 0;
    while (offset < FLASHPAGE_SIZE) {
        *buffer++ = *(uint32_t*)(page_base_addr + offset);
        offset += 4;
    }
}

int flashpage_verify(int page, void *data)
{
    const uint32_t page_base_addr = page * FLASHPAGE_SIZE;
    uint32_t *buffer = (uint32_t*)data;
    uint32_t offset = 0;
    while (offset < FLASHPAGE_SIZE) {
        uint32_t tmp = *(uint32_t*)(page_base_addr + offset);
        if (tmp != *buffer)
            return FLASHPAGE_NOMATCH;

        offset += 4;
        buffer++;
    }

    return FLASHPAGE_OK;
}

int flashpage_write_and_verify(int page, void *data)
{
    flashpage_write(page, data);
    return flashpage_verify(page, data);
}
