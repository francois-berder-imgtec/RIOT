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
 * @ingroup     boards_pic32-wifire
 * @{
 *
 * @file
 * @brief       peripheral configuration for the Digilent PIC32 WiFire
 *
 * @author       Neil Jones <Neil.Jones@imgtec.com>
 */
#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H

#include "cpu.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The peripheral clock is required for the UART Baud rate calculation
 *          It is configured by the 'config' registers (see pic32_config_settings.c)
 */
#define PERIPHERAL_CLOCK (100000000)  /* Hz */

/**
 * @name    Timer definitions
 * @{
 */
#define TIMER_NUMOF         (1)
#define TIMER_0_CHANNELS    (3)
/** @} */

/**
  * @name    UART Definitions
  * @{
  */
static const uart_conf_t uart_config[] = {
    {   /* Virtual COM port */
        .base       = (volatile unsigned int *)_UART4_BASE_ADDRESS,
        .clock      = PERIPHERAL_CLOCK,
        .rx_pin     = GPIO_PIN(PORT_F, 2),
        .tx_pin     = GPIO_PIN(PORT_F, 8),
        .rx_mux_reg = &U4RXR,
        .tx_mux_reg = &RPF8R,
        .rx_af      = GPIO_AF11,
        .tx_af      = GPIO_AF2,
        .vector     = _UART4_RX_VECTOR,
    },
};

#define UART_0_ISR          (isr_usart4)
#define UART_NUMOF          ((unsigned int)ARRAY_SIZE(uart_config))
/** @} */

/**
 * @name    SPI device configuration
 *
 * @{
 */

static const spi_conf_t spi_config[] = {
    {   /*
         * SPI 3 (microSD card)
         *   MOSI -> RC4
         *   MISO -> RB10
         *   SCK  -> RB14
         */
        .base     = (volatile uint32_t *)_SPI3_BASE_ADDRESS,
        .mosi_pin = GPIO_PIN(PORT_C, 4),
        .mosi_reg = (volatile uint32_t*)&RPC4R,
        .mosi_af  = 0b0111,
        .miso_pin = GPIO_PIN(PORT_B, 10),
        .miso_reg = (volatile uint32_t*)&SDI3R,
        .miso_af  = 0b0110,
    },

    {   /*
         * SPI 4 (MRF24WG0MA - wifi module)
         *   MOSI -> RG0
         *   MISO -> RF5
         *   SCK  -> RD10
         */
        .base     = (volatile uint32_t *)_SPI4_BASE_ADDRESS,
        .mosi_pin = GPIO_PIN(PORT_G, 0),
        .mosi_reg = (volatile uint32_t*)&RPG0R,
        .mosi_af  = 0b1000,
        .miso_pin = GPIO_PIN(PORT_F, 5),
        .miso_reg = (volatile uint32_t*)&SDI4R,
        .miso_af  = 0b0010,
    },
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
