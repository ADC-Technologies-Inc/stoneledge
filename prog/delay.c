/*
 * delay.c
 *
 *  Created on: Mar 14, 2018
 *      Author: Zack Lyzen
 */

#include "delay.h"

void delay_loop(void)
{
    long      i;

    for (i = 0; i < 3; i++) {}
}

void  delay_loop_t(int time)
{
	long i;

	for (i = 0; i < time; i++) {}
}


