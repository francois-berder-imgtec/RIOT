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
 * @ingroup     boards_6lowpan-clicker
 * @{
 *
 * @file
 * @brief       peripheral configuration for the MikroE 6LoWPAN Clicker
 *
 * @author      Neil Jones <Neil.Jones@imgtec.com>
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
 *          Note 120MHz is the max F for this device.
 */
#define PERIPHERAL_CLOCK (96000000)  /* Hz */

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
    {   /* UART port available on MikroBus */
        .base       = (volatile unsigned int *)_UART3_BASE_ADDRESS,
        .clock      = PERIPHERAL_CLOCK,
        .rx_pin     = GPIO_PIN(PORT_F, 5),
        .tx_pin     = GPIO_PIN(PORT_F, 4),
        .rx_mux_reg = &U3RXR,
        .tx_mux_reg = &RPF4R,
        .rx_af      = GPIO_AF2,
        .tx_af      = GPIO_AF1,
        .vector     = _UART_3_VECTOR,
        .irq        = _UART3_RX_IRQ,
    },
};

#define UART_0_ISR          (isr_usart3)
#define UART_NUMOF          ((unsigned int)ARRAY_SIZE(uart_config))
/** @} */

/**
 * @name    SPI device configuration
 *
 * @{
 */
static const spi_conf_t spi_config[] = {
    {   /*
         * SPI 1 (Mikrobus)
         *      MOSI -> RD4
         *      MISO -> RD3
         *      SCK  -> RD2
         */
        .base = (volatile uint32_t *)_SPI1_BASE_ADDRESS,
        .mosi_pin = GPIO_PIN(PORT_D, 4),
        .mosi_reg = (volatile uint32_t*)&RPD4R,
        .mosi_af  = 0b1000,
        .miso_pin = GPIO_PIN(PORT_D, 3),
        .miso_reg = (volatile uint32_t*)&SDI1R,
        .miso_af  = 0b0000
    },

    {   /*
         * SPI 2 (6LoWPAN radio)
         *      MOSI -> RG8
         *      MISO -> RG7
         *      SCK  -> RG6
         */
        .base = (volatile uint32_t *)_SPI2_BASE_ADDRESS,
        .mosi_pin = GPIO_PIN(PORT_G, 8),
        .mosi_reg = (volatile uint32_t*)&RPG8R,
        .mosi_af  = 0b0110,
        .miso_pin = GPIO_PIN(PORT_G, 7),
        .miso_reg = (volatile uint32_t*)&SDI2R,
        .miso_af  = 0b0001
    }
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
