/*
 * Copyright(C) 2020 Francois Berder <fberder@outlook.fr>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

#include "cpu.h"
#include "eic.h"
#include "periph/rtc.h"

#ifdef _RTCC

#define RTC_YEAR_OFFSET (100)       /**< RTCC has only two-digit notation */

static struct {
    rtc_alarm_cb_t cb;          /**< callback called from RTC interrupt */
    void *arg;                  /**< argument passed to the callback */
} isr_ctx;

static uint32_t val2bcd(int val, int shift, uint32_t mask)
{
    uint32_t bcdhigh = 0;

    while (val >= 10) {
        bcdhigh++;
        val -= 10;
    }

    return ((((bcdhigh << 4) | val) << shift) & mask);
}

static int bcd2val(uint32_t val, int shift, uint32_t mask)
{
    int tmp = (int)((val & mask) >> shift);
    return (((tmp >> 4) * 10) + (tmp & 0x0f));
}

static inline void rtc_unlock(void)
{
    unsigned int ctx;

    ctx = irq_disable();
    SYSKEY = 0xaa996655;
    SYSKEY = 0x556699aa;
    RTCCONSET = _RTCCON_RTCWREN_MASK;
    irq_restore(ctx);
}

static inline void rtc_lock(void)
{
    RTCCONCLR = _RTCCON_RTCWREN_MASK;
}

void rtc_init(void)
{
    rtc_unlock();

    rtc_lock();
}

int rtc_set_time(struct tm *time)
{
    rtc_unlock();

    RTCDATE = (val2bcd((time->tm_year % 100), _RTCDATE_YEAR01_POSITION, _RTCDATE_YEAR10_MASK | _RTCDATE_YEAR01_MASK) |
               val2bcd(time->tm_mon + 1,  _RTCDATE_MONTH01_POSITION, _RTCDATE_MONTH10_MASK | _RTCDATE_MONTH01_MASK) |
               val2bcd(time->tm_mday, _RTCDATE_MONTH10_MASK | _RTCDATE_MONTH01_MASK, _RTCDATE_DAY10_MASK | _RTCDATE_DAY01_MASK));
    RTCTIME = (val2bcd(time->tm_hour, _RTCTIME_HR01_POSITION, _RTCTIME_HR10_MASK | _RTCTIME_HR01_MASK) |
               val2bcd(time->tm_min,  _RTCTIME_MIN01_POSITION, _RTCTIME_MIN10_MASK | _RTCTIME_MIN01_MASK) |
               val2bcd(time->tm_sec,  _RTCTIME_SEC01_POSITION, _RTCTIME_SEC10_MASK | _RTCTIME_SEC01_MASK));

    rtc_lock();
}

int rtc_get_time(struct tm *time)
{
    uint32_t tr = RTCTIME;
    uint32_t dr = RTCDATE;

    time->tm_year = bcd2val(dr, _RTCDATE_YEAR01_POSITION, _RTCDATE_YEAR10_MASK | _RTCDATE_YEAR01_MASK) + YEAR_OFFSET;
    time->tm_mon  = bcd2val(dr, _RTCDATE_MONTH01_POSITION, _RTCDATE_MONTH10_MASK | _RTCDATE_MONTH01_MASK) - 1;
    time->tm_mday = bcd2val(dr, _RTCDATE_DAY01_POSITION, _RTCDATE_DAY10_MASK | _RTCDATE_DAY01_MASK);
    time->tm_hour = bcd2val(tr, _RTCTIME_HR01_POSITION, _RTCTIME_HR10_MASK | _RTCTIME_HR01_MASK);
    time->tm_min  = bcd2val(tr, _RTCTIME_MIN01_POSITION, _RTCTIME_MIN10_MASK | _RTCTIME_MIN01_MASK);
    time->tm_sec  = bcd2val(tr, _RTCTIME_SEC01_POSITION, _RTCTIME_SEC10_MASK | _RTCTIME_SEC01_MASK);

    return 0;
}

int rtc_set_alarm(struct tm *time, rtc_alarm_cb_t cb, void *arg)
{
    isr_ctx.cb = cb;
    isr_ctx.arg = arg;
    while(RTCALRM&_RTCALRM_ALRMSYNC_MASK);          // wait ALRMSYNC to be off
    RTCALRMCLR=0xCFFF;              // clear ALRMEN, CHIME, AMASK and ARPT;
    ALRMTIME=alTime;
    ALRMDATE=alDate;                // update the alarm time and date
    RTCALRMSET=0x8000|0x00000600;   // re-enable the alarm, set alarm mask at once per day
    RTCALRMSET = _RTCALRM_ALRMEN_MASK;

#if defined(CPU_FAM_PIC32MX)
    eic_configure_priority(_RTCC_IRQ, 1, 0);
#elif defined(CPU_FAM_PIC32MZ)
    eic_configure_priority(_RTCC_VECTOR, 1, 0);
#endif
    eic_enable(_RTCC_VECTOR);
}

int rtc_get_alarm(struct tm *time)
{
    uint32_t tr = ALRMTIME;
    uint32_t dr = ALRMDATE;

    time->tm_year = bcd2val(dr, _ALRMDATE_YEAR01_POSITION, _ALRMDATE_YEAR10_MASK | _ALRMDATE_YEAR01_MASK) + YEAR_OFFSET;
    time->tm_mon  = bcd2val(dr, _ALRMDATE_MONTH01_POSITION, _ALRMDATE_MONTH10_MASK | _ALRMDATE_MONTH01_MASK) - 1;
    time->tm_mday = bcd2val(dr, _ALRMDATE_DAY01_POSITION, _ALRMDATE_DAY10_MASK | _ALRMDATE_DAY01_MASK);
    time->tm_hour = bcd2val(tr, _ALRMTIME_HR01_POSITION, _ALRMTIME_HR10_MASK | _ALRMTIME_HR01_MASK);
    time->tm_min  = bcd2val(tr, _ALRMTIME_MIN01_POSITION, _ALRMTIME_MIN10_MASK | _ALRMTIME_MIN01_MASK);
    time->tm_sec  = bcd2val(tr, _ALRMTIME_SEC01_POSITION, _ALRMTIME_SEC10_MASK | _ALRMTIME_SEC01_MASK);

    return 0;
}

void rtc_clear_alarm(void)
{
    RTCALRMCLR = _RTCALRM_ALRMEN_MASK;
}

void rtc_poweron(void)
{
    PDM6CLR = _PMD6_RTCCMD_MASK;
}

void rtc_poweroff(void)
{
    PDM6SET = _PMD6_RTCCMD_MASK;
}

#endif /* _RTCC */
