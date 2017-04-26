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
 * @defgroup    boards_pic32-wifire Digilent PIC32 WiFire
 * @ingroup     boards
 * @brief       peripheral configuration for the Digilent PIC32 WiFire
 * @{
 *
 * @file
 * @brief       peripheral configuration for the Digilent PIC32 WiFire
 *
 * @author       Neil Jones <Neil.Jones@imgtec.com>
 */
#ifndef _PERIPH_CONF_H_
#define _PERIPH_CONF_H_

#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "vendor/p32mz2048efg100.h"

/**
 * @brief The peripheral clock is required for the UART Baud rate calculation
 *        It is configured by the 'config' registers (see pic32_config_settings.c)
 */
#define PERIPHERAL_CLOCK (100000000)  /* Hz */

/**
 * @brief   Timer definitions
 * @{
 */
#define TIMER_NUMOF         (1)
#define TIMER_0_CHANNELS    (3)
/** @} */

/**
  * @brief   UART Definitions
  *          There are 6 UARTS available on this CPU.
  *          We route debug via UART4 on this board,
  *          this is the UART connected to the FTDI USB <-> UART device.
  *
  *          Note Microchip number the UARTS 1->4.
  * @{
  */
#define UART_NUMOF          (6)
#define DEBUG_VIA_UART      (4)
#define DEBUG_UART_BAUD     (9600)
/** @} */

/**
 * @name    SPI device configuration
 *
 * @{
 */

static const spi_conf_t spi_config[] = {
    {}, /* No SPI0 on PIC32 */

    {   /*
         * SPI 1 (J10 connector)
         *      MOSI -> RE5
         *      MISO -> RD2
         *      SCK  -> RD1
         */
        .mosi_pin = GPIO_PIN(PORT_E, 5),
        .mosi_reg = (volatile uint32_t*)&RPE5R,
        .mosi_af  = 0b0101,
        .miso_pin = GPIO_PIN(PORT_D, 2),
        .miso_reg = (volatile uint32_t*)&SDI1R,
        .miso_af  = 0b000
    },

    {   /*
         * SPI 2 (J9 connector)
         *      MOSI -> RF0
         *      MISO -> RD11
         *      SCK  -> RG6
         */
        .mosi_pin = GPIO_PIN(PORT_F, 0),
        .mosi_reg = (volatile uint32_t*)&RPF0R,
        .mosi_af  = 0b0110,
        .miso_pin = GPIO_PIN(PORT_D, 11),
        .miso_reg = (volatile uint32_t*)&SDI2R,
        .miso_af  = 0b0011,
    },
};

#define SPI_NUMOF           (2)
/** @} */

/**
 * @name    I2C device configuration
 *
 * @{
 */

static const i2c_conf_t i2c_config[] = {
    {}, /* No I2C0 on PIC32 */

    {   /*
         * No I2C 1, since SDA pin is connected
         * only to the INT pin of MRF24WG0MA
         */
    },

    {   /* I2C 2 (J7 and J10 connector)
         *      SCL -> RA2
         *      SDA -> RA3
         */
        .scl_pin = GPIO_PIN(PORT_A, 2),
        .sda_pin = GPIO_PIN(PORT_A, 3)
    },

    {   /* I2C 3 (J10 connector)
         *      SCL -> RG8
         *      SDA -> RG7
         */
        .scl_pin = GPIO_PIN(PORT_F, 8),
        .sda_pin = GPIO_PIN(PORT_F, 2)
    },

    {   /* I2C 4 (J6 and J8 connector)
         *      SCL -> RG8
         *      SDA -> RG7
         */
        .scl_pin = GPIO_PIN(PORT_G, 8),
        .sda_pin = GPIO_PIN(PORT_G, 7)
    }
};

#define I2C_NUMOF           (4)
/** @} */


/**
 * @name    RTC device configuration
 *
 * @{
 */
#define RTC_NUMOF           (1)
/** @} */


/**
 * @name    Flash device configuration
 *
 * @{
 */
 #define FLASHPAGE_SIZE     (16 * 1024)

 #define FLASHPAGE_NUMOF    ((__PIC32_FLASH_SIZE * 1024) / FLASHPAGE_SIZE)
/** @} */

#ifdef __cplusplus
}
#endif

#endif
/** @} */
