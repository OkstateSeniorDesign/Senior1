#ifndef TIMER1_H_  /* Include guard */
#define TIMER1_H_

#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>



class Timer1{
	
	
	
	public:
	Timer1();
	void initTimer1();
	
	private:

	
	
};



#endif