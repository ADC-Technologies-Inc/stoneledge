/*
 * pprint.h
 *
 *  Created on: Mar 8, 2017
 *      Author: Zack Lyzen
 */

#ifndef HAL_PPRINT_H_
#define HAL_PPRINT_H_

#include "../../HW/pprint/pprintconfig.h"
//#include "../../HW/SerialCommunicationInterface.h"

void  PPrint(void); 								// sends ("prints") the loaded buffer over serial
short PPrintAS(short subject); 						// appends a short to the buffer
short PPrintDL(void); 								// appends a delimiter to the buffer
short PPrintCR(void); 								// appends a newline to the buffer
char* GetBuffer(void); 								// returns the buffer in its current state
void ClearBuffer(void); 							// clears the buffer back to empty

#endif /* HAL_PPRINT_H_ */
