/*
 * time.c
 *
 *  Created on: Mar 7, 2018
 *      Author: Zack Lyzen
 */

#include "time.h"

static uint32_t time_ms = 0; 		// stores program time in ms

uint32_t get_time_ms(void)
{
	return time_ms;
}

void delay_ms(uint16_t ms_)
{
	static uint32_t i;
	i = time_ms;
	for(;;)
	{
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
