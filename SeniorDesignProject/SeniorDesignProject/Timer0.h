#ifndef TIMER0_H_  /* Include guard */
#define TIMER0_H_

#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
class Timer0{
	public:

	/************************************************************************/
	//Pre Nothing
	//Post Timer one will be set up to fire at 112000 Hz
	/************************************************************************/
	void initTimer0(){
		
		ASSR = 0x00; 	//Use main clock
		
		TCNT0 = 0;//Set counter to zero. It will auto clear.

		//Set counter to correct value.
		OCR0A = 142; //16000000=1*112001*x; x=142.85
		
		TCCR0A = 1<<(WGM01); //Make the counter register reset when counter is reached
		
		TCCR0B = ((1<<CS00));//Set prescaler for 1;

		
		TIFR0 = TIFR0;//Clear interrupt flag so interrupt can fire.

		TIMSK0 = 1<< (OCIE0A);//Enable interrupt when counter is reached.
	}


	/************************************************************************/
	//Pre Nothing
	// Post LED will be set to output
	/************************************************************************/
	void initIRLED(){
		//Set pin7 on port D to output
		DDRD|=1<<PIND7;
	}
	/************************************************************************/
	//Pre IRLED must be inited
	//Post IRLED will be set to correct value for this given pulse
	//Return if a packet was completed.
	/************************************************************************/
	uint16_t healCount=0;
	bool sendIRHeal(){
		const uint16_t firstPacketstart=0;
		const uint16_t firstPacketEnd=20;
		const uint16_t secondPacketstart=300;
		const uint16_t secondPacketEnd=360;
		const uint16_t thirdPacketstart=600;
		const uint16_t thirdPacketEnd=660;
		const uint16_t forthPacketstart=900;
		const uint16_t forthPacketEnd=1199;
		const uint16_t mirpsEnd=1999;
		
		//Check if it is at the correct spot to set the IRLED
		if((healCount<firstPacketEnd&&healCount>=firstPacketstart)||(healCount<secondPacketEnd&&healCount>=secondPacketstart)||(healCount<thirdPacketEnd&&healCount>=thirdPacketstart)||(healCount<forthPacketEnd&&healCount>=forthPacketstart)){
			//Make rising edge
			if(healCount&1)
			PORTD|=1<<PIND7;
			//Make falling edge
			else
			PORTD&=~(1<<PIND7);
			//Increment the position in the wave
			++healCount;
			//Packet not completed
			return 0;
			//Check if packet was completed
			}else if(healCount>mirpsEnd){
			PORTD&=~(1<<PIND7);
			healCount=0;
			return 1;
			//In a non transmit part of a MIRPs Packet
			}else{
			PORTD&=~(1<<PIND7);
			++healCount;
			return 0;
		}
		
	}
	/************************************************************************/
	//Pre IRLED must be inited
	//Post IRLED will be set to correct value for this given pulse
	//Return if a packet was completed.
	/************************************************************************/
	uint16_t damageCount=0;
	bool sendIRDamage(){
		const uint16_t firstPacketstart=0;
		const uint16_t firstPacketEnd=20;
		const uint16_t secondPacketstart=300;
		const uint16_t secondPacketEnd=340;
		const uint16_t thirdPacketstart=600;
		const uint16_t thirdPacketEnd=640;
		const uint16_t forthPacketstart=900;
		const uint16_t forthPacketEnd=1199;
		const uint16_t mirpsEnd=1999;
		//Check if it is at the correct spot to set the IRLED
		if((damageCount<firstPacketEnd&&damageCount>=firstPacketstart)||(damageCount<secondPacketEnd&&damageCount>=secondPacketstart)||(damageCount<thirdPacketEnd&&damageCount>=thirdPacketstart)||(damageCount<forthPacketEnd&&damageCount>=forthPacketstart)){
			//Make rising edge
			if(damageCount&1)
			PORTD|=1<<PIND7;
			//Make falling edge
			else
			PORTD&=~(1<<PIND7);
			//Increment the position in the wave
			++damageCount;
			return 0;
			//Check if packet was completed
			}else if(damageCount>mirpsEnd){
			PORTD&=~(1<<PIND7);
			damageCount=0;
			return 1;
			//In a non transmit part of a MIRPs Packet
			}else{
			PORTD&=~(1<<PIND7);
			++damageCount;
			return 0;
		}
		
	}
	volatile bool MIRPStoggle=true;
	volatile int16_t MIRPScount=0;
	/************************************************************************/
	//Pre IRLED inited
	//Post MIRPs Packets sent
	/************************************************************************/
	void sendMIRPS(int16_t count,bool sendHeal){
		while(MIRPScount>0);//wait for last MIRPs to finish
		cli();//disable interupst or packet mismatch might happen
		MIRPScount=count;//set number of packets
		MIRPStoggle=sendHeal;//set type of packets
		sei();//start global interupts
	}

	/************************************************************************/
	//Pre IRLED inited
	//Post at 112000Hz will fire and set the led correctly
	/************************************************************************/
	void MIRPSISR(){
		if(MIRPScount>0){
			if(MIRPStoggle){
				if(sendIRHeal()==1){
					--MIRPScount;
				}
			}
			else{
				if(sendIRDamage()==1){
					--MIRPScount;
				}
			}
		}
	}
	
};

#endif