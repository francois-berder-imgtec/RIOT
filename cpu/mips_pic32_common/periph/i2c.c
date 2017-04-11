/*
 * Copyright(C) 2017 Imagination Technologies Limited and/or its
 *              affiliated group companies.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include "board.h"
#include "mutex.h"
#include "periph/gpio.h"
#include "periph/i2c.h"

#define I2CxCON(I)              ((I).regs[0x00/0x04])
#define I2CxCONCLR(I)           ((I).regs[0x04/0x04])
#define I2CxCONSET(I)           ((I).regs[0x08/0x04])
#define I2CxSTAT(I)             ((I).regs[0x10/0x04])
#define I2CxBRG(I)              ((I).regs[0x40/0x04])
#define I2CxTRN(I)              ((I).regs[0x50/0x04])
#define I2CxRCV(I)              ((I).regs[0x60/0x04])
#define I2C_REGS_SPACING        (_I2C2_BASE_ADDRESS - _I2C1_BASE_ADDRESS)
#define WRITE_MODE              (0)
#define READ_MODE               (1)
#define PULSE_GOBBLER_DELAY     (104)   /* In nanoseconds */

/* PERIPHERAL_CLOCK must be defined in board file */

static mutex_t locks[I2C_NUMOF + 1];
typedef struct PIC32_I2C_tag {
    volatile uint32_t   *regs;
} PIC32_I2C_T;

static PIC32_I2C_T pic_i2c[I2C_NUMOF + 1];

static void wait_for_idle_bus(i2c_t dev)
{
    while(I2CxCON(pic_i2c[dev]) &
            ( _I2C1CON_SEN_MASK
            | _I2C1CON_PEN_MASK
            | _I2C1CON_RSEN_MASK
            | _I2C1CON_RCEN_MASK
            | _I2C1CON_ACKEN_MASK)
    || (I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_TRSTAT_MASK)) {}
}

static int send_byte(i2c_t dev, uint8_t data)
{
    I2CxTRN(pic_i2c[dev]) = data;
    wait_for_idle_bus(dev);
    while (I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_TBF_MASK) {}

    if (I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_BCL_MASK         /* Collision detected */
    ||  I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_ACKSTAT_MASK)    /* NACK received */
        return 0;

    return 1;
}

static int receive_byte(i2c_t dev, uint8_t *data, uint8_t nak)
{
    I2CxCONSET(pic_i2c[dev]) = _I2C1CON_RCEN_MASK;
    wait_for_idle_bus(dev);

    /* Wait for some data in RX FIFO */
    while (!(I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_RBF_MASK)) {}

    /* Check for a collision */
    if (I2CxSTAT(pic_i2c[dev]) & _I2C1STAT_BCL_MASK)
        return 0;

    /* Send ACK/NAK */
    if (nak)
        I2CxCONSET(pic_i2c[dev]) = _I2C1CON_ACKDT_MASK;
    else
        I2CxCONCLR(pic_i2c[dev]) = _I2C1CON_ACKDT_MASK;
    I2CxCONSET(pic_i2c[dev]) = _I2C1CON_ACKEN_MASK;
    wait_for_idle_bus(dev);

    *data = I2CxRCV(pic_i2c[dev]);
    return 1;
}

static int send_address(i2c_t dev, uint8_t address, uint8_t read_byte)
{
    uint8_t tmp = (address << 1) | (read_byte & 0x1);

    return send_byte(dev, tmp);
}

static void send_start(i2c_t dev)
{
    I2CxCONSET(pic_i2c[dev]) = _I2C1CON_SEN_MASK;
    while (I2CxCON(pic_i2c[dev]) & _I2C1CON_SEN_MASK) {}
}

static void send_stop(i2c_t dev)
{
    I2CxCONSET(pic_i2c[dev]) = _I2C1CON_PEN_MASK;
    while (I2CxCON(pic_i2c[dev]) & _I2C1CON_PEN_MASK) {}
}

int i2c_init_master(i2c_t dev, i2c_speed_t speed)
{
    uint64_t baudrate = 0;

    if (dev == 0 || dev > I2C_NUMOF)
        return -1;
    if (speed == 0)
        return -2;

    pic_i2c[dev].regs = (volatile uint32_t *)(_I2C1_BASE_ADDRESS + (dev - 1) * I2C_REGS_SPACING);

    mutex_init(&locks[dev]);

    /* Configure SCL and SDA as digital output */
    gpio_init(i2c_config[dev].scl_pin, GPIO_OUT);
    gpio_init(i2c_config[dev].sda_pin, GPIO_OUT);

    I2CxCON(pic_i2c[dev]) = 0;

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
    baudrate = (1000 * 1000 * 1000) / (2 * speed) - PULSE_GOBBLER_DELAY;
    baudrate *= PERIPHERAL_CLOCK;
    baudrate /= (1000 * 1000 * 1000);
    baudrate -= 2;

    /* I2CxBRG must not be set to 0 or 1 */
    if (baudrate == 0 || baudrate == 1)
        return -2;

    I2CxBRG(pic_i2c[dev]) = baudrate;
    I2CxCONSET(pic_i2c[dev]) = _I2C1CON_SMEN_MASK;

    return 0;
}

int i2c_acquire(i2c_t dev)
{
    if (dev == 0 || dev > I2C_NUMOF)
        return -1;

    mutex_lock(&locks[dev]);
    return 0;
}

int i2c_release(i2c_t dev)
{
    if (dev == 0 || dev > I2C_NUMOF)
        return -1;

    mutex_unlock(&locks[dev]);
    return 0;
}

int i2c_read_byte(i2c_t dev, uint8_t address, void *data)
{
    return i2c_read_bytes(dev, address, data, 1);
}

int i2c_read_bytes(i2c_t dev, uint8_t address, void *data, int length)
{
    uint8_t *buffer = (uint8_t*)data;
    int byte_received_count = 0;

    if (dev == 0 || dev > I2C_NUMOF)
        return -1;

    if (length == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, address, READ_MODE) != 1)
        return 0;

    while (byte_received_count < length) {
        if (receive_byte(dev, &buffer[byte_received_count], (byte_received_count + 1) == length) != 1)
            return byte_received_count;
        ++byte_received_count;
    }

    send_stop(dev);

    return 0;
}

int i2c_read_reg(i2c_t dev, uint8_t address, uint8_t reg, void *data)
{
    return i2c_read_regs(dev, address, reg, data, 1);
}

int i2c_read_regs(i2c_t dev, uint8_t address, uint8_t reg,
                  void *data, int length)
{
    int ret = i2c_write_byte(dev, address, reg);
    if (ret != 1)
        return ret;

    return i2c_read_bytes(dev, address, data, length);
}

int i2c_write_byte(i2c_t dev, uint8_t address, uint8_t data)
{
    return i2c_write_bytes(dev, address, &data, sizeof(data));
}

int i2c_write_bytes(i2c_t dev, uint8_t address, const void *data, int length)
{
    uint8_t *buffer = (uint8_t*)data;
    int byte_sent_count = 0;

    if (dev == 0 || dev > I2C_NUMOF)
        return -1;

    if (length == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, address, WRITE_MODE) != 1)
        return 0;

    while (byte_sent_count < length) {
        if (send_byte(dev, buffer[byte_sent_count]) != 1)
            return byte_sent_count;
        ++byte_sent_count;
    }

    send_stop(dev);

    return byte_sent_count;
}

int i2c_write_reg(i2c_t dev, uint8_t address, uint8_t reg, uint8_t data)
{
    return i2c_write_regs(dev, address, reg, &data, 1);
}

int i2c_write_regs(i2c_t dev, uint8_t address, uint8_t reg,
                   const void *data, int length)
{
    uint8_t *buffer = (uint8_t*)data;
    int byte_sent_count = 0;

    if (dev == 0 || dev > I2C_NUMOF)
        return -1;

    if (length == 0)
        return 0;

    send_start(dev);

    if (send_address(dev, address, WRITE_MODE) != 1)
        return 0;

    if (send_byte(dev, reg) != 1)
        return 0;

    while (byte_sent_count < length) {
        if (send_byte(dev, buffer[byte_sent_count]) != 1)
            return byte_sent_count;
        ++byte_sent_count;
    }

    send_stop(dev);

    return byte_sent_count;
}

void i2c_poweron(i2c_t dev)
{
    if (dev != 0 && dev <= I2C_NUMOF)
        I2CxCONSET(pic_i2c[dev]) = _I2C1CON_ON_MASK;
}

void i2c_poweroff(i2c_t dev)
{
    if (dev != 0 && dev <= I2C_NUMOF)
        I2CxCONCLR(pic_i2c[dev]) = _I2C1CON_ON_MASK;
}