
#ifndef LED_H
#define LED_H

#include "stdint.h"

#define LED_COLOR_RED     (1 << 0)
#define LED_COLOR_GREEN   (1 << 1)
#define LED_COLOR_BLUE    (1 << 2)
#define LED_COLOR_YELLOW  (LED_COLOR_RED | LED_COLOR_GREEN)
#define LED_COLOR_AQUA    (LED_COLOR_GREEN | LED_COLOR_BLUE)
#define LED_COLOR_FUCHSIA (LED_COLOR_RED | LED_COLOR_BLUE)
#define LED_COLOR_WHITE   (LED_COLOR_RED | LED_COLOR_GREEN | LED_COLOR_BLUE)

int  Init_LED(void);

void TurnOnLED( uint32_t color );
void TurnOffLED( uint32_t color );

#endif
