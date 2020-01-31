/*
 * Copyright(C) 2020 Francois Berder <fberder@outlook.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     cpu_mips_pic32_common
 * @ingroup     drivers_periph_i2c
 * @{
 *
 * @file
 * @brief       Low-level I2C driver implementation
 *
 * This driver supports the PIC32MX and PIC32MZ families.
 * @note This implementation only implements the 7-bit addressing polling mode
 * (for now interrupt mode is not available)
 *
 * @author      Francois Berder <fberder@outlook.fr>
 *
 * @}
 */

#include <assert.h>
#include <errno.h>
#include "board.h"
#include "mutex.h"
#include "periph/gpio.h"
#include "periph/i2c.h"

#define I2CxCON(I)              ((I)[0x00/0x04])
#define I2CxCONCLR(I)           ((I)[0x04/0x04])
#define I2CxCONSET(I)           ((I)[0x08/0x04])
#define I2CxSTAT(I)             ((I)[0x10/0x04])
#define I2CxBRG(I)              ((I)[0x40/0x04])
#define I2CxTRN(I)              ((I)[0x50/0x04])
#define I2CxRCV(I)              ((I)[0x60/0x04])
#define WRITE_MODE              (0)
#define READ_MODE               (1)
#define PULSE_GOBBLER_DELAY     (104)   /* In nanoseconds */

/* PERIPHERAL_CLOCK must be defined in board file */

static mutex_t locks[I2C_NUMOF];

static inline volatile unsigned int *base(i2c_t dev)
{
    return i2c_config[dev].base;
}

static void wait_for_idle_bus(i2c_t dev)
{
    while(I2CxCON(base(dev)) &
            ( _I2C1CON_SEN_MASK
            | _I2C1CON_PEN_MASK
            | _I2C1CON_RSEN_MASK
            | _I2C1CON_RCEN_MASK
            | _I2C1CON_ACKEN_MASK)
    || (I2CxSTAT(base(dev)) & _I2C1STAT_TRSTAT_MASK)) {}
}

static int send_byte(i2c_t dev, uint8_t data)
{
    I2CxTRN(base(dev)) = data;
    wait_for_idle_bus(dev);
    while (I2CxSTAT(base(dev)) & _I2C1STAT_TBF_MASK) {}

    if (I2CxSTAT(base(dev)) & _I2C1STAT_BCL_MASK         /* Collision detected */
    ||  I2CxSTAT(base(dev)) & _I2C1STAT_ACKSTAT_MASK)    /* NACK received */
        return 0;

    return 1;
}

static int receive_byte(i2c_t dev, uint8_t *data, uint8_t nak)
{
    I2CxCONSET(base(dev)) = _I2C1CON_RCEN_MASK;
    wait_for_idle_bus(dev);

    /* Wait for some data in RX FIFO */
    while (!(I2CxSTAT(base(dev)) & _I2C1STAT_RBF_MASK)) {}

    /* Check for a collision */
    if (I2CxSTAT(base(dev)) & _I2C1STAT_BCL_MASK)
        return 0;

    /* Send ACK/NAK */
    if (nak)
        I2CxCONSET(base(dev)) = _I2C1CON_ACKDT_MASK;
    else
        I2CxCONCLR(base(dev)) = _I2C1CON_ACKDT_MASK;
    I2CxCONSET(base(dev)) = _I2C1CON_ACKEN_MASK;
    wait_for_idle_bus(dev);

    *data = I2CxRCV(base(dev));
    return 1;
}

static int send_address(i2c_t dev, uint8_t address, uint8_t read_byte)
{
    uint8_t tmp = (address << 1) | (read_byte & 0x1);

    return send_byte(dev, tmp);
}

static void send_start(i2c_t dev)
{
    I2CxCONSET(base(dev)) = _I2C1CON_SEN_MASK;
    while (I2CxCON(base(dev)) & _I2C1CON_SEN_MASK) {}
}

static void send_stop(i2c_t dev)
{
    I2CxCONSET(base(dev)) = _I2C1CON_PEN_MASK;
    while (I2CxCON(base(dev)) & _I2C1CON_PEN_MASK) {}
}

void i2c_init(i2c_t dev)
{
    uint64_t baudrate = 0;

    assert(dev < I2C_NUMOF);

    mutex_init(&locks[dev]);

    /* Configure SCL and SDA as digital output */
    gpio_init(i2c_config[dev].scl_pin, GPIO_OUT);
    gpio_init(i2c_config[dev].sda_pin, GPIO_OUT);

    I2CxCON(base(dev)) = 0;

    /*
     * From PIC32 family reference manual,
     * Section 24. Inter-Integrated Circuit, Equation 24-1:
     *
     *               10^9
     *              ------  - PGD
     *              2*Fsck
     * baudrate = ---------------    - 2
     *                10^9
     */
    baudrate = (1000 * 1000 * 1000) / (2 * i2c_config[dev].speed) - PULSE_GOBBLER_DELAY;
    baudrate *= PERIPHERAL_CLOCK;
    baudrate /= (1000 * 1000 * 1000);
    baudrate -= 2;

    I2CxBRG(base(dev)) = baudrate;
    I2CxCONSET(base(dev)) = _I2C1CON_SMEN_MASK;
}

int i2c_acquire(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    mutex_lock(&locks[dev]);
    return 0;
}

void i2c_release(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    mutex_unlock(&locks[dev]);
}

int i2c_read_byte(i2c_t dev, uint16_t addr, void *data, uint8_t flags)
{
    return i2c_read_bytes(dev, addr, data, 1, flags);
}

int i2c_read_bytes(i2c_t dev, uint16_t addr,
                   void *data, size_t len, uint8_t flags)
{
    uint8_t *buffer = (uint8_t*)data;
    size_t byte_received_count = 0;

    assert(dev < I2C_NUMOF);

    if (len == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, addr, READ_MODE) != 1)
        return 0;

    while (byte_received_count < len) {
        if (receive_byte(dev, &buffer[byte_received_count], (byte_received_count + 1) == len) != 1)
            return byte_received_count;
        ++byte_received_count;
    }

    send_stop(dev);

    return 0;
}

int i2c_read_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                 void *data, uint8_t flags)
{
    return i2c_read_regs(dev, addr, reg, data, 1, flags);
}

int i2c_read_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                  void *data, size_t len, uint8_t flags)
{
    int ret = i2c_write_byte(dev, addr, reg, flags);
    if (ret != 1)
        return ret;

    return i2c_read_bytes(dev, addr, data, len, flags);
}

int i2c_write_byte(i2c_t dev, uint16_t addr, uint8_t data, uint8_t flags)
{
    return i2c_write_bytes(dev, addr, &data, sizeof(data), flags);
}

int i2c_write_bytes(i2c_t dev, uint16_t addr, const void *data,
                    size_t len, uint8_t flags)
{
    uint8_t *buffer = (uint8_t*)data;
    size_t byte_sent_count = 0;

    assert(dev < I2C_NUMOF);

    /* If reload was NOT set, must either stop or start */
    if ((flags & I2C_NOSTART) && (flags & I2C_NOSTOP)) {
        return -EOPNOTSUPP;
    }

    if (len == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, addr, WRITE_MODE) != 1)
        return 0;

    while (byte_sent_count < len) {
        if (send_byte(dev, buffer[byte_sent_count]) != 1)
            return byte_sent_count;
        ++byte_sent_count;
    }

    send_stop(dev);

    return byte_sent_count;
}

int i2c_write_reg(i2c_t dev, uint16_t addr, uint16_t reg,
                  uint8_t data, uint8_t flags)
{
    return i2c_write_regs(dev, addr, reg, &data, 1, flags);
}

int i2c_write_regs(i2c_t dev, uint16_t addr, uint16_t reg,
                  const void *data, size_t len, uint8_t flags)
{
    uint8_t *buffer = (uint8_t*)data;
    size_t byte_sent_count = 0;

    assert(dev < I2C_NUMOF);

    if (len == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, addr, WRITE_MODE) != 1)
        return 0;

    if (send_byte(dev, reg) != 1)
        return 0;

    while (byte_sent_count < len) {
        if (send_byte(dev, buffer[byte_sent_count]) != 1)
            return byte_sent_count;
        ++byte_sent_count;
    }

    send_stop(dev);

    return byte_sent_count;
}

void i2c_poweron(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    I2CxCONSET(base(dev)) = _I2C1CON_ON_MASK;
}

void i2c_poweroff(i2c_t dev)
{
    assert(dev < I2C_NUMOF);

    I2CxCONCLR(base(dev)) = _I2C1CON_ON_MASK;
}
