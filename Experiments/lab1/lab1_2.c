#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

uint8_t sw2Status = 0;

void onButtonDown();
void onButtonUp();

void onButtonDown(void) {
    if (GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0) {
        // PF4 was interrupt cause
    	sw2Status++;
        GPIOIntRegister(GPIO_PORTF_BASE, onButtonUp);   // Register our handler function for port F
        GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0,
        		GPIO_RISING_EDGE);          // Configure PF4 for rising edge trigger
        GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);  // Clear interrupt flag
    }

}

void onButtonUp(void) {
    if (GPIOIntStatus(GPIO_PORTF_BASE, false) & GPIO_PIN_0) {
        // PF4 was interrupt cause
    	//GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        GPIOIntRegister(GPIO_PORTF_BASE, onButtonDown); // Register our handler function for port F
        GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0,
            GPIO_FALLING_EDGE);         // Configure PF4 for falling edge trigger
        GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);  // Clear interrupt flag
    }
}


int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4,
	        GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4

	GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_0);        // Disable interrupt for PF4 (in case it was enabled)
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);      // Clear pending interrupts for PF4
	GPIOIntRegister(GPIO_PORTF_BASE, onButtonDown);     // Register our handler function for port F
	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0,
			GPIO_FALLING_EDGE);             // Configure PF4 for falling edge trigger
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_PIN_0);     // Enable interrupt for PF4

	while(1){

	}

}
