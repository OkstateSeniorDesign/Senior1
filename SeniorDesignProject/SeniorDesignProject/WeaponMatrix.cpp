#include "WeaponMatrix.h"

#define max7219_reg_noop        0x00
#define max7219_reg_digit0      0x01
#define max7219_reg_digit1      0x02
#define max7219_reg_digit2      0x03
#define max7219_reg_digit3      0x04
#define max7219_reg_digit4      0x05
#define max7219_reg_digit5      0x06
#define max7219_reg_digit6      0x07
#define max7219_reg_digit7      0x08
#define max7219_reg_decodeMode  0x09
#define max7219_reg_intensity   0x0a
#define max7219_reg_scanLimit   0x0b
#define max7219_reg_shutdown    0x0c
#define max7219_reg_displayTest 0x0f
	
WeaponMatrix::WeaponMatrix(){
	
	
}
void WeaponMatrix::setCommand(uint8_t reg, uint8_t data){
		spi.pickASlave(none);
		spi.pickASlave(weaponIndicatorMatrix);
	
	for(uint8_t i=1;i<=numPanels;++i){
		spi.sendSPI(reg);
		spi.sendSPI(data);
	}
	spi.pickASlave(none);
}
void WeaponMatrix::initMatrix(SPI out){
		 spi=out;
		 spi.pickASlave(none);
		 setCommand(max7219_reg_scanLimit, 0x07);
		 setCommand(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
		 setCommand(max7219_reg_shutdown, 0x01);    // not in shutdown mode
		 setCommand(max7219_reg_displayTest, 0x00); // no display test
		 setCommand(max7219_reg_intensity, 0x0F);
		
		
		//clear
		 for(uint16_t i=1;i<=8;++i){
			 spi.pickASlave(weaponIndicatorMatrix);
			 for(uint8_t j=1;j<=numPanels;++j){
				 spi.sendSPI(i);
				 spi.sendSPI(0x00);
			 }
			 spi.pickASlave(none);
		 }
		 counter=0;
		 
		 
		 
}
void WeaponMatrix::incStartingLocation(){
	counter=(counter>=maxChars*5+numPanels*8)?0:counter+1;
}

uint8_t WeaponMatrix::getData(uint16_t i){
	uint16_t j=i-numPanels*8;
	if(i<numPanels*8||j>maxChars*5){
		return 0x00;
	}
	else{
		if(usebuffA){
			return buffA[j];
		}else
			return buffB[j];
	}
}
void WeaponMatrix::writeToMatrix(){
	
	for(uint8_t i=1;i<=8;i++){
		spi.pickASlave(weaponIndicatorMatrix);
		for(uint8_t j=0;j<numPanels;++j){
			spi.sendSPI(i);
			spi.sendSPI(getData((counter+i)+j*8));
			
		}
		spi.pickASlave(none);
	}
	
}
void WeaponMatrix::newString(char * newData){
	uint8_t loc=0;
	uint8_t matrixPosition=0;
	while(newData[loc]!='\0'){
		//for(uint8_t i=0;i<CH[(newData[loc]-32)*7];i++){
		for(uint8_t i=0;i<5;i++){
			if(usebuffA){
				buffB[matrixPosition]=CH[(newData[loc]-32)*7+i+2];
				}else{
				buffA[matrixPosition]=CH[(newData[loc]-32)*7+i+2];
			}
			matrixPosition++;
		}
		loc++;
	}
	usebuffA=!usebuffA;
	counter=0;
}