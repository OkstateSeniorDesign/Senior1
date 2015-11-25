#ifndef NFC_H_  /* Include guard */
#define NFC_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include "SPI.h"

class NFC{


	public:
	NFC();
	/**
	File name: pn532_spi.h

	Header file for pn532 library written for A Mega644.
	Adapted from pn532.h written by Adafruit Industries

	Author: Renato Amez

*/

#define PN532_PREAMBLE 0x00
#define PN532_STARTCODE1 0x00
#define PN532_STARTCODE2 0xFF
#define PN532_POSTAMBLE 0x00

#define PN532_HOSTTOPN532 0xD4

#define PN532_FIRMWAREVERSION 0x02
#define PN532_GETGENERALSTATUS 0x04
#define PN532_SAMCONFIGURATION  0x14
#define PN532_INLISTPASSIVETARGET 0x4A
#define PN532_INDATAEXCHANGE 0x40
#define PN532_MIFARE_READ 0x30
#define PN532_MIFARE_WRITE 0xA0

#define PN532_AUTH_WITH_KEYA 0x60
#define PN532_AUTH_WITH_KEYB 0x61


#define PN532_WAKEUP 0x55

#define  PN532_SPI_STATREAD 0x02
#define  PN532_SPI_DATAWRITE 0x01
#define  PN532_SPI_DATAREAD 0x03
#define  PN532_SPI_READY 0x01

#define PN532_MIFARE_ISO14443A 0x0

#define KEY_A	1
#define KEY_B	2



	void begin(void);
	SPI spi;
	uint8_t SAMConfig(void);
	uint32_t getFirmwareVersion(void);
	uint32_t readPassiveTargetID(uint8_t cardbaudrate);
	uint8_t authenticateBlock(uint8_t cardnumber /* 1 or 2 */,
						uint32_t cid /* Card NUID */,
						uint8_t blockaddress /* 0 to 63 */,
						uint8_t authtype /* Either KEY_A or KEY_B */,
						uint8_t* keys);

	uint8_t readMemoryBlock(uint8_t cardnumber /* 1 or 2 */,
						uint8_t blockaddress /* 0 to 63 */,
						uint8_t* block);
	uint8_t writeMemoryBlock(uint8_t cardnumber /* 1 or 2 */,
						uint8_t blockaddress /* 0 to 63 */,
						uint8_t* block);
	uint8_t sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout);

	uint8_t spi_readack(void);
	uint8_t readspistatus(void);
	void readspidata(uint8_t* buff, uint8_t n);
	void spiwritecommand(uint8_t* cmd, uint8_t cmdlen);
	void spiwrite(uint8_t c);
	uint8_t spiread(void);
};

#endif