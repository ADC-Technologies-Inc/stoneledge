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
extern uint32_t time_ms;

uint32_t get_time_ms(void);

void delay_ms(uint16_t ms_);
void increment_time_ms(void);
void reset_time_ms(void);

#endif /* PROG_TIME_H_ */
