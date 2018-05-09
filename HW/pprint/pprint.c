/*
 * pprint.c
 *
 *  Created on: Mar 8, 2017
 *      Author: Zack Lyzen
 */

#include "../../HW/pprint/pprint.h"

static short indexPointer = 0;
static char printBuffer[100];

void ShortToChar(short passed);

void  PPrint(void) 									// sends ("prints") the loaded buffer over serial
{

	//ChkErr();
	//UartMultiTx(printBuffer, indexPointer);
}

short PPrintAS(short subject) 			// appends a short to the buffer
{
	ShortToChar(subject);
	printBuffer[indexPointer] = ',';
	indexPointer++;
	return indexPointer;
}

short PPrintDL(void)
{
	printBuffer[indexPointer] = ',';
	indexPointer++;
	return indexPointer;
}

short PPrintCR(void) 						// appends a newline to the buffer
{
	printBuffer[indexPointer] = '\r';
	indexPointer++;
	printBuffer[indexPointer] = '\n';
	indexPointer++;
	return indexPointer;
}

char* GetBuffer(void) 								// returns the buffer in its current state
{
	return printBuffer;
}

void ClearBuffer(void)
{
	while(indexPointer)
	{
		printBuffer[indexPointer - 1] = 0;
		indexPointer--;
	}
}

void ShortToChar(short passed)
{
	static short modifier;
	static short holder;
	static short place;
	static short divider;
	static short MSD_DETECT;
	static short i;
	MSD_DETECT = 0;
	modifier = passed;
	place = 5;

	if(modifier < 0)
	{
		modifier *= -1;
		printBuffer[indexPointer] = '-';
		indexPointer++;
	}
	while(place)
	{
		divider = 1;
		for(i = 1; i < place; i++)
		{
			divider*=10;
		}
		holder = modifier/divider;
		if((holder != 0) || (MSD_DETECT == 1))
		{
			printBuffer[indexPointer] = 48 + holder;
			indexPointer++;
			MSD_DETECT = 1;
		}

		modifier -= (holder * divider);

		place--;
	}
	if(MSD_DETECT == 0)
	{
		printBuffer[indexPointer] = 48 + 0;
		indexPointer++;
	}
}
