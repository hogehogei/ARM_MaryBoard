
#ifndef  SPI_H
#define  SPI_H

#include "stdint.h"

int Init_SPI0( uint8_t bitlen );
int SPI0_Send( uint16_t* data, uint32_t datalen );

#endif
