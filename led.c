
#include "led.h"
#include "LPC1100.h"

int Init_LED(void)
{
	SYSAHBCLKCTRL |= (1 << 6);

	GPIO2DIR |= _BV(0);  // RED
	GPIO1DIR |= _BV(5);  // BLUE
	GPIO0DIR |= _BV(7);  // GREEN

	return 1;
}

void TurnOnLED( uint32_t color )
{
	if( color & LED_COLOR_RED ){
		GPIO2DATA &= ~_BV(0);
	}
	if( color & LED_COLOR_BLUE ){
		GPIO1DATA &= ~_BV(5);
	}
	if( color & LED_COLOR_GREEN ){
		GPIO0DATA &= ~_BV(7);
	}
}

void TurnOffLED( uint32_t color )
{
	if( color & LED_COLOR_RED ){
		GPIO2DATA |= _BV(0);
	}
	if( color & LED_COLOR_BLUE ){
		GPIO1DATA |= _BV(5);
	}
	if( color & LED_COLOR_GREEN ){
		GPIO0DATA |= _BV(7);
	}
}
