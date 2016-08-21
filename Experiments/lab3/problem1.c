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

#define LONG_COUNT_TIMEOUT 200

uint8_t longCountSW1 = 0;
uint8_t longCountSW2 = 0;

uint8_t mode = 0;

uint32_t red = 240;
uint32_t green = 0;
uint32_t blue = 0;

uint32_t angle = 0;
uint32_t incAngle = 1;

uint8_t state1 = 0;
uint8_t state2 = 0;

void changeColor();
unsigned int detectKeyPress1();
unsigned int detectKeyPress2();

void Timer0IntHandler(void) {
// Clear the timer interrupt
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
// Read the current state of the GPIO pin and
// write back the opposite state
	bool longPressx = false;
	unsigned int keyPress = detectKeyPress1();
	if(keyPress == 0x01)
	{
		incAngle++;
	}
	else if(detectKeyPress2())
	{
		incAngle--;
	}
	else if (keyPress == 0x10)
	{
		longPressx = true;
	}

	if(incAngle < 1)
	{
		incAngle =1 ;
	}
	else if(incAngle > 10)
	{
		incAngle = 10;
	}

}


int main(void)
{

	volatile uint32_t ui32Load;
	volatile uint32_t ui32PWMClock;
	uint32_t ui32Period;

	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
	SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
	ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);
	ROM_GPIOPinConfigure(GPIO_PF2_M1PWM6);
	ROM_GPIOPinConfigure(GPIO_PF3_M1PWM7);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	ui32PWMClock = SysCtlClockGet() / 64;
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
	PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
	PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, red * ui32Load / 240);		// RED
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, green * ui32Load / 240);		// GREEN
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, blue * ui32Load / 240);		// BLUE

	PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_2);
	PWMGenEnable(PWM1_BASE, PWM_GEN_3);

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
		if(mode == 0)
		{
			angle = angle + incAngle;
			changeColor();
		}

		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, red * ui32Load / 240);		// RED
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, green * ui32Load / 240);		// GREEN
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, blue * ui32Load / 240);		// BLUE

		SysCtlDelay(1000000);
	}
}

void changeColor()
{
	if(angle > 360)
	{
		angle = 0;
	}

	if((int) angle > 0 && (int) angle <= 120)
	{
		red = 240 - 2 * angle;
		blue = 0;
		green = 2 * angle;
	}
	else if(angle > 120 && angle < 240)
	{
		red = 0;
		green = 480 - 2 * angle;
		blue = 2 * angle - 240;
	}

	else
	{
		red = 2 * angle - 480;
		green = 0;
		blue = 720 - 2 * angle;
	}
}

unsigned int detectKeyPress2()
{
	unsigned int retFlag = false;
	uint8_t value = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);
	switch(state2)
		{
		case 0:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 1;
				retFlag = 0x00;
				longCountSW2 = 0;
			}
			else
			{
				state2 = 0;
				retFlag = 0x00;
				longCountSW2 = 0;
			}
			break;
		case 1:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 2;
				retFlag = 0x01;
				longCountSW2 = 0;

			}
			else
			{
				state2 = 0;
				retFlag = 0x00;
				longCountSW2 = 0;
			}
			break;
		case 2:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)&0x01)!=0x01)
			{
				state2 = 2;
				retFlag = 0x00;
				longCountSW2++;
				if(longCountSW2 > LONG_COUNT_TIMEOUT)
				{
					retFlag = 0x10;
				}
			}
			else
			{
				state2 = 0;
				retFlag = 0x00;
				longCountSW2 = 0;
			}
			break;
		}
	return retFlag;
}

unsigned int detectKeyPress1()
{
	unsigned int retFlag = 0x00;
	switch(state1)
		{
		case 0:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 1;
				retFlag = 0x00;
				longCountSW1 = 0;
			}
			else
			{
				state1 = 0;
				retFlag = 0x00;
				longCountSW1 = 0;
			}
			break;
		case 1:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 2;
				retFlag = 0x01;
				longCountSW1 = 0;
			}
			else
			{
				state1 = 0;
				retFlag = 0x00;
				longCountSW1 = 0;
			}
			break;
		case 2:
			if((GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)&0x10)!=0x10)
			{
				state1 = 2;
				retFlag = 0x00;
				longCountSW1++;
				if(longCountSW1 > LONG_COUNT_TIMEOUT)
				{
					retFlag = 0x10;
				}
			}
			else
			{
				state1 = 0;
				retFlag = 0x00;
				longCountSW1 = 0;
			}
			break;
		}
	return retFlag;
}
