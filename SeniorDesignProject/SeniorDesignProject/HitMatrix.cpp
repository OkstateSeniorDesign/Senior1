#include "HitMatrix.h"

HitMatrix::HitMatrix(void ){
	
}
void HitMatrix::testHitMatrix(void){
	while(true){
		for(int i=0; i<254;++i){
			outputMatrix();
		}
		moveBar();
	}
}