#include "UART.h"

UART::UART(void){
	
}

//Speed of the occilator
#ifndef FOSC
#define	FOSC			16000000UL
#endif

//Macros from AVR for UART adapted to fit this MCU
/* Settings */
#define _BAUD			19200					// Baud rate
#define _DATA			0x03					// Number of data bits in frame = byte tranmission
#define _UBRR			(FOSC/16)/_BAUD - 1		// Used for UBRRL and UBRRH
/* Useful macros */
#define TX_START()		UCSR0B |= 1<<(TXEN0)	// Enable TX
#define TX_STOP()		UCSR0B &= ~1<<(TXEN0)	// Disable TX
#define RX_START()		UCSR0B |= 1<<(RXEN0)	// Enable RX
#define RX_STOP()		UCSR0B &= ~1<<(RXEN0)	// Disable RX
#define COMM_START()		TX_START(); RX_START()	// Enable communications

/* Interrupt macros; Remember to set the GIE bit in SREG before using (see datasheet) */
#define RX_INTEN()		UCSR0B |= 1<<(RXCIE0)	// Enable interrupt on RX complete
#define RX_INTDIS()		UCSR0B &= ~1<<(RXCIE0)	// Disable RX interrupt
#define TX_INTEN()		UCSR0B |= 1<<(TXCIE0)	// Enable interrupt on TX complete
#define TX_INTDIS()		UCSR0B &= ~1<<(TXCIE0)	// Disable TX interrupt

/************************************************************************/
//Pre Nothing
//Post UART1 will be enabled

/************************************************************************/
void UART::initUART0(void)
{

	DDRD |= 1<<(PD0);//set TX as output
	DDRD &= ~1<<(PD1);//set RX as input

	// Set baud rate; lower byte and top nibble
	UBRR0H = ((_UBRR) & 0xF00);
	UBRR0L = (uint8_t) ((_UBRR) & 0xFF);

	//Start Coms
	TX_START();
	RX_START();
	//RX_INTEN();
	// Set frame format = 8-N-1
	UCSR0C = (_DATA << UCSZ00);

}
/************************************************************************/
//Pre Uart must be inited
//Post data passed in is wrote out.
/************************************************************************/
void UART::putByte(unsigned char data)
{
	
	while (!(UCSR0A & 1<<(UDRE0)));//Wait for last transmission to compleat
	UDR0 = (unsigned char) data;

}

char UART::getByte()
{
	//Wait untill a data is available

	while(!(UCSR0A & (1<<UDRE0)));

	return UDR0;
}