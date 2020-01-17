/*
 * Copyright(C) 2020 Francois Berder <fberder@outlook.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include "assert.h"
#include "cpu.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

#include "periph/flashpage.h"

#define WORD_SIZE           (4)

#if defined(CPU_FAM_PIC32MX)
#define ROW_SIZE            (512)
#elif defined(CPU_FAM_PIC32MZ)
#define ROW_SIZE            (2048)
#endif

/* NVM op */
#define WORD_PROGRAM_OP     (0x1)
#define ROW_PROGRAM_OP      (0x3)
#define ERASE_PAGE_OP       (0x4)

#define NVM_UNLOCK_KEY_1    (0xAA996655)
#define NVM_UNLOCK_KEY_2    (0x556699AA)

static void nvm_perform_op(uint32_t op)
{
    uint32_t ctx;

    ctx = irq_disable();

    NVMCON = _NVMCON_WREN_MASK | (op & _NVMCON_NVMOP_MASK);

    NVMKEY = 0x0;
    NVMKEY = NVM_UNLOCK_KEY_1;
    NVMKEY = NVM_UNLOCK_KEY_2;
    NVMCONSET = _NVMCON_WR_MASK;
    while(NVMCON & _NVMCON_WR_MASK) {}

    irq_restore(ctx);

    NVMCONCLR = _NVMCON_WREN_MASK;
}

static void _erase_page(uint32_t *page_addr)
{
    DEBUG("address to erase: %p\n", page_addr);

    NVMADDR = (uint32_t)page_addr;
    nvm_perform_op(ERASE_PAGE_OP);
}

void flashpage_write_raw(void *target_addr, const void *data, size_t len)
{
    /* assert multiples of FLASHPAGE_RAW_BLOCKSIZE are written and no less of
       that length. */
    assert(!(len % FLASHPAGE_RAW_BLOCKSIZE));

    /* ensure writes are aligned */
    assert(!(((unsigned)target_addr % FLASHPAGE_RAW_ALIGNMENT) ||
            ((unsigned)data % FLASHPAGE_RAW_ALIGNMENT)));

    /* ensure the length doesn't exceed the actual flash size */
    assert(((unsigned)target_addr + len) <
           (CPU_FLASH_BASE + (FLASHPAGE_SIZE * FLASHPAGE_NUMOF)) + 1);

    uint32_t *dst = target_addr;
    const uint32_t *data_addr = data;


    DEBUG("[flashpage_raw] write: now writing the data\n");

    while (len) {
        uint32_t op;
        if (len < ROW_SIZE || (((uint32_t)data_addr) % ROW_SIZE)) {
            op = WORD_PROGRAM_OP;
#if defined(CPU_FAM_PIC32MX)
            NVMDATA = *data_addr;
#elif defined(CPU_FAM_PIC32MZ)
            NVMDATA0 = *data_addr;
#endif
        } else {
            op = ROW_PROGRAM_OP;
            NVMSRCADDR = (uint32_t)data_addr;
        }

        DEBUG("[flashpage_raw] writing %d bytes to %p\n", op == WORD_PROGRAM_OP ? WORD_SIZE : ROW_SIZE, dst);

        NVMADDR = (uint32_t)dst;
        nvm_perform_op(op);

        if (op == WORD_PROGRAM_OP) {
            len -= WORD_SIZE;
            dst++;
            data_addr++;
        } else {
            len -= ROW_SIZE;
            dst += ROW_SIZE / WORD_SIZE;
            data_addr += ROW_SIZE / WORD_SIZE;
        }
    }

    DEBUG("[flashpage_raw] write: done writing data\n");
}

void flashpage_write(int page, const void *data)
{
    uint32_t *page_addr;

    assert(page < (int)FLASHPAGE_NUMOF);

    page_addr = flashpage_addr(page);

    /* ERASE sequence */
    _erase_page(page_addr);

    /* WRITE sequence */
    if (data != NULL) {
        flashpage_write_raw(page_addr, data, FLASHPAGE_SIZE);
    }
}
