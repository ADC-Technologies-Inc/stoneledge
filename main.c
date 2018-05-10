#include <stdint.h>
#include <stdio.h>

#include "HW/hal.h"
#include "HW/Interface.h"
#include "prog/prog_conf.h"
#include "prog/adct.h"
#include "memCopy.h"
#include "prog/time.h"
#include "prog/lcd.h"

void Pause()
{
	while (!IsRealTimeClockTriggered());
	UnTriggerRealTimeClock();
}

int main(void)
{
	memCopy((unsigned int *) &RamfuncsLoadStart,
			(unsigned int *) &RamfuncsLoadEnd,
			(unsigned int *) &RamfuncsRunStart);

	InitializeHardware(); 					// initializes clocks, gpio, and peripherials to safe states
	InitializeProgram(); 					// initializes program variables and other program-specific functions

	while (1)
	{

#ifdef USE_LCD 								// if use LCD is selected
			LcdService(); 					// checks LCD to see if display needs to be updated
#endif /* USE_LCD */
#ifdef USE_ETHERNET 						// if use Ethernet is selected
			ServiceEthernet(); 				// service ethernet every 1ms - though sends every 500ms
#endif /* USE_ETHERNET*/

		Pause(); 						// while loop shall take 1ms if 1ms hasn't been completed yet (timer starts at beginning of while loop)
										// then wait here for timer to complete
	}
}
