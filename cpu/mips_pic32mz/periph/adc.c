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
#include "periph/adc.h"

#define ANSELxSET(P)    (base_address[P].ansel[0x08/0x4])
#define TRISxSET(P)     (base_address[P].tris[0x08/0x4])

typedef struct PIC32_ADC_tag {
    volatile uint32_t* ansel;
    volatile uint32_t* tris;
} PIC32_ADC_T;

static PIC32_ADC_T base_address[] = {
 #ifdef _PORTA_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELA,
         .tris  = (volatile uint32_t*)&TRISA
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTB_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELB,
         .tris  = (volatile uint32_t*)&TRISB
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTC_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELC,
         .tris  = (volatile uint32_t*)&TRISC
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTD_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELD,
         .tris  = (volatile uint32_t*)&TRISD
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTE_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELE,
         .tris  = (volatile uint32_t*)&TRISE
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTF_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELF,
         .tris  = (volatile uint32_t*)&TRISF
     },
 #else
     {0 , 0},
 #endif
 #ifdef _PORTG_BASE_ADDRESS
     {
         .ansel = (volatile uint32_t*)&ANSELG,
         .tris  = (volatile uint32_t*)&TRISG
     },
 #else
     {0 , 0},
 #endif
 };

/**
 * @brief   Initialize the given ADC line
 *
 * The ADC line is initialized in synchronous, blocking mode.
 *
 * @param[in] line          line to initialize
 *
 * @return                  0 on success
 * @return                  -1 on invalid ADC line
 */
int adc_init(adc_t line)
{
    uint8_t port = line >> 4;
    uint8_t pin_no = line & 0x0F;

    ANSELxSET(port) = pin_no;
    TRISxSET(port) = pin_no;
}

/**
 * @brief   Sample a value from the given ADC line
 *
 * This function blocks until the conversion has finished. Please note, that if
 * more than one line share the same ADC device, and if these lines are sampled
 * at the same time (e.g. from different threads), the one called secondly waits
 * for the first to finish before its conversion starts.
 *
 * @param[in] line          line to sample
 * @param[in] res           resolution to use for conversion
 *
 * @return                  the sampled value on success
 * @return                  -1 if resolution is not applicable
 */
int adc_sample(adc_t line, adc_res_t res)
{
    /* The ADC module only allows 10-bit resolutions */
    if (res > ADC_RES_10BIT)
        return -1;


  //  AD1CHS = pin << AD1CON1_PINS;
    ADCCON1AD1CON1 = _AD1CON1_SAMP_MASK;
    while( AD1CON1bits.SAMP || !AD1CON1bits.DONE );
    return ADC1BUF0;
}