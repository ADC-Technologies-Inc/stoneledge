/*
 * lcd_driver.c
 *
 *  Created on: Mar 12, 2018
 *      Author: Zack Lyzen
 */

// INCLUDES

#include "lcd_driver.h"

// DECLARATIONS

#define NUM_CHARS 	32

struct lcd_char{
	uint16_t 	data;
	uint16_t 	addr;
	uint16_t 	new;
};

static struct lcd_char screen[NUM_CHARS]; 			// 32 chars for every character on the 16x2 display

// FUNCTION PROTOTYPES

void lcd_set_di(void);
void lcd_clear_di(void);

void lcd_set_rw(void);
void lcd_clear_rw(void);

void lcd_set_e(void);
void lcd_clear_e(void);

void d0_set(void);
void d1_set(void);
void d2_set(void);
void d3_set(void);
void d4_set(void);
void d5_set(void);
void d6_set(void);
void d7_set(void);

void d0_clear(void);
void d1_clear(void);
void d2_clear(void);
void d3_clear(void);
void d4_clear(void);
void d5_clear(void);
void d6_clear(void);
void d7_clear(void);

void lcd_set_char(uint16_t char_, uint16_t loc_)
{
	if(screen[loc_].data != char_)
	{
		screen[loc_].data = char_;
		screen[loc_].new = 1;
	}
}

void lcd_write_data(void)
{
	static int i;
	i = 0;
	while(i < NUM_CHARS)
	{
		if(screen[i].new == 1)
		{
			lcd_command(BIT7 | screen[i].addr);
			lcd_write_char(screen[i].data);
		}
		i++;
	}
}

void lcd_command(uint16_t command_)
{
	lcd_set_pins(command_);
	lcd_clear_di();
	lcd_clear_rw();
	lcd_set_e();
	delay_ms(1); 			// 1ms delay
	lcd_clear_e();
	delay_ms(1); 			// 1ms delay
}

void lcd_write_char(uint16_t char_)
{
	lcd_set_pins(char_);
	lcd_set_di();
	lcd_clear_rw();
	lcd_set_e();
	delay_ms(1); 			// 1ms delay
	lcd_clear_e();
	delay_ms(1); 			// 1ms delay
}

void lcd_set_pins(uint16_t data_)
{
	if(data_ & BIT0)
		d0_set();
	else
		d0_clear();
	if(data_ & BIT1)
		d1_set();
	else
		d1_clear();
	if(data_ & BIT2)
		d2_set();
	else
		d2_clear();
	if(data_ & BIT3)
		d3_set();
	else
		d3_clear();
	if(data_ & BIT4)
		d4_set();
	else
		d4_clear();
	if(data_ & BIT5)
		d5_set();
	else
		d5_clear();
	if(data_ & BIT6)
		d6_set();
	else
		d6_clear();
	if(data_ & BIT7)
		d7_set();
	else
		d7_clear();

}

void lcd_set_di(void)
{
	GpioDataRegs.GPASET.bit.GPIO28 = 0x01;
}

void lcd_clear_di(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO28 = 0x01;
}

void lcd_set_rw(void)
{
	GpioDataRegs.GPASET.bit.GPIO9 = 0x01;
}

void lcd_clear_rw(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO9 = 0x01;
}

void lcd_set_e(void)
{
	GpioDataRegs.GPASET.bit.GPIO26 = 0x01;
}

void lcd_clear_e(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO26 = 0x01;
}

void d0_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO22 = 0x01;
}

void d1_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO23 = 0x01;
}

void d2_set(void)
{
	GpioDataRegs.GPBSET.bit.GPIO42 = 0x01;
}

void d3_set(void)
{
	GpioDataRegs.GPBSET.bit.GPIO43 = 0x01;
}

void d4_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO27 = 0x01;
}

void d5_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO31 = 0x01;
}

void d6_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO30 = 0x01;
}

void d7_set(void)
{
	GpioDataRegs.GPASET.bit.GPIO29 = 0x01;
}

void d0_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO22 = 0x01;
}

void d1_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO23 = 0x01;
}

void d2_clear(void)
{
	GpioDataRegs.GPBCLEAR.bit.GPIO42 = 0x01;
}

void d3_clear(void)
{
	GpioDataRegs.GPBCLEAR.bit.GPIO43 = 0x01;
}

void d4_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO27 = 0x01;
}

void d5_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO31 = 0x01;
}

void d6_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO30 = 0x01;
}

void d7_clear(void)
{
	GpioDataRegs.GPACLEAR.bit.GPIO29 = 0x01;
}


void lcd_init(void)
{
	static int i;
	i = 0;
	while(i < (NUM_CHARS/2))
	{
		screen[i].addr = i;
		screen[i + (NUM_CHARS/2)].addr = i + 0x40;
		i++;
	}

	lcd_clear_e(); 				// remove enable
	delay_ms(40); 				// 40ms delay
	lcd_command(0x30); 			// wake up #1
	delay_ms(5); 				// 5ms delay
	lcd_command(0x30); 			// wake up #2
	delay_ms(1); 				// 1ms delay
	lcd_command(0x30); 			// wake up #3
	delay_ms(1); 				// 1ms delay
	lcd_command(0x38); 			// 8-bit-2-line
	lcd_command(0x10); 			// set cursor
	lcd_command(0x0C); 			// display on, cursor on
	lcd_command(0x06); 			// entry mode set
	delay_ms(1); 				// 1ms delay
	lcd_command(BIT7 | 0x00); 	// set cursor to position 0
	lcd_command(0x01); 			// clear display
	delay_ms(1);
}

