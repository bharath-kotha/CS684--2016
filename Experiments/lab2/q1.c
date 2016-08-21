#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

uint8_t state = 0;
uint8_t ui8LED = 0x02;
uint8_t watch1 = 0;

unsigned char detectKeyPress();

unsigned char detectKeyPress()
{
	unsigned char retFlag = false;
	switch(state)
		{
		case 0:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state = 1;
				retFlag = false;
			}
			else
			{
				state = 0;
				retFlag = false;
			}
			break;
		case 1:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state = 2;
				retFlag = true;
			}
			else
			{
				state = 0;
				retFlag = false;
			}
			break;
		case 2:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state = 2;
				retFlag = false;
			}
			else
			{
				state = 0;
				retFlag = false;
			}
			break;
		}
	return retFlag;
}

void Timer0IntHandler(void) {
// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
// Read the current state of the GPIO pin and
// write back the opposite state
	if(detectKeyPress())
	{
		if (ui8LED == 8)
		{
			ui8LED = 2;
		}
		else
		{
			ui8LED = ui8LED*2;
		}
	}

}

int main(void) {
	uint32_t ui32Period;

	SysCtlClockSet(
			SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ
					| SYSCTL_OSC_MAIN);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,
			GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_4);

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4 |GPIO_PIN_0,
		        GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	ui32Period = (SysCtlClockGet() / 100) ;
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period - 1);		// Change here

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);

	while (1) {
		GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,ui8LED);
		watch1 = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4);
	}
}

