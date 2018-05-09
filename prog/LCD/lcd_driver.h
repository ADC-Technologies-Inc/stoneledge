/*
 * lcd_driver.h
 *
 *  Created on: Mar 12, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_LCD_LCD_DRIVER_H_
#define PROG_LCD_LCD_DRIVER_H_

#include <stdint.h>

#include "../../bits.h"

#include "../time.h"
#include "DSP2803x_Device.h"

void lcd_set_char(uint16_t char_, uint16_t loc_); 			// stores value for specific char at a certain value
void lcd_write_data(void); 									// writes all stored chars to their locations on the screen

void lcd_command(uint16_t command_); 						// sends a command to the LCD screen
void lcd_write_char(uint16_t char_); 						// writes a single char to the screen
void lcd_set_pins(uint16_t data_); 							// gets gpio pins ready to output correct char
void lcd_init(void); 										// initializes the lcd to the proper settings


#endif /* PROG_LCD_LCD_DRIVER_H_ */
