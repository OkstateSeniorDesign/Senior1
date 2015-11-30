#include "SPI.h"

SPI::SPI(void){
	initMSPIM();
	initSlaveSelect();
}

/************************************************************************/
//Pre Nothing
//Post MSPIM will be set up at 2 MHz
/************************************************************************/
void SPI::initMSPIM(){

	UBRR1 = 0;//Stop Uart
	
	DDRD |= (1<<PIND4);//Set XCK1 to output.
	
	
	UCSR1C =0b11000000;//set to MSPIM MSB first
	
	
	UCSR1B = (1<<RXEN1)|(1<<TXEN1);//Enable MISO and MOSI
	
	UBRR1 = 3;//Set baud rate. Must be done afer enabled above//normaly 0
	
}
void SPI::bitBangAudioSender(bool toBitBang){
	
	DDRA|=0b00000111;
	
	if(toBitBang>0){
		
		PORTA&=0b11111110; //reset
		_delay_us(200000);
		PORTA|=0b00000111;//none
		_delay_us(200000);
		PORTA&=0b11111101 ;//play
		_delay_us(600000);
		PORTA|=0b00000111;//none
		}
	else{
		PORTA&=0b11111110 ;//reset
		_delay_us(200000);
		PORTA|=0b00000111;//none
		_delay_us(200000);
		PORTA&=0b11111011; //next
		_delay_us(600000);
		PORTA|=0b00000111;//none
	}

	
	#undef pulseTime
}
/************************************************************************/
//Pre MSPIM was inited
//Post a byte of data will be written out
/************************************************************************/
uint8_t SPI::sendSPI(uint8_t data){
	// wait for transmitter ready
	while ((UCSR1A & 1<< (UDRE1)) == 0)	{};//Wait until last data was finished
	
	// send byte
	UDR1 = data;
	
	// wait for receiver ready
	while ((UCSR1A & 1<< (RXC1)) == 0)
	{};
	
	// receive byte, return it
	return UDR1;
}  // end of sendSPI

uint8_t SPI::sendLSBSPI(uint8_t data){
	//Dont ask, dont tell
	return BitReverseTable256[sendSPI(BitReverseTable256[data])];
	
	
}
//Sets the slave
void SPI::pickASlave(SLAVE s){
	while ((UCSR1A & 1<< (UDRE1)) == 0)	{};//Wait until last data was finished
	PORTC=s;
	
}
/************************************************************************/
//Pre nothing
//Post PORTC will be set as output and all the slaves will be toggled once
/************************************************************************/
void SPI::initSlaveSelect(){
	DDRC|=(1<<PINC0)|(1<<PINC2)|(1<<PINC2)|(1<<PINC3)|(1<<PINC4)|(1<<PINC5)|(1<<PINC6)|(1<<PINC7);//Set PORTC as outputs
	/*pickASlave(audio);
	_delay_ms(500);
	pickASlave(nfc);
	_delay_ms(500);
	pickASlave(strikeDetector);
	_delay_ms(500);
	pickASlave(weaponIndicatorMatrix);
	_delay_ms(500);
	pickASlave(hitIndicatorMatrix);
	_delay_ms(500);
	pickASlave(none);*/
}