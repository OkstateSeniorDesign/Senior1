#ifndef UART_H_  /* Include guard */
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>


#define  endl	"\r\n"


class UART{
	
	
	
	public:
		void putByte(unsigned char data);
		char getByte();
		UART();
		void initUART0();
		
		
		//Send out any data type over uart
		template <typename T> uint16_t UARTOut(const T& value)
		{
			const uint8_t * p = (const uint8_t*) &value;
			uint16_t i;
			for (i = 0; i < sizeof value; i++)
			putByte(*p++);
			return i;
		}
	private:
		void initUART1(void);
};


//Overload << (insertion) operator for UARTs
template <typename T> UART& operator<<(UART& os, const T& data )
{
	os.UARTOut(data);
	return os;
}

#endif 