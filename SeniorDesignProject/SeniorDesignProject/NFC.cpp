#include "NFC.h"

char pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
char pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

#define PN532_PACKBUFFSIZ	64
char pn532_packetbuffer[PN532_PACKBUFFSIZ];
NFC::NFC(){}
void NFC::begin(void) {
	spi.pickASlave(nfcDev);
	
	_delay_ms(1000);
	
	// send dummy command to get it synched up
	pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;
	uint8_t check = sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 1, 1000);
}

uint32_t NFC::getFirmwareVersion(void) {
	uint32_t response;

	pn532_packetbuffer[0] = PN532_FIRMWAREVERSION;

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 1, 1000)) return 0;

	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 12);

	// check some basic stuff
	if (0 != strncmp((char*)pn532_packetbuffer,
	(char*)pn532response_firmwarevers, 6))
	return 0;

	response = pn532_packetbuffer[6];
	response << 8;
	response |= pn532_packetbuffer[7];
	response <<= 8;
	response |= pn532_packetbuffer[8];
	response <<= 8;
	response |= pn532_packetbuffer[9];

	return response;

}

// default timeout of one second
uint8_t NFC::sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
	uint16_t timer = 0;
	
	// write the command
	spiwritecommand(cmd, cmdlen);

	// wait for the chip to say it's ready
	while(readspistatus() != PN532_SPI_READY) {
		if (timeout != 0) {
			timer += 10;
			if (timer > timeout) {
				return 0;
			}
		}
		_delay_ms(10);
	}

	// read acknowledgement
	if (0 == spi_readack()) {
		return 0;
	}

	timer = 0;
	// wait for the chip to say it's ready
	while(readspistatus() != PN532_SPI_READY) {
		if (timeout != 0) {
			timer += 10;
			if (timer > timeout) {
				return 0;
			}
		}
		_delay_ms(10);
	}
	
	return 1;
}

uint8_t NFC::SAMConfig(void) {
	pn532_packetbuffer[0] = PN532_SAMCONFIGURATION;
	pn532_packetbuffer[1] = 0x01; // normal mode
	pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
	pn532_packetbuffer[3] = 0x01; // use IRQ pin!

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 4, 1000)) return 0;

	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 8);

	return (pn532_packetbuffer[5] == 0x15 ? 1 : 0);
}

uint8_t NFC::authenticateBlock(uint8_t cardnumber, uint32_t cid, uint8_t blockaddress,
uint8_t authtype, uint8_t* keys) {
	pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
	pn532_packetbuffer[1] = cardnumber;

	if (authtype == KEY_A) {
		pn532_packetbuffer[2] = PN532_AUTH_WITH_KEYA;
		} else {
		pn532_packetbuffer[2] = PN532_AUTH_WITH_KEYB;
	}
	pn532_packetbuffer[3] = blockaddress; // Address can be 0-63 for MIFARE 1K card

	pn532_packetbuffer[4] = keys[0];
	pn532_packetbuffer[5] = keys[1];
	pn532_packetbuffer[6] = keys[2];
	pn532_packetbuffer[7] = keys[3];
	pn532_packetbuffer[8] = keys[4];
	pn532_packetbuffer[9] = keys[5];

	pn532_packetbuffer[10] = ((cid >> 24) & 0xFF);
	pn532_packetbuffer[11] = ((cid >> 16) & 0xFF);
	pn532_packetbuffer[12] = ((cid >> 8) & 0xFF);
	pn532_packetbuffer[13] = ((cid >> 0) & 0xFF);

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 14, 1000)) return 0;
	
	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 2+6);

	if ((pn532_packetbuffer[6] = 0x41) && (pn532_packetbuffer[7] == 0x00)) {
		return 1;
		} else {
		return 0;
	}
}

uint8_t NFC::readMemoryBlock(uint8_t cardnumber, uint8_t blockaddress, uint8_t* block) {
	pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
	pn532_packetbuffer[1] = cardnumber;
	pn532_packetbuffer[2] = PN532_MIFARE_READ;
	pn532_packetbuffer[3] = blockaddress;

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 4, 1000)) return 0;

	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 18+6);
	for (uint8_t i = 8; i < 18+6; i++) {
		block[i-8] = pn532_packetbuffer[i];
	}

	if ((pn532_packetbuffer[6] == 0x41) && (pn532_packetbuffer[7] == 0x00)) {
		return 1; // read successful
		} else {
		return 0;
	}
}

// Do not write to Sector Trailer Block unless you know what you are doing
uint8_t NFC::writeMemoryBlock(uint8_t cardnumber, uint8_t blockaddress, uint8_t* block) {
	pn532_packetbuffer[0] = PN532_INDATAEXCHANGE;
	pn532_packetbuffer[1] = cardnumber;
	pn532_packetbuffer[2] = PN532_MIFARE_WRITE;
	pn532_packetbuffer[3] = blockaddress;

	for (uint8_t byte = 0; byte < 16; byte++) {
		pn532_packetbuffer[4+byte] = block[byte];
	}

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 4+16, 1000)) return 0;

	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 2+6);

	if ((pn532_packetbuffer[6] == 0x41) && (pn532_packetbuffer[7] == 0x00)) {
		return 1; // write successful
		} else {
		return 0;
	}
}

uint32_t NFC::readPassiveTargetID(uint8_t cardbaudrate) {
	uint32_t cid;

	pn532_packetbuffer[0] = PN532_INLISTPASSIVETARGET;
	pn532_packetbuffer[1] = 1; // max 1 card at once
	pn532_packetbuffer[2] = cardbaudrate;

	if (0 == sendCommandCheckAck((uint8_t *)pn532_packetbuffer, 3, 1000)) return 0x00; // no cards read

	// read data packet
	readspidata((uint8_t *)pn532_packetbuffer, 20);

	if (pn532_packetbuffer[7] != 1) return 0;

	uint16_t sens_res = pn532_packetbuffer[9];
	sens_res <<= 8;
	sens_res |= pn532_packetbuffer[10];

	cid = 0;
	for (uint8_t i = 0; i < pn532_packetbuffer[12]; i++) {
		cid <<= 8;
		cid |= pn532_packetbuffer[13+i];
	}

	return cid;
}

// high level SPI
uint8_t NFC::spi_readack(void) {
	uint8_t ackbuff[6];
	
	readspidata(ackbuff, 6);

	return (0 == strncmp((char*) ackbuff, (char*) pn532ack, 6)) ? 1 : 0;
}

// mid level SPI
uint8_t NFC::readspistatus(void) {
	spi.pickASlave(nfcDev);
	//PORTX &= ~(1 << PN532_CS); // write LOW to CS pin
	_delay_us(2000);

	spiwrite(PN532_SPI_STATREAD);
	// read byte
	uint8_t x = spiread();

	//PORTX |= (1 << PN532_CS); // write HIGH to CS pin
	spi.pickASlave(none);
	return x;
}

void NFC::readspidata(uint8_t* buff, uint8_t n) {
	//PORTX &= ~(1 << PN532_CS); // write LOW to CS pin
	spi.pickASlave(nfcDev);
	_delay_us(2000);
	spiwrite(PN532_SPI_DATAREAD);

	for (uint8_t i = 0; i < n; i++) {
		_delay_us(1000);
		buff[i] = spiread();
	}

	//PORTX |= (1 << PN532_CS); // write HIGH to CS pin
	spi.pickASlave(none);
}

void NFC::spiwritecommand(uint8_t* cmd, uint8_t cmdlen) {
	uint8_t checksum;

	cmdlen++;

	//PORTX &= ~(1 << PN532_CS); // write LOW to CS pin
	spi.pickASlave(nfcDev);
	_delay_us(2000);
	spiwrite(PN532_SPI_DATAWRITE);

	checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
	spiwrite(PN532_PREAMBLE);
	spiwrite(PN532_PREAMBLE);
	spiwrite(PN532_STARTCODE2);

	spiwrite(cmdlen);
	uint8_t cmdlen_1 = ~cmdlen + 1;
	spiwrite(cmdlen_1);

	spiwrite(PN532_HOSTTOPN532);
	checksum += PN532_HOSTTOPN532;

	for (uint8_t i=0; i<cmdlen-1; i++) {
		spiwrite(cmd[i]);
		checksum += cmd[i];
	}

	uint8_t checksum_1 = ~checksum;
	spiwrite(checksum_1);
	spiwrite(PN532_POSTAMBLE);
	spi.pickASlave(none);
	//PORTX |= (1 << PN532_CS); // write HIGH to CS pin
}

// low level SPI
void NFC::spiwrite(uint8_t c) {
	spi.pickASlave(nfcDev);
	spi.sendLSBSPI(c);
	spi.pickASlave(none);
}

uint8_t NFC::spiread(void) {
	spi.pickASlave(nfcDev);
	return spi.sendLSBSPI(0x00);
	spi.pickASlave(none);
}
