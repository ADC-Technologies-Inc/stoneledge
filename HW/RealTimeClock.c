#include "../HW/RealTimeClock.h"

#include "DSP2803x_Device.h"

__interrupt void cpu_timer0_isr(void);

struct _realTimeClock {
	struct CPUTIMER_VARS* CpuTimerVars;
	volatile struct CPUTIMER_REGS * CpuTimerRegs;
	void (*Interrupt)(void);
	Uint16  latched:1;
};

RealTimeClock RTC0;

char IsRealTimeClockTriggered() {
	return RTC0.latched;
}

void UnTriggerRealTimeClock() {
	DINT;
	RTC0.latched = 0;
	EINT;
}

InitializeRealTimeClock(RealTimeClock* RTC, struct CPUTIMER_VARS* CpuTimer, volatile struct CPUTIMER_REGS* CpuTimerRegs, void (*callBack)(void)) {
	RTC->CpuTimerRegs = CpuTimerRegs;
	RTC->CpuTimerVars = CpuTimer;
	RTC->Interrupt = callBack;
	RTC->latched = 0;
}

void InitTimer0() {
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
	IER |= M_INT1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

void ConfigureRealTimeClockInterrupt(RealTimeClock* RTC) {
	EALLOW;  // This is needed to write to EALLOW protected registers
	PieVectTable.TINT0 = RTC->Interrupt;
	EDIS;    // This is needed to disable write to EALLOW protected registers
}

void InitializeRealTimeClocks(void) {
	InitializeRealTimeClock(&RTC0, &CpuTimer0, &CpuTimer0Regs, cpu_timer0_isr);
	ConfigureRealTimeClockInterrupt(&RTC0);
	InitCpuTimers();

	ConfigCpuTimer(RTC0.CpuTimerVars, 60, 100000);
	InitTimer0();
}

__interrupt void cpu_timer0_isr(void){
	CpuTimer0.InterruptCount++;
	RTC0.latched = 1;
	// Acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
