
#ifndef HW_HAL_H_
#define HW_HAL_H_

void InitializeHardware(void);
char IsRealTimeClockTriggered();
void UnTriggerRealTimeClock();

void DisableGlobalInterrupts();
void EnableGlobalInterrupts();

#endif /* HW_HAL_H_ */
