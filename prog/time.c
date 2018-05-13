/*
 * time.c
 *
 *  Created on: 12 May 2018
 *      Author: ntd
 */

#include "time.h"

uint32_t time_ms = 0;

uint32_t get_time_ms(void) { return time_ms; }

void delay_ms(uint16_t ms_)
{
    uint32_t i = time_ms;
    for(;;)
    {
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");asm(" NOP");
        if((i + ms_) < time_ms)
        {
            break;
        }
    }
}

void increment_time_ms(void)
{
    time_ms++;
}

void reset_time_ms(void)
{
    time_ms = 0;
}

