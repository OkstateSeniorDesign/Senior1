#ifndef HITMATRIX_H_  /* Include guard */
#define HITMATRIX_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include "SPI.h"

class HitMatrix{
	
	#define maxPanels 3

	private:
	SPI spi;
	const uint8_t numPanels=maxPanels;
	uint8_t one=1;
	bool moveRight=false;
	uint8_t scrollCounter=15;
	uint8_t multiplexCounter=0;
	uint8_t lastHit=0;
	public:
		HitMatrix(void);
		void testHitMatrix(void);
	inline bool wasGoodHit(){
		return ((lastHit>16||lastHit<8)?false : true);
	}
	inline void wasHit(){
		lastHit=scrollCounter;
	}
	inline void moveBar(void){
	
		if(moveRight){
			++scrollCounter;
			}else{
			--scrollCounter;
		}
		if(scrollCounter==0||scrollCounter==numPanels*8-1){
			moveRight=!moveRight;
		}
		
	};
	inline void outputMatrix(void){
		spi.pickASlave(hitIndicatorMatrix);
				spi.sendSPI( 0b00000000);//Green 3
				scrollCounter >  7 && scrollCounter <16 ? spi.sendSPI( 1<<scrollCounter%8) : spi.sendSPI( 0b00000000);//Green 2
				spi.sendSPI( 0b00000000);//Green 1
				scrollCounter > 15 && scrollCounter <24 ? spi.sendSPI( 1<<scrollCounter%8) : spi.sendSPI( 0b00000000);//Red 3
				spi.sendSPI( (scrollCounter ==8||scrollCounter ==9||scrollCounter ==14||scrollCounter ==15)?1<<scrollCounter%8:0b00000000);//Red   2
				scrollCounter >= 0 && scrollCounter < 8 ? spi.sendSPI( 1<<scrollCounter%8) : spi.sendSPI( 0b00000000);//Red 1
				spi.sendSPI( 0b00000000);//Ground
				
		spi.pickASlave(none);

	};

	
	#undef  maxPanels
};
#endif