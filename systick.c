
#include "systick.h"
#include "led.h"
#include "uart.h"
#include "LPC1100.h"

static volatile uint32_t sSystick_WaitCalled = 0;
static volatile uint32_t sSystick_WaitCnt = 0;

int Init_Systick(void)
{
	SYST_RVR = 36*1000 - 1;  // 1ms 毎に割り込み発生
	SYST_CVR = 0;
	SYST_CSR = 0x07;

	sSystick_WaitCalled = 0;
	sSystick_WaitCnt = 0;

	return 1;
}

// 引数の時間（ミリ秒）待つ
void Systick_Wait( uint32_t mill_sec )
{
	sSystick_WaitCalled = 1;
	sSystick_WaitCnt = 0;
	while( sSystick_WaitCnt < mill_sec ) ;

	sSystick_WaitCalled = 0;
}

void SysTick_Handler(void)
{
	if( sSystick_WaitCalled ){
		++sSystick_WaitCnt;
	}
}
