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

HitMatrix hitMatrix;

const uint8_t maxStringLength=20;
volatile uint8_t otherOtherCounter=0;
volatile bool readUart=false;
volatile uint8_t lookUpIndex=0;
volatile uint8_t weaponHealth=0;


char * lookUpYourMomsItem(){

	const uint8_t MysticPuzzle=0x95;
	const uint8_t Ebow=0x92;
	const uint8_t MOAB=0x8E;
	const uint8_t OmegaBlade=0x8F;
	const uint8_t WatchDog=0x93;
	const uint8_t Matt=0x8D;
	const uint8_t Nathan=0x78;
	const uint8_t DrR=0x71;
	const uint8_t Lucas=0x7B;
	const uint8_t MattR=0x7C;
	const uint8_t Cherwa=0x7A;
	
	if(lookUpIndex==MysticPuzzle){
		return "Mystic Wires         ";
	}else if(lookUpIndex==Ebow){
		return "E-NERF Bow           ";
	}else if(lookUpIndex==MOAB){
		return "MOA Easy Tasks       ";
	}else if(lookUpIndex==OmegaBlade){
		return "Omega Regulators     ";
	}else if(lookUpIndex==WatchDog){
		return "Sucks 2 b Wall-E     ";
	}else if(lookUpIndex==Matt){
		return "Matt Litchfield      ";	
	}else if(lookUpIndex==Nathan){
		return "Nathan Lea           ";
	}else if(lookUpIndex==DrR){
		return "Dr. Rajamani         ";
	}else if(lookUpIndex==Lucas){
		return "You all Suck         ";
	}else if(lookUpIndex==MattR){
		return "I hate code          ";
	}else if(lookUpIndex==Cherwa){
		return "oh yeah, do it       ";
	}else{
		return "Welcome              ";
	}
}
void sendNFCInit(){
		
		while (!(UCSR0A & 1<<(UDRE0)));
		UDR0=0xFF;
		while (!(UCSR0A & 1<<(UDRE0)));
		UDR0=0x00;
		while (!(UCSR0A & 1<<(UDRE0)));
		UDR0=0x01;
		while (!(UCSR0A & 1<<(UDRE0)));
		UDR0=0x82;
		while (!(UCSR0A & 1<<(UDRE0)));
		UDR0=0x83;
		
}

char * reamAStringForHP(char * str, uint8_t hp,uint8_t length){
	str[length-1]='p';
	str[length-2]='h';
	str[length-3]=(hp%10)+48;
	hp=hp/10;
	str[length-4]=(hp%10==0&&hp<10)?' ':(hp%10)+48;
	hp=hp/10;
	str[length-5]=(hp%10==0)?' ':(hp%10)+48;
	return str;
}
void playFlames(uint8_t flameLevel){
	DDRA=0b11100000;
	if(flameLevel==0){//Flames
		PORTA&=0b00011111;
	}else if(flameLevel==1){//sadFace
		PORTA|=0b11000000;
	}else if (flameLevel==2)//HappyFace
	{
		PORTA|=0b00100000;
	}else{
		PORTA&=0b00011111;
	}
	
	
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
	
	weaponMatrix.initMatrix(spi);
	
	
	playFlames(0);
	debugUART.initUART0();
	
	

	
	uint8_t counter=0;
	timer0.initIRLED();
	timer0.initTimer0();
	timer1.initTimer1();
		DDRB&=~(1<<PINB2||1<<PINB1);
	  EICRA|=((1<<ISC21)|(1<<ISC20));
	  EIMSK|=(1<<INT2);

	sendNFCInit();
   weaponMatrix.newString("Welcome");
   


	uint8_t counter2=0;
   while(1){
	  if(accel){ 
			if(hitMatrix.wasGoodHit()){
				if(weaponHealth<90)
					weaponHealth+=10;
				else
					weaponHealth=100;
				timer0.sendMIRPS(10,true);
				spi.bitBangAudioSender(true);
				playFlames(2);
			}else{
				if(weaponHealth>9)
					weaponHealth-=10;
				else
					weaponHealth=0;
				timer0.sendMIRPS(10,false);
				spi.bitBangAudioSender(false);
				playFlames(1);
			}
			
		weaponMatrix.newString(reamAStringForHP(lookUpYourMomsItem(),weaponHealth,maxStringLength));
		accel=false;
		
		}
		if(readUart){
			weaponHealth=0;
			weaponMatrix.newString(reamAStringForHP(lookUpYourMomsItem(),weaponHealth,maxStringLength));
			readUart=false;
			playFlames(0);

		}
		if(counter==0);
			sendNFCInit();
		counter2=counter2++;
   }

 
	return 0;
}
ISR(TIMER0_COMPA_vect){
	timer0.MIRPSISR();
}

ISR(TIMER1_OVF_vect)
{
	weaponMatrix.writeToMatrix();
	weaponMatrix.incStartingLocation();

	if(otherOtherCounter!=0)
		--otherOtherCounter;
	else{
			hitMatrix.outputMatrix();
			hitMatrix.moveBar();
	}
}

ISR (INT2_vect)
{
		if(otherOtherCounter==0){
			accel=true;
			hitMatrix.wasHit();
			otherOtherCounter=63;
		}
	return;
}
volatile uint8_t uartData=0;
volatile uint16_t uartDelayCounter=0;
ISR(USART0_RX_vect)
{
	uartData=UDR0;
	if(uartData==0x95||uartData==0x92||uartData==0x8E||uartData==0x8F||uartData==0x93||uartData==0x8D||uartData==0x78||uartData==0x7B||uartData==0x71||uartData==0x7C||uartData==0x7A){
		if(lookUpIndex!=uartData){
		lookUpIndex=uartData;
		readUart=true;
		
		}
		uartDelayCounter=2048;
	}
	else{
		if(uartDelayCounter==1){
			if(lookUpIndex!=0){
				weaponMatrix.newString("Welcome!             ");
				playFlames(0);
			}
			uartDelayCounter=2048;
			lookUpIndex=0;
		}
		else
		--uartDelayCounter;
	}

}


#include "ISR.Impl.h"