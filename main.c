#include "LPC1100.h"
#include "led.h"
#include "timer32.h"
#include "uart.h"
#include "ledarray.h"
#include "systick.h"
#include "oled.h"

/* Section boundaries defined in linker script */
extern long _data[], _etext[], _edata[], _bss[], _ebss[];//, _endof_sram[];
extern long _vStackTop[];

static const uint32_t skLEDColorTbl[] = {
		LED_COLOR_RED,
		LED_COLOR_GREEN,
		LED_COLOR_BLUE,
		LED_COLOR_YELLOW,
		LED_COLOR_AQUA,
		LED_COLOR_FUCHSIA,
		LED_COLOR_WHITE,
};

// HELLOWORLD! を表示するためのビットマップ
static const uint8_t skLEDArrayBitmap[8*11] =
{
	0xFF, 0x10, 0x10, 0x10, 0x10, 0x10, 0xFF, 0x00,    // H
	0xFF, 0x91, 0x91, 0x91, 0x91, 0x91, 0x81, 0x00,    // E
	0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,    // L
	0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,    // L
	0x3C, 0x42, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00,    // O
	0x7C, 0x02, 0x01, 0x3E, 0x01, 0x02, 0x7C, 0x00,    // W
	0x3C, 0x42, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00,    // O
	0xFF, 0x88, 0x88, 0x88, 0x88, 0x88, 0x77, 0x00,    // R
	0xFF, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,    // L
	0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7E, 0x00,    // D
	0x00, 0x00, 0x00, 0xFB, 0xFB, 0x00, 0x00, 0x00,    // !
};
static int sLEDArrayDrawIdx = 0;
static int sLEDArrayColorIdx = 0;


// LEDArray表示用ルーチン
// 1列ずつしか点灯できないので、タイマ割り込みで呼び出してダイナミック点灯させる
void drawLEDArray(void)
{
	static uint8_t ledary_row = 0x80;
	static uint8_t ledary_col = 0;
	uint8_t c = skLEDArrayBitmap[sLEDArrayDrawIdx*8 + ledary_col];

	switch( sLEDArrayColorIdx ){
	case 0:
		// Color Red
		DrawLEDArray( ledary_row, c, 0x00 );
		break;
	case 1:
		// Color Green
		DrawLEDArray( ledary_row, 0x00, c );
		break;
	case 2:
		// Color Orange
		DrawLEDArray( ledary_row, c, c );
		break;
	default:
		// ここに来ることはないはず
		break;
	}

	ledary_row <<= 1;
	++ledary_col;
	if( ledary_col == 8 ){
		ledary_row = 0x01;
		ledary_col = 0;
	}
}

//
// エントリポイント
//
int ResetISR (void)
{
	long *d = 0, *s = 0;
	/* Configure BOD control (Reset on Vcc dips below 2.7V) */
	BODCTRL = 0x13;

	/* Configure system clock generator (36MHz system clock with IRC) */
	MAINCLKSEL = 0;							/* Select IRC as main clock */
	MAINCLKUEN = 0; MAINCLKUEN = 1;
	FLASHCFG = (FLASHCFG & 0xFFFFFFFC) | 1;	/* Set wait state for flash memory (1WS) */
#if 1
	SYSPLLCLKSEL = 0;						/* Select IRC for PLL-in */
	SYSPLLCLKUEN = 0; SYSPLLCLKUEN = 1;
	SYSPLLCTRL = (3 - 1) | (2 << 6);		/* Set PLL parameters (M=3, P=4) */
	PDRUNCFG &= ~0x80;						/* Enable PLL */
	while ((SYSPLLSTAT & 1) == 0) ;			/* Wait for PLL locked */
#endif
	SYSAHBCLKDIV = 1;						/* Set system clock divisor (1) */
	MAINCLKSEL = 3;							/* Select PLL-out as main clock */
	MAINCLKUEN = 0; MAINCLKUEN = 1;

	/* Initialize .data/.bss section and static objects get ready to use after this process */
	for (s = _etext, d = _data; d < _edata; *d++ = *s++) ;
	for (d = _bss; d < _ebss; *d++ = 0) ;

	SYSAHBCLKCTRL |= 0x1005F;

	// UARTの初期化
	Init_UART();
	UART_Print( "Init_UART()" );
	// オンボードLEDの初期化
	UART_Print( "Init_LED()" );
	Init_LED();
	// 32bitタイマーの初期化
	Init_Timer32B1( 2000 );  // 2ms毎にコールバック呼び出し
	UART_Print( "Init_Timer32B1()" );
	Timer32B1_SetCallback( drawLEDArray );
	// LEDarray の初期化
	Init_LEDArray();
	UART_Print( "Init_LEDArray()" );
	// Systick Timerの初期化
	Init_Systick();
	UART_Print( "Init Systick()");

	int led_idx = 0;
	TurnOnLED( skLEDColorTbl[led_idx] );

	// main loop
	while(1){
		Systick_Wait( 1000 );
		if( ++sLEDArrayDrawIdx == 11 ){
			sLEDArrayDrawIdx = 0;
			if( ++sLEDArrayColorIdx == 3 ){
				sLEDArrayColorIdx = 0;
			}
		}

		UART_Print( "Hello, world!" );
		TurnOffLED( skLEDColorTbl[led_idx] );
		led_idx = (led_idx + 1) % (sizeof(skLEDColorTbl) / 4);
		TurnOnLED( skLEDColorTbl[led_idx] );
	}
}

/*--------------------------------------------------------------------/
/ Exception Vector Table                                              /
/--------------------------------------------------------------------*/

void trap (void)
{
	TurnOnLED( LED_COLOR_RED );
	for (;;) ;	/* Trap spurious interrupt */
}

void* const vector[] __attribute__ ((section(".isr_vector"))) =	/* Vector table to be allocated to address 0 */
{
	&_vStackTop,//_endof_sram,	/* Reset value of MSP */
	ResetISR,			/* Reset entry */
	trap,//NMI_Handler,
	trap,//HardFault_Hander,
	0, 0, 0, 0, 0, 0, 0,//<Reserved>
	trap,//SVC_Handler,
	0, 0,//<Reserved>
	trap,//PendSV_Handler,
	SysTick_Handler,
	trap,//PIO0_0_IRQHandler,
	trap,//PIO0_1_IRQHandler,
	trap,//PIO0_2_IRQHandler,
	trap,//PIO0_3_IRQHandler,
	trap,//PIO0_4_IRQHandler,
	trap,//PIO0_5_IRQHandler,
	trap,//PIO0_6_IRQHandler,
	trap,//PIO0_7_IRQHandler,
	trap,//PIO0_8_IRQHandler,
	trap,//PIO0_9_IRQHandler,
	trap,//PIO0_10_IRQHandler,
	trap,//PIO0_11_IRQHandler,
	trap,//PIO1_0_IRQHandler,
	trap,//C_CAN_IRQHandler,
	trap,//SPI1_IRQHandler,
	trap,//I2C_IRQHandler,
	trap,//CT16B0_IRQHandler,
	trap,//CT16B1_IRQHandler,
	trap,//CT32B0_IRQHandler,
	CT32B1_IRQHandler,//CT32B1_IRQHandler,
	trap,//SPI0_IRQHandler,
	UART_IRQHandler,  //UART_IRQHandler,
	0, 0,//<Reserved>
	trap,//ADC_IRQHandler,
	trap,//WDT_IRQHandler,
	trap,//BOD_IRQHandler,
	0,//<Reserved>
	trap,//PIO_3_IRQHandler,
	trap,//PIO_2_IRQHandler,
	trap,//PIO_1_IRQHandler,
	trap //PIO_0_IRQHandler
};


