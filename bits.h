/*
 * bits.h
 *
 *  Created on: Feb 25, 2018
 *      Author: Zack Lyzen
 */

#ifndef BITS_H_
#define BITS_H_

#define BIT_0 					0x01 		// 0b00000001
#define BIT_1 					0x02 		// 0b00000010
#define BIT_2 					0x04 		// 0b00000100
#define BIT_3 					0x08 		// 0b00001000
#define BIT_4 					0x10 		// 0b00010000
#define BIT_5 					0x20 		// 0b00100000
#define BIT_6 					0x40 		// 0b01000000
#define BIT_7 					0x80 		// 0b10000000

static uint16_t bit_arrays[8] = {BIT_0, BIT_1, BIT_2, BIT_3, BIT_4, BIT_5, BIT_6, BIT_7};



#endif /* BITS_H_ */
