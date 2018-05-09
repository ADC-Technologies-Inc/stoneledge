/*
 * switches.h
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_SWITCHES_H_
#define PROG_SWITCHES_H_

#include <stdint.h>

void InitializeSwitches(void);
void SetBoardID(void);
void SetDutyCycles(void);
uint16_t GetBoardID(void);
uint16_t GetDutyCycles(uint16_t rhu_);

#endif /* PROG_SWITCHES_H_ */
