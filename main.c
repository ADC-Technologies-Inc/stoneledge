#include <stdint.h>
#include <stdio.h>

#include "HW/hal.h"
#include "HW/Interface.h"
#include "prog/prog_conf.h"
#include "memCopy.h"
#include "prog/ctl.h"
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
	CTL_Enter(); 					// initializes program variables and other program-specific functions
}
