/*
 * mux.h
 *
 *  Created on: Mar 9, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_MUX_H_
#define PROG_MUX_H_

#include <stdint.h>

void MUX_Init(void);
uint16_t MUX_Get(void);
void MUX_Set(uint16_t channel_);


#endif /* PROG_MUX_H_ */
