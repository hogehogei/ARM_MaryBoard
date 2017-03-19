
#ifndef LEDARRAY_H
#define LEDARRAY_H

#include "stdint.h"

int Init_LEDArray(void);
void DrawLEDArray( uint8_t row, uint8_t col_red, uint8_t col_green );

#endif
