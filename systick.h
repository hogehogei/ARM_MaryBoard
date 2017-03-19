
#ifndef SYSTICK_H
#define SYSTICK_H

#include "stdint.h"


int Init_Systick(void);
void Systick_Wait( uint32_t mill_sec );

void SysTick_Handler(void);

#endif
