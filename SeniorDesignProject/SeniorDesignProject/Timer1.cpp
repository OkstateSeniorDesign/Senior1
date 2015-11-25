#include "Timer1.h"
Timer1::Timer1(void){
	initTimer1();
}

void Timer1::initTimer1(){
	
	// set up timer with prescaler = 8
	TCCR1B |= (1 << CS11);

	// initialize counter
	TCNT1 = 0;

	// enable overflow interrupt
	TIMSK1 |= (1 << TOIE1);

	// enable global interrupts
sei();
}



