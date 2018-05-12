/*
 * time.h
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_TIME_H_
#define PROG_TIME_H_

#include <stdint.h>

//Global Var
uint32_t time_ms = 0;

uint32_t get_time_ms(void);;

void delay_ms(uint16_t ms_);
void increment_time_ms(void);
void reset_time_ms(void);

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



#endif /* PROG_TIME_H_ */
