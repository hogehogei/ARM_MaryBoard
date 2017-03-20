
#include "timer32.h"
#include "LPC1100.h"

void (*gTimer32B1_Callback)(void) = 0;

int Init_Timer32B1( uint32_t irq_micro_sec )
{
	// Timer32B1 へのクロック供給を有効
	SYSAHBCLKCTRL |= (1 << 10);
	// ピン設定
	//IOCON_R_PIO1_1 = 0xd3;    // CT32B1_MAT0

	// PCLK 36Mhz
	// PrescaleCounter 360カウントでTCが1増える
	// TC 1μs/count
	TMR32B1PR = 36-1;
	// TMR32B0MR0 を有効
	// マッチしたときにTCを0にリセットする
	// マッチしたときに割り込みを発生させる
	TMR32B1MCR = 0x03;
	// MR0 TimerCounter の上限
	// TCがこの値になったら割り込みが発生する
	TMR32B1MR0 = irq_micro_sec;

	// 割り込みを有効
	__enable_irqn( CT32B1_IRQn );
	// TC/PC をリセットしてカウント開始
	TMR32B1TCR = 0x03;
	TMR32B1TCR &= ~0x02;

	gTimer32B1_Callback = 0;

	return 1;
}

int Timer32B1_SetCallback( void (*callback_func)(void) )
{
	gTimer32B1_Callback = callback_func;

	return 1;
}

void CT32B1_IRQHandler(void)
{
	if( TMR32B1IR & 0x01 ){
		if( gTimer32B1_Callback ){
			(*gTimer32B1_Callback)();
		}
		TMR32B1IR = (1 << 0);
	}
}



