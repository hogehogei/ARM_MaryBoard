
#include "uart.h"
#include "LPC1100.h"
#include "led.h"

#define UART_BUFSIZE (128)
#define UART_RECV_FIFO_EMPTY (0)
#define UART_RECV_OK (1)
#define UART_RECV_BUFFULL (2)

volatile uint8_t gUART_RxBuffer[UART_BUFSIZE] = {0};
volatile uint8_t gUART_TxBuffer[UART_BUFSIZE] = {0};
volatile int gUART_RxBufIdx = 0;
volatile int gUART_RxBufBegin = 0;
volatile int gUART_TxBufIdx = 0;
volatile int gUART_TxBufBegin = 0;

int Init_UART(void)
{
	IOCON_PIO1_6 = 0xd1;
	IOCON_PIO1_7 = 0xd1;
	GPIO1DIR |= _BV(7);

	// UART へのクロックは 36Mhz/3 = 12Mhz
	UARTCLKDIV = 3;
	// UART へのクロック供給を有効
	SYSAHBCLKCTRL |= (1 << 12);

	// enable latch access bit
	U0LCR = 0x83;

	// UARTの設定
	// baudrate 115200
	// PCLK = 12MHz
	// DLL = 4, DLM = 0, MULVAL = 8, DIVADDVAL = 5
	U0DLL = 4;
	U0DLM = 0;
	U0FDR = (8 << 4) | 5;

	// 8bit char length
	// 1 stop bit
	// non parity
	// non break control
	// disable latch access bit
	U0LCR = 0x03;
	// Enable receive data available interrupt
	// Enable THRE interrupt
	U0IER = 0x07;
	// Rx/Tx FIFO Enable
	U0FCR |= 0x81;
	// 割り込み発生 Rx trigger level2 に設定(8 characters)
	U0FCR |= (2 << 6);
	U0TER = 0x80;

	__enable_irqn( UART_IRQn );

	gUART_TxBufIdx = 0;
	gUART_TxBufBegin = 0;
	gUART_RxBufIdx = 0;
	gUART_RxBufBegin = 0;

	return 1;
}

static int UART_RecvByte(void)
{
	if( !(U0LSR & 0x01) ){
		return UART_RECV_FIFO_EMPTY;
	}

	int result = 0;
	if( (gUART_RxBufIdx + 1) % UART_BUFSIZE != gUART_RxBufBegin ){
		__disable_irq();
		gUART_RxBuffer[gUART_RxBufIdx] = U0RBR;
		gUART_RxBufIdx = (gUART_RxBufIdx + 1) % UART_BUFSIZE;
		__enable_irq();

		result = UART_RECV_OK;
	}
	else {
		//　読み取りバッファが一杯
		result = UART_RECV_BUFFULL;
	}

	return result;
}

static void UART_MoveRxFIFODataToRxBuf(void)
{
	int result;
	uint8_t temp;

	// Rx FIFOバッファにあるデータを全部読む
	while( (result = UART_RecvByte()) != UART_RECV_FIFO_EMPTY ) {
		// 読み取りバッファいっぱいならしょうがないので読み捨てる
		if( result == UART_RECV_BUFFULL ){
			temp = U0RBR;
			break;
		}
	}
}

static int UART_SendByte(void)
{
	if( gUART_TxBufBegin == gUART_TxBufIdx ){
		return 0;
	}

	while( !(U0LSR & (1 << 5)) ) ;
	__disable_irq();
	U0THR = gUART_TxBuffer[gUART_TxBufBegin];
	gUART_TxBufBegin = (gUART_TxBufBegin + 1) % UART_BUFSIZE;
	__enable_irq();

	return 1;
}


int UART_Recv( uint8_t* rx, uint32_t len )
{
	UART_MoveRxFIFODataToRxBuf();

	int rxcnt = 0;
	while( rxcnt < len && gUART_RxBufBegin != gUART_RxBufIdx ){
		__disable_irq();
		rx[rxcnt] = gUART_RxBuffer[gUART_RxBufBegin];
		gUART_RxBufBegin = (gUART_RxBufBegin + 1) % UART_BUFSIZE;
		__enable_irq();

		++rxcnt;
	}

	return rxcnt;
}


int UART_Send( const uint8_t* tx, uint32_t len )
{
	int i = 0;
	for( i = 0; i < len; ++i ){
		//バッファが一杯なら空きがでるまで待つ
		while( ((gUART_TxBufIdx + 1) % UART_BUFSIZE) == gUART_TxBufBegin ) {
			//if( U0LSR & (1 << 6) ){
			//	UART_SendByte();
			//}
		}

		__disable_irq();
		gUART_TxBuffer[gUART_TxBufIdx] = tx[i];
		gUART_TxBufIdx = (gUART_TxBufIdx + 1) % UART_BUFSIZE;
		__enable_irq();

		// Transmitter（U0THRとU0TSR)が空ならとりあえず1バイト送っておく
		// 何かバイトを送って、割り込みを発生させるため
		if( U0LSR & (1 << 6) ){
			UART_SendByte();
		}
	}

	return len;
}

void UART_ClearRecvBuffer(void)
{
	__disable_irq();
	gUART_RxBufIdx = 0;
	gUART_RxBufBegin = 0;
	volatile uint8_t d;
	while( U0LSR & 0x01 ){
		d = U0RBR;
	}
	__enable_irq();
}

int UART_IsPresentRecvData(void)
{
	int is_rxbuf_present_data = (gUART_RxBufBegin != gUART_RxBufIdx);
	int is_rxfifo_present_data = (U0LSR & 0x01);
	return is_rxbuf_present_data || is_rxfifo_present_data;
}

int UART_Print( const char* str )
{
	uint32_t len = 0;
	while( str[len] != '\0' ){
		++len;
	}

	int result = UART_Send( (const uint8_t*)str, len );
	// windows の改行コードは CRLF
	UART_Send( (const uint8_t*)"\r\n", 2 );

	return result;
}

void UART_IRQHandler(void)
{
	while(1){
		uint32_t int_flag = U0IIR;
		// 割り込みが残ってないなら抜ける
		if( (int_flag & 0x01) ){
			break;
		}

		uint32_t t = 0;
		switch( int_flag & 0x0E ){
		case 2:    // THER
			// 送信バッファに残っているデータをすべて送る
			while( UART_SendByte() ) ;
			break;
		case 4:   // Recieve Data Available
			UART_MoveRxFIFODataToRxBuf();
			break;
		default:  // Rx FIFO Error
			// エラーをクリア
			t = U0LSR;
			// U0RBR の中身を読み捨てる
			t = U0RBR;
			break;
		}
	}
}




