/*
 * switches.c
 *
 *  Created on: Feb 18, 2018
 *      Author: Zack Lyzen
 */

#include "switches.h"

static uint16_t boardID = 0;
static uint16_t dutySettings[8];
static uint16_t switchesInitialized = 0;

void InitializeSwitches(void)
{
	if(!switchesInitialized)
	{
		SetBoardID();
		SetDutyCycles();
	}
	switchesInitialized = 1;
}

void SetBoardID(void)
{

}

void SetDutyCycles(void)
{

}

uint16_t GetBoardID(void)
{
	return boardID;
}

uint16_t GetDutyCycles(uint16_t rhu_)
{
	return dutySettings[rhu_-1];
}


