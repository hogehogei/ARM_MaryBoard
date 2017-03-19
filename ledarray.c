
#include "ledarray.h"
#include "LPC1100.h"
#include "spi.h"

int Init_LEDArray(void)
{
	// GPIOへのクロック供給を有効に
	SYSAHBCLKCTRL |= (1 << 6);

	// GPIOを使うようにピンを設定
	IOCON_PIO1_8 = 0xd0;
	IOCON_PIO1_9 = 0xd0;

	// PIO1_9 をoutputに設定
	GPIO1DIR |= _BV(9);
	GPIO1DATA &= ~_BV(9);
	// LEDモジュールをリセット
	GPIO1DIR |= _BV(8);
	GPIO1DATA &= ~_BV(8);
	GPIO1DATA |= _BV(8);

	// SPI0 の初期化
	Init_SPI0( 8 );  // 8bit

	return 1;
}

void DrawLEDArray( uint8_t row, uint8_t col_red, uint8_t col_green )
{
	uint16_t data[3] = { row, col_red, col_green };
	SPI0_Send( data, 3 );

	// ラッチ信号
	GPIO1DATA |= _BV(9);
	GPIO1DATA &= ~_BV(9);
}


