
#ifndef TIMER32_H
#define TIMER32_H

#include "stdint.h"

int Init_Timer32B1( uint32_t irq_micro_sec );
int Timer32B1_SetCallback( void (*callback_func)(void) );

void CT32B1_IRQHandler(void);

#endif
