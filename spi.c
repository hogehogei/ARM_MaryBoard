
#include "spi.h"
#include "LPC1100.h"

int Init_SPI0( uint8_t bitlen )
{
	// 1回でやり取りするデータ長
	// 3bit から 16bit まで設定可能
	if( bitlen < 3 || bitlen > 16 ){
		return 0;  
	}

	// SPI0へのクロック供給を有効
	SYSAHBCLKCTRL |= (1 << 11);

	// SCK0 を PIO0_6  に割り当て
	IOCON_SCK0_LOC = 0x2;
	// SPI0のピン設定
	IOCON_PIO0_6 = 0x02;    // SCK0
	IOCON_PIO0_8 = 0x11;    // MISO0 / pullup
	IOCON_PIO0_9 = 0x01;    // MOSI0
	IOCON_PIO0_2 = 0x01;    // SSEL0

	// SPI0をリセット
	PRESETCTRL |= 0x01;
	// SPI0のシステムクロック 36Mhz
	SSP0CLKDIV = 1;
	// SPI0の bit frequency 18Mhz
	//SSP0CR0 |= 0x0000;
	SSP0CPSR = 0x02;

	// 1回でやり取りするデータ長
	SSP0CR0 |= (0x0F & (bitlen-1));
	// SSP0有効化　ここまでにレジスタの設定すませる
	SSP0CR1 |= 0x02;

	return 1;
}

int SPI0_Send( uint16_t* data, uint32_t datalen )
{
	int i = 0;
	for( i = 0; i < datalen; ++i ){
		while( !(SSP0SR & 0x02) );
		SSP0DR = data[i];
	}
	// SPIがデータを送り終わるまで待つ
	while( SSP0SR & 0x10 );

	return 1;
}

