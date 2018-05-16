/*
 * lcd.h
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#ifndef PROG_LCD_H_
#define PROG_LCD_H_

#include <stdint.h>
#include "../bits.h"

#include "LCD/lcd_driver.h"

#include "current.h"
#include "temps.h"

void LcdPostStatic(uint16_t msg_);
void LcdPostStaticRamp(uint16_t msg_, uint16_t val_);
void LcdPostModal(uint16_t msg_);
void IncrementLcdCycleMsg(void);
void LcdWritePower(uint16_t watts_);
void LcdWriteMaxTemp(uint16_t temp_);
void UpdateRhuTempsLcd(void);
void LcdPostMsgTop(uint16_t msg_);
void LcdPostMsgCycle(uint16_t msg_);
void LcdWriteStaticBottom(void);
void LcdTopRow(void);
void LcdBottomRow(void);
void LcdUpdate(void);
void LcdService(void);
void LcdInit(void);

#endif /* PROG_LCD_H_ */
