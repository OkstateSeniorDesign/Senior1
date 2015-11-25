/*
 * SeniorDesignProject.cpp
 *
 * Created: 10/25/2015 9:09:30 PM
 * Author : mattP_000
 */ 
#define F_CPU 16000000UL

#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "UART.h"
#include "Timer0.h"
#include "Timer1.h"
#include "SPI.h"
#include "WeaponMatrix.h"
#include "NFC.h"
#include "HitMatrix.h"
#include <string.h>

UART debugUART;
Timer0 timer0;
Timer1 timer1;
SPI spi;
WeaponMatrix weaponMatrix;
NFC nfc;
HitMatrix hitMatrix;
char * weaponName="Team 6";
volatile uint8_t posCounter=0;
volatile uint8_t otherCounter=0;
volatile uint8_t otherOtherCounter=0;

uint8_t readRegister(uint8_t thisRegister) {
	uint8_t result = 0;   // result to return
	spi.pickASlave(strikeDetector);
	spi.sendSPI(thisRegister | 0x80);
	result=spi.sendSPI(0x00);
	spi.pickASlave(none);
	return (result);
}

uint8_t writeRegister(uint8_t thisRegister, uint8_t byteToWrite) {
	
	uint8_t result = 0;   // result to return
	spi.pickASlave(strikeDetector);
	spi.sendSPI(thisRegister);
	result =spi.sendSPI(byteToWrite);
	spi.pickASlave(none);
	return (result);
}

char * lookUpYourMomsItem(uint32_t MomsItem){
	const uint32_t MysticPuzzle=0xB28D2004;
	const uint32_t Ebow=0xB28D2004;
	if(MomsItem==MysticPuzzle){
		return "Mystic Wires";
	}else if(MomsItem==Ebow){
		return "E-NERF Bow";
	}else{
		return "Team 6";
	}
}

void playFlames(){
	DDRA=0b00000111;
	for (uint16_t i =0; i<65000;i++)
	{
			PORTA=0b0000111;
			_delay_us(10);
			PORTA=0b0000000;
			_delay_us(90);
	}
	for (uint16_t i=0; i<10000;i++)
	{
		PORTA=0b0000111;
		_delay_us(50);
		PORTA=0b0000000;
		_delay_us(50);
	}
	PORTA=0b0000111;
	debugUART<<"Flames Played"<<endl;
}

void initTimer2(){
	
}
volatile bool accel=false;
volatile bool data=true;
int main(void)
{
	_delay_ms(500);
	DDRB |= (1<<PINB0);
	PORTB|= (1<<PINB0);
	_delay_ms(500);
	PORTB&= ~(1<<PINB0);
	_delay_ms(500);
	PORTB|= (1<<PINB0);
	
	debugUART<<"Welcome"<<endl;
	weaponMatrix.initMatrix(spi);
	
	playFlames();
	
	debugUART.initUART0();
	debugUART<<"About to INIT weaponMatrix"<<endl;
	

	debugUART<<"weaponMatrix was Inited"<<endl;
	uint8_t counter=0;
	timer0.initIRLED();
	timer0.initTimer0();
	timer1.initTimer1();
		DDRB&=~(1<<PINB2||1<<PINB1);
	  EICRA|=((1<<ISC21)|(1<<ISC20));
	  EIMSK|=(1<<INT2);

   
   weaponMatrix.newString(lookUpYourMomsItem(0xB28D2004));
   debugUART<<"New String Set"<<endl;
   /*
   for(int i=0;i<15*5;i++){
	for(int j=0; j<8;++j){
		debugUART<<((weaponMatrix.buffA[i]&(1<<(7-j)))==0?' ':'X'+' ');
	}
	debugUART<<endl;
   }*/
   /*
	uint8_t response[20];
	
	unsigned char getReadyForRFID[6]={0xFF,0x00,0x01,0x82,0x83};
	
	for(uint8_t i=0;i<6;i++){
		debugUART.putByte(getReadyForRFID[i]);
		_delay_us(200);
	}
	for(uint8_t i=0;i<6;i++){
		response[i]=debugUART.getByte();
	}
	for(uint8_t i=0;i<6;i++){
			for(int j=0; j<8;++j){
				debugUART<<((response[i]&(1<<(7-j)))==0?'1':'0');
			}
			debugUART<<endl;
	}
	debugUART<<"Done turning on NFC"<<endl;*/
	_delay_ms(500);	
	timer0.sendMIRPS(20,true);
		_delay_ms(500);
		timer0.sendMIRPS(20,true);
			_delay_ms(500);
			timer0.sendMIRPS(20,true);
				_delay_ms(500);
				timer0.sendMIRPS(20,true);
					_delay_ms(500);
					timer0.sendMIRPS(20,true);
						_delay_ms(500);
						timer0.sendMIRPS(20,true);
   while(1){
	  if(accel){ 
		if((posCounter%8)==3||(posCounter%8)==4){
			timer0.sendMIRPS(20,true);
			spi.bitBangAudioSender(true);
		}else{
			timer0.sendMIRPS(20,false);
			spi.bitBangAudioSender(false);
			}
		data=!data;
		accel=false;
		}
		

   }

 
	return 0;
}
ISR(TIMER0_COMPA_vect){
	timer0.MIRPSISR();

	//hitMatrix.outputMatrix();
	//debugUART<<"MIRPSISR"<<endl;
}

ISR(TIMER1_OVF_vect)
{
	weaponMatrix.writeToMatrix();
	weaponMatrix.incStartingLocation();
	
	hitMatrix.moveBar();
		spi.pickASlave(hitIndicatorMatrix);
		spi.sendSPI(((posCounter%8)>4||(posCounter%8)<3)?0b00000000:0b11111111);
		spi.sendSPI(((posCounter%8)>4||(posCounter%8)<3)?0b11111111:0b00000000);
		spi.sendSPI(~(1<<(posCounter%8)));
		spi.pickASlave(none);
	
	if(otherOtherCounter==0){
		otherCounter++;
		if((otherCounter&0x04)>0)
		posCounter++;
	}else
		otherOtherCounter--;
}

ISR (INT2_vect)
{
	//cli();
	//_delay_ms(10);
	
	if(PINB & (1<<PINB1)){
		accel=true;
		debugUART<<"IR2 Latched High"<<endl;
		debugUART<<"It was Accelerometer"<<endl;
		otherOtherCounter=21;
	}
	
	return;
	}
	uint8_t uartData;
	ISR(USART0_RX_vect)
	{
		debugUART<<UDR0<<endl;
	}
#include "ISR.Impl.h"