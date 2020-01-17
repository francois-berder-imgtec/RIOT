/*
 * Copyright(C) 2020 Francois Berder <fberder@outlook.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include "assert.h"
#include "board.h"
#include "mutex.h"
#include "periph/gpio.h"
#include "periph/spi.h"

#define SPIxCON(U)          ((U)[0x00 / 4])
#define SPIxCONCLR(U)       ((U)[0x04 / 4])
#define SPIxCONSET(U)       ((U)[0x08 / 4])
#define SPIxSTAT(U)         ((U)[0x10 / 4])
#define SPIxSTATCLR(U)      ((U)[0x14 / 4])
#define SPIxBUF(U)          ((U)[0x20 / 4])
#define SPIxBRG(U)          ((U)[0x30 / 4])
#define SPIxCON2(U)         ((U)[0x40 / 4])

/* PERIPHERAL_CLOCK must be defined in board file */

static mutex_t locks[SPI_NUMOF];

static inline volatile uint32_t *dev(spi_t bus)
{
    return spi_config[bus].base;
}

void spi_init(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    mutex_init(&locks[bus]);

    PMD5SET = _PMD5_SPI1MD_MASK << (bus - 1);
    spi_init_pins(bus);
}

void spi_init_pins(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    gpio_init(spi_config[bus].mosi_pin, GPIO_OUT);
    gpio_init(spi_config[bus].miso_pin, GPIO_IN);
    *(spi_config[bus].mosi_reg) = spi_config[bus].mosi_af;
    *(spi_config[bus].miso_reg) = spi_config[bus].miso_af;
}

int spi_acquire(spi_t bus, spi_cs_t cs, spi_mode_t mode, spi_clk_t clk)
{
    volatile int rdata __attribute__((unused));
    (void)cs;

    assert(bus < SPI_NUMOF);

    mutex_lock(&locks[bus]);

    PMD5CLR = _PMD5_SPI1MD_MASK << (bus - 1);

    SPIxCON(dev(bus)) = 0;
    SPIxCON2(dev(bus)) = 0;

    /* Clear receive FIFO */
    rdata = SPIxBUF(dev(bus));

    switch (mode) {
        case SPI_MODE_0:
            SPIxCONCLR(dev(bus)) = (_SPI1CON_CKP_MASK | _SPI1CON_CKE_MASK);
            break;
        case SPI_MODE_1:
            SPIxCONCLR(dev(bus)) = _SPI1CON_CKP_MASK;
            SPIxCONSET(dev(bus)) = _SPI1CON_CKE_MASK;
            break;
        case SPI_MODE_2:
            SPIxCONCLR(dev(bus)) = _SPI1CON_CKE_MASK;
            SPIxCONSET(dev(bus)) = _SPI1CON_CKP_MASK;
            break;
        case SPI_MODE_3:
            SPIxCONSET(dev(bus)) = (_SPI1CON_CKP_MASK | _SPI1CON_CKE_MASK);
            break;
        default:
            return SPI_NOMODE;
    }

    SPIxBRG(dev(bus)) = (PERIPHERAL_CLOCK / (2 * clk)) - 1;
    SPIxSTATCLR(dev(bus)) = _SPI1STAT_SPIROV_MASK;
    SPIxCONSET(dev(bus)) = (_SPI1CON_ON_MASK | _SPI1CON_MSTEN_MASK);

    return SPI_OK;
}

void spi_release(spi_t bus)
{
    assert(bus < SPI_NUMOF);

    /* SPI module must be turned off before powering it down */
    SPIxCON(dev(bus)) = 0;

    PMD5SET = _PMD5_SPI1MD_MASK << (bus - 1);

    mutex_unlock(&locks[bus]);
}

void spi_transfer_bytes(spi_t bus, spi_cs_t cs, bool cont,
                        const void *out, void *in, size_t len)
{
    const uint8_t *out_buffer = (const uint8_t*)out;
    uint8_t *in_buffer = (uint8_t*)in;

    assert(bus < SPI_NUMOF);

    if (cs != SPI_CS_UNDEF)
        gpio_clear((gpio_t)cs);

    while (len--) {
        uint8_t rdata;

        if (out_buffer) {
            SPIxBUF(dev(bus)) = *out_buffer++;

            /* Wait until TX FIFO is empty */
            while (!(SPIxSTAT(dev(bus)) & _SPI1STAT_SPITBE_MASK)) {}
        }

        /* Wait until RX FIFO is not empty */
        while (!(SPIxSTAT(dev(bus)) & _SPI1STAT_SPIRBF_MASK)) {}

        rdata = SPIxBUF(dev(bus));

        if (in_buffer)
            *in_buffer++ = rdata;
    }

    if (!cont && cs != SPI_CS_UNDEF)
        gpio_set((gpio_t)cs);
}
