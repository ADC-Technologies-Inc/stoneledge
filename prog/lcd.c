/*
 * lcd.c
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#include "lcd.h"

#define DEBUG_LCD

#ifdef DEBUG_LCD
#include "ntd_debug.h"
#endif

static uint16_t lcd_mode = 2;
static uint32_t lcd_bottom_time = 0;
static uint32_t lcd_msg_time = 0;
static uint16_t lcd_cycle_msg_holder = 0;
static uint16_t screen_update = 0;

static uint16_t top_row[16];
static uint16_t bottom_row[16];

const static uint16_t bottom_row_static[16] = {0x50, 0x3A, 0x78, 0x78, 0x78, 0x57, 0x20, 0x4D, 0x41, 0x58, 0x3A, 0x78, 0x78, 0x2E, 0x78, 0x43}; // "P:xxxW MAX:xx.xC"

const static uint16_t msg_pre_startup[16] 				= {0x43, 0x48, 0x45, 0x43, 0x4B, 0x49, 0x4E, 0x47, 0x20, 0x54, 0x45, 0x4D, 0x50, 0x53, 0x20, 0x20}; // "CHECKING TEMPS  "
const static uint16_t msg_startup[16] 					= {0x53, 0x54, 0x41, 0x52, 0x54, 0x49, 0x4E, 0x47, 0x20, 0x48, 0x45, 0x41, 0x54, 0x45, 0x52, 0x53}; // "STARTING HEATERS"
const static uint16_t msg_init_ok[16]     				= {0x53, 0x54, 0x41, 0x52, 0x54, 0x55, 0x50, 0x20, 0x43, 0x4F, 0x4D, 0x50, 0x4C, 0x45, 0x54, 0x45}; // "STARTUP COMPLETE"
const static uint16_t msg_delay_startup_fail_therm[16] 	= {0x4F, 0x56, 0x45, 0x52, 0x48, 0x45, 0x41, 0x54, 0x20, 0x57, 0x41, 0x49, 0x54, 0x49, 0x4E, 0x47}; // "OVERHEAT WAITING"
const static uint16_t msg_init_fail_tco_ptc[16]  		= {0x54, 0x43, 0x4F, 0x20, 0x46, 0x41, 0x49, 0x4C, 0x20, 0x4F, 0x4E, 0x20, 0x49, 0x4E, 0x49, 0x54}; // "TCO FAIL ON INIT"
const static uint16_t msg_init_fail_startup[16] 		= {0x4F, 0x56, 0x45, 0x52, 0x48, 0x45, 0x41, 0x54, 0x20, 0x4F, 0x4E, 0x20, 0x49, 0x4E, 0x49, 0x54}; // "OVERHEAT ON INIT"
const static uint16_t msg_fail_tco_ptc[16] 				= {0x54, 0x43, 0x4F, 0x2F, 0x50, 0x54, 0x43, 0x20, 0x46, 0x41, 0x49, 0x4C, 0x20, 0x20, 0x20, 0x20}; // "TCO/PTC FAIL    "
const static uint16_t msg_fail_gen_therm[16] 			= {0x47, 0x45, 0x4E, 0x20, 0x4F, 0x56, 0x45, 0x52, 0x48, 0x45, 0x41, 0x54, 0x20, 0x20, 0x20, 0x20}; // "GEN OVERHEAT    "
const static uint16_t msg_overheat[16] 					= {0x52, 0x48, 0x55, 0x20, 0x4F, 0x56, 0x45, 0x52, 0x48, 0x45, 0x41, 0x54, 0x20, 0x20, 0x20, 0x20}; // "RHU OVERHEAT    "
const static uint16_t msg_recover[16] 					= {0x54, 0x45, 0x4D, 0x50, 0x53, 0x20, 0x52, 0x45, 0x43, 0x4F, 0x56, 0x45, 0x52, 0x45, 0x44, 0x20}; // "TEMPS RECOVERED "
const static uint16_t msg_preinit_fail_bad_read[16]     = {0x52, 0x45, 0x41, 0x44, 0x20, 0x54, 0x45, 0x4d, 0x50, 0x53, 0x20, 0x46, 0x41, 0x49, 0x4c, 0x20}; // "READ TEMPS FAIL "
const static uint16_t msg_init_bad_48v[16]              = {0x34, 0x38, 0x56, 0x20, 0x50, 0x4f, 0x57, 0x45, 0x52, 0x20, 0x46, 0x41, 0x49, 0x4c, 0x20, 0x20}; // "48V POWER FAIL  "
const static uint16_t msg_fail_overheat_rhu_not_on[16]  = {0x4f, 0x56, 0x45, 0x52, 0x48, 0x45, 0x41, 0x54, 0x20, 0x52, 0x48, 0x55, 0x20, 0x44, 0x49, 0x53}; // "OVERHEAT RHU DIS"
const static uint16_t msg_ramp_fail[16]                 = {0x46, 0x41, 0x49, 0x4c, 0x20, 0x52, 0x41, 0x4d, 0x50, 0x49, 0x4e, 0x47, 0x20, 0x52, 0x48, 0x55}; // "FAIL RAMPING RHU"

const static uint16_t *lcd_status_messages[14] = {msg_pre_startup
                                                  , msg_startup
                                                  , msg_init_ok
                                                  , msg_delay_startup_fail_therm
                                                  , msg_init_fail_tco_ptc
                                                  , msg_init_fail_startup
                                                  , msg_fail_tco_ptc
                                                  , msg_fail_gen_therm
                                                  , msg_overheat
                                                  , msg_recover
                                                  , msg_preinit_fail_bad_read
                                                  , msg_init_bad_48v
                                                  , msg_fail_overheat_rhu_not_on
                                                  , msg_ramp_fail};

static uint16_t msg_rhu_1[16] = {0x43, 0x50, 0x55, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43}; 					// RHU 1 INFO CPU1               "CPU1       xx.xC"
static uint16_t msg_rhu_2[16] = {0x43, 0x50, 0x55, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43}; 					// RHU 2 INFO CPU2               "CPU2       xx.xC"
static uint16_t msg_rhu_3[16] = {0x43, 0x48, 0x49, 0x50, 0x53, 0x45, 0x54, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43};                     // RHU 3 INFO MISC               "CHIPSET    xx.xC"
static uint16_t msg_rhu_4[16] = {0x42, 0x4F, 0x41, 0x52, 0x44, 0x5F, 0x52, 0x41, 0x4D, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43};                     // RHU 5 INFO RAM                "BOARD_RAM  xx.xC"
static uint16_t msg_rhu_5[16] = {0x44, 0x49, 0x4D, 0x4D, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43}; 					// RHU 4 INFO DIMM_GRP           "DIMMs      xx.xC"
static uint16_t msg_rhu_6[16] = {0x4D, 0x2E, 0x32, 0x00, 0x53, 0x53, 0x44, 0x00, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43}; 					// RHU 7 INFO M.2_GRP            "M.2 SSD    xx.xC"
static uint16_t msg_rhu_7[16] = {0x32, 0x2E, 0x35, 0x22, 0x00, 0x48, 0x44, 0x44, 0x00, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43}; 					// RHU 8 INFO SFF_GRP            "2.5" HDD   xx.xC"
static uint16_t msg_rhu_8[16] = {0x4D, 0x45, 0x5A, 0x5A, 0x41, 0x4E, 0x49, 0x4E, 0x45, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43};                     // RHU 6 INFO MEZZ               "MEZZANINE  xx.xC"
static uint16_t msg_rhu_9[16] = {0x4D, 0x41, 0x49, 0x4E, 0x42, 0x4F, 0x41, 0x52, 0x44, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43};                     // RHU 9 INFO ADDITIONAL SENSORS "MAINBOARD  xx.xC"
static uint16_t msg_rhu_10[16] = {0x4D, 0x41, 0x49, 0x4E, 0x42, 0x4F, 0x41, 0x52, 0x44, 0x00, 0x00, 0x78, 0x78, 0x2E, 0x78, 0x43};                    // RHU 9 INFO ADDITIONAL SENSORS "ADDITIONAL xx.xC"


static uint16_t *lcd_rhu_messages[10] = {msg_rhu_1, msg_rhu_2, msg_rhu_3, msg_rhu_4, msg_rhu_5, msg_rhu_6, msg_rhu_7, msg_rhu_8, msg_rhu_9, msg_rhu_10};

void LcdPostStatic(uint16_t msg_)
{
	lcd_mode = STATIC;

#ifdef DEBUG_LCD
	printf("LcdPostStatic::() Printing Static Message %d\n", msg_);
#endif
	LcdPostMsgTop(msg_);
}

void LcdPostModal(uint16_t msg_)
{
#ifdef DEBUG_LCD
    printf("LcdPostModal::() Printing Modal Message %d\n", msg_);
#endif

	lcd_mode = MODAL;
	lcd_msg_time = get_time_ms();

	LcdPostMsgTop(msg_);
}

void IncrementLcdCycleMsg(void)
{
	lcd_cycle_msg_holder++;
	if(lcd_cycle_msg_holder > 8)
	{
		lcd_cycle_msg_holder = 0;
	}

	UpdateRhuTempsLcd();

	LcdPostMsgCycle(lcd_cycle_msg_holder);
}

void LcdWritePower(uint16_t watts_)
{
	static int i;
	static int started;
	started = 0;

	if(watts_ > 999)
	{
		watts_ = 999;
	}
	i = watts_;
	if(watts_ < 100)
	{
		bottom_row[2] = 0x20; 		// if power is only 2 digits, pad with blank space
	}
	else
	{
		i /= 100;
		bottom_row[2] = (0x30 + i);
		watts_ -= (i * 100);
		started = 1;
		i = watts_;
	}
	if(watts_ < 10)
	{
		if(started)
		{
			bottom_row[3] = 0x30; 	// if there was a hundres digit then a zero for tens
		}
		else
		{
			bottom_row[3] = 0x20; 	// if there wasn't a hudreds digit either, blank space for tens digit
		}
	}
	else
	{
		i/=10;
		bottom_row[3] = (0x30 + i);
		watts_ -= (i * 10);
		started = 1;
		i = watts_;
	}
	if(!watts_)
	{
		bottom_row[4] = 0x30; 		// always put a zero
	}
	else
	{
		bottom_row[4] = (0x30 + i);
	}
}

void LcdWriteMaxTemp(uint16_t temp_)
{
	static int i;
	static int started;
	started = 0;
	i = temp_;
	if(temp_ < 100)
	{
		bottom_row[11] = 0x20; 		// if power is only 2 digits, pad with blank space
	}
	else
	{
		i /= 100;
		bottom_row[11] = (0x30 + i);
		temp_ -= (i * 100);
		started = 1;
		i = temp_;
	}
	if(temp_ < 10)
	{
		if(started)
		{
			bottom_row[12] = 0x30; 	// if there was a hundres digit then a zero for tens
		}
		else
		{
			bottom_row[12] = 0x30; 	// if there wasn't a hudreds digit either, blank space for tens digit
		}
	}
	else
	{
		i/=10;
		bottom_row[12] = (0x30 + i);
		temp_ -= (i * 10);
		started = 1;
		i = temp_;
	}
	if(!temp_)
	{
		bottom_row[14] = 0x30; 		// always put a zero
	}
	else
	{
		bottom_row[14] = (0x30 + i);
	}
}

void LcdWriteTemp(uint16_t temp_, uint16_t rhu_)
{
	static int i;
	static int started;
	started = 0;
	i = temp_;
	if(temp_ < 100)
	{
		lcd_rhu_messages[rhu_][11] = 0x20; 		// if power is only 2 digits, pad with blank space
	}
	else
	{
		i /= 100;
		lcd_rhu_messages[rhu_][11] = (0x30 + i);
		temp_ -= (i * 100);
		started = 1;
		i = temp_;
	}
	if(temp_ < 10)
	{
		if(started)
		{
			lcd_rhu_messages[rhu_][12] = 0x30; 	// if there was a hundreds digit then a zero for tens
		}
		else
		{
			lcd_rhu_messages[rhu_][12] = 0x30; 	// if there wasn't a hudreds digit either, blank space for tens digit
		}
	}
	else
	{
		i/=10;
		lcd_rhu_messages[rhu_][12] = (0x30 + i);
		temp_ -= (i * 10);
		started = 1;
		i = temp_;
	}
	if(!temp_)
	{
		lcd_rhu_messages[rhu_][14] = 0x30; 		// always put a zero
	}
	else
	{
		lcd_rhu_messages[rhu_][14] = (0x30 + i);
	}
}

void UpdateRhuTempsLcd(void)
{
	static int i;
	i = 0;
	while(i < 9)
	{
		LcdWriteTemp(GetTempDataSingle(i), i);
		i++;
	}

}

void LcdWriteStaticBottom(void)
{
	static int i;
	i = 0;
	while(i < 16)
	{
		bottom_row[i] = bottom_row_static[i];
		i++;
	}
}



void LcdPostMsgTop(uint16_t msg_)
{
	static int i;
	i = 0;
	while(i < 16)
	{
		top_row[i] = (lcd_status_messages[msg_])[i];
		i++;
	}
}

void LcdPostMsgCycle(uint16_t msg_)
{
	static int i;
	i = 0;
	while(i < 16)
	{
		top_row[i] = (lcd_rhu_messages[msg_])[i];
		i++;
	}
}

void LcdTopRow(void) // sends top row data over to lcd driver
{
	static int i;
	i = 0;
	while(i<16)
	{
		lcd_set_char(top_row[i], i);
		i++;
	}
}

void LcdBottomRow(void) // sends bottom row data over to lcd driver
{
	static int i;
	i = 0;
	while(i<16)
	{
		lcd_set_char(bottom_row[i], i+16);
		i++;
	}
}

void LcdUpdate(void)
{

	LcdTopRow(); 						// sends the top row buffer over to the lcd driver
	LcdBottomRow(); 					// sends the bottom row buffer over to the lcd driver
	lcd_write_data(); 					// tells the lcd driver to refresh and display the new information
}

void LcdService(void)
{
	if((lcd_bottom_time + MODAL_MSG_TIME) < time_ms)
	{
		LcdWriteStaticBottom(); 			// writes static characters to the bottom row buffer
		LcdWritePower(GetPowerData()); 		// writes power consumption to the bottom row buffer
		LcdWriteMaxTemp(GetMaxTempData());  // writes max temp data to the bottom row buffer
		lcd_bottom_time = get_time_ms();
		screen_update = 1;
	}

	if(lcd_mode == STATIC)
	{
		// static message posted do nothing
	}
	else if(lcd_mode == MODAL)
	{
		if((lcd_msg_time + MODAL_MSG_TIME) < time_ms) 	// if modal change after 5 seconds
		{
			lcd_mode = 2;
			IncrementLcdCycleMsg();
			lcd_msg_time = get_time_ms();
			screen_update = 1;
		}
	}
	else // if lcd_mode == CYCLE
	{
		if((lcd_msg_time + CYCLE_MSG_TIME) < time_ms) 	// if cycle change after 8 seconds
		{
			IncrementLcdCycleMsg();
			lcd_msg_time = get_time_ms();
			screen_update = 1;
		}
	}

	if(screen_update > 0) 											// if there is new info to display
	{
		LcdUpdate(); 											// sends data to lcd driver and then tells the screen to refresh
	}
	screen_update = 0;
}

void LcdInit(void)
{
	lcd_init();
}
