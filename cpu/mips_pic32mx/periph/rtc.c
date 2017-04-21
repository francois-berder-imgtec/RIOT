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
#include "periph/rtc.h"

#ifdef RTC_NUMOF

static uint32_t bcd_to_binary(uint8_t a, uint8_t b)
{
    return a * 10 + b;
}

static uint32_t binary_to_bcd(uint32_t a)
{
    uint32_t decimal = a / 10;
    uint32_t digit = a - decimal * 10;

    return (decimal << 4) | digit;
}

static void unlock_rtc_regs(void)
{
    /* Unlock sequence */
    SYSKEY = 0;
    SYSKEY = 0xaa996655;
    SYSKEY = 0x556699aa;

    RTCCON |= _RTCCON_RTCWREN_MASK;

    SYSKEY = 0;
}

static void lock_rtc_regs(void)
{
    RTCCON &= ~_RTCCON_RTCWREN_MASK;
}

void rtc_init(void)
{
    SYSKEY = 0;
    SYSKEY = 0xaa996655;
    SYSKEY = 0x556699aa;

    OSCCONSET = _OSCCON_SOSCEN_MASK;  /* Enable external 32.768kHz crystal */
    RTCCON = _RTCCON_RTCWREN_MASK;    /* Enable write to RTCCON register */

    SYSKEY = 0;

    RTCALRM = 0;      /* Disable alarm */
    RTCTIME = 0;
    RTCDATE = 0;

    /* Turn on module and disable further writes to RTCCON */
    RTCCON = _RTCCON_ON_MASK;

    /* Wait for the clock of RTC module */
    while (!(RTCCON & _RTCCON_RTCCLKON_MASK)) {}

    rtc_poweroff();
}

int rtc_set_time(struct tm *time)
{
    uint32_t rtcdate, rtctime;

    rtcdate = binary_to_bcd(time->tm_year) << _RTCDATE_YEAR01_POSITION;
    rtcdate |= binary_to_bcd(time->tm_mon + 1) << _RTCDATE_MONTH01_POSITION;
    rtcdate |= binary_to_bcd(time->tm_mday) << _RTCDATE_DAY01_POSITION;
    rtcdate |= binary_to_bcd(time->tm_wday) << _RTCDATE_WDAY01_POSITION;

    rtctime = binary_to_bcd(time->tm_hour) << _RTCTIME_HR01_POSITION;
    rtctime |= binary_to_bcd(time->tm_min) << _RTCTIME_MIN01_POSITION;
    rtctime |= binary_to_bcd(time->tm_sec) << _RTCTIME_SEC01_POSITION;

    /* Wait until it is safe to write to registers */
    while (RTCCON & _RTCCON_RTCSYNC_MASK) {}

    unlock_rtc_regs();
    RTCDATE = rtcdate;
    RTCTIME = rtctime;
    lock_rtc_regs();

    return 0;
}

int rtc_get_time(struct tm *time)
{
    uint32_t rtcdate, rtctime;

    /* Wait until it is safe to read from registers */
    while (RTCCON & _RTCCON_RTCSYNC_MASK) {}

    rtcdate = RTCDATE;
    rtctime = RTCTIME;

    time->tm_year = bcd_to_binary(
        (rtcdate & _RTCDATE_YEAR10_MASK) >> _RTCDATE_YEAR10_POSITION,
        (rtcdate & _RTCDATE_YEAR01_MASK) >> _RTCDATE_YEAR01_POSITION);

    time->tm_mon = bcd_to_binary(
        (rtcdate & _RTCDATE_MONTH10_MASK) >> _RTCDATE_MONTH10_POSITION,
        (rtcdate & _RTCDATE_MONTH01_MASK) >> _RTCDATE_MONTH01_POSITION);

    /*
    * RTC Module stores month in range 1..12
    * Decrement to get month in range 0..11
    */
    --time->tm_mon;

    time->tm_mday = bcd_to_binary(
        (rtcdate & _RTCDATE_DAY10_MASK) >> _RTCDATE_DAY10_POSITION,
        (rtcdate & _RTCDATE_DAY01_MASK) >> _RTCDATE_DAY01_POSITION);

    time->tm_wday = bcd_to_binary(0,
        (rtcdate & _RTCDATE_WDAY01_MASK) >> _RTCDATE_WDAY01_POSITION);

    time->tm_hour = bcd_to_binary(
        (rtctime & _RTCTIME_HR10_MASK) >> _RTCTIME_HR10_POSITION,
        (rtctime & _RTCTIME_HR01_MASK) >> _RTCTIME_HR01_POSITION);

    time->tm_min = bcd_to_binary(
        (rtctime & _RTCTIME_MIN10_MASK) >> _RTCTIME_MIN10_POSITION,
        (rtctime & _RTCTIME_MIN01_MASK) >> _RTCTIME_MIN01_POSITION);

    time->tm_sec = bcd_to_binary(
        (rtctime & _RTCTIME_SEC10_MASK) >> _RTCTIME_SEC10_POSITION,
        (rtctime & _RTCTIME_SEC01_MASK) >> _RTCTIME_SEC01_POSITION);

    return 0;
}

/**
 * @brief Set an alarm for RTC to the specified value.
 *
 * @note Any already set alarm will be overwritten.
 *
 * @param[in] time          The value to trigger an alarm when hit.
 * @param[in] cb            Callback executed when alarm is hit.
 * @param[in] arg           Argument passed to callback when alarm is hit.
 *
 * @return  0 for success
 * @return -2 invalid `time` parameter
 * @return -1 other errors
 */
int rtc_set_alarm(struct tm *time, rtc_alarm_cb_t cb, void *arg)
{
    (void)time;
    (void)cb;
    (void)arg;
    return -1;
}

/**
 * @brief Gets the current alarm setting
 *
 * @param[out]  time        Pointer to structure to receive alarm time
 *
 * @return  0 for success
 * @return -1 an error occurred
 */
int rtc_get_alarm(struct tm *time)
{
    (void)time;
    return -1;
}

void rtc_clear_alarm(void)
{
}

void rtc_poweron(void)
{
    PMD6 &= ~_PMD6_RTCCMD_MASK;

    unlock_rtc_regs();
    RTCCON |= _RTCCON_ON_MASK;
    lock_rtc_regs();
}

void rtc_poweroff(void)
{
    unlock_rtc_regs();
    RTCCON &= ~_RTCCON_ON_MASK;
    lock_rtc_regs();

    PMD6 |= _PMD6_RTCCMD_MASK;
}

#endif /* RTC_NUMOF */
