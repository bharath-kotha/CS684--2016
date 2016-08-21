#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

#define PWM_FREQUENCY 55

volatile float ui8Adjust = 83;

uint8_t state1 = 0;
uint8_t state2 = 0;

unsigned char detectKeyPress1();
unsigned char detectKeyPress2();


void Timer0IntHandler(void)
{
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	if(detectKeyPress1())
	{
		ui8Adjust+= 3.04;
	}
	else if(detectKeyPress2())
	{
		ui8Adjust -= 3.04;
	}

	if (ui8Adjust < 56)
	{
		ui8Adjust = 56;
	}
	else if (ui8Adjust > 111)
	{
		ui8Adjust = 111;
	}
}


int main(void)
{

	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;

	uint32_t ui32Period;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
	ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

	ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
	ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
	ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	ui32Period = (SysCtlClockGet() / 100) ;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);		// Change here

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1)
	{
		ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);

		SysCtlDelay(100000);
	}
}

unsigned char detectKeyPress2()
{
	unsigned char retFlag = false;
	uint8_t value = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);
	switch(state2)
		{
		case 0:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 1;
				retFlag = false;
			}
			else
			{
				state2 = 0;
				retFlag = false;
			}
			break;
		case 1:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 2;
				retFlag = true;
			}
			else
			{
				state2 = 0;
				retFlag = false;
			}
			break;
		case 2:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 2;
				retFlag = false;
			}
			else
			{
				state2 = 0;
				retFlag = false;
			}
			break;
		}
	return retFlag;
}

unsigned char detectKeyPress1()
{
	unsigned char retFlag = false;
	switch(state1)
		{
		case 0:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 1;
				retFlag = false;
			}
			else
			{
				state1 = 0;
				retFlag = false;
			}
			break;
		case 1:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 2;
				retFlag = true;
			}
			else
			{
				state1 = 0;
				retFlag = false;
			}
			break;
		case 2:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 2;
				retFlag = false;
			}
			else
			{
				state1 = 0;
				retFlag = false;
			}
			break;
		}
	return retFlag;
}
