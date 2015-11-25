#include <SPI.h>

#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)
#define PN532_HOSTTOPN532                   (0xD4)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)
#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)
#define PN532_MIFARE_ISO14443A              (0x00)
#define MIFARE_CMD_READ                     (0x30)


// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SS  (33)

byte pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
byte pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};


#ifdef PN532DEBUG
  #define debug_println(...) do Serial.println(...)
  #define debug_print(...) do Serial.print(...)
#endif
#define PN532_PACKBUFFSIZ 64
byte pn532_packetbuffer[PN532_PACKBUFFSIZ];

void NFCbegin(void) {
    // SPI initialization
    pinMode(PN532_SS, OUTPUT);
    
    SPI.begin();      
    SPI.beginTransaction(SPISettings(100000000, LSBFIRST, SPI_MODE0));
    
    digitalWrite(PN532_SS, LOW);

    delay(1000);
    
    // not exactly sure why but we have to send a dummy command to get synced up
    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    sendCommandCheckAck(pn532_packetbuffer, 1, 1000);

    // ignore response!
    digitalWrite(PN532_SS, HIGH);
  SPI.endTransaction();
}

boolean NFCconfig(void) {
  pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = 0x01; // normal mode;
  pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
  pn532_packetbuffer[3] = 0x01; // use IRQ pin!

  if (!sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
    return false;

  // read data packet
  readdata(pn532_packetbuffer, 8);

  return  (pn532_packetbuffer[5] == 0x15);
}

void sendReadPassiveTargetID(uint8_t cardbaudrate) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
  pn532_packetbuffer[2] = cardbaudrate;

  //sendCommandCheckAck(pn532_packetbuffer, 3)
  writecommand(pn532_packetbuffer, 3);
}

byte readPassiveTargetID(uint8_t * uid, uint8_t * uidLength, uint16_t timeout) {
  // read data packet
  readdata(pn532_packetbuffer, 20);
  // check some basic stuff

  /* ISO14443A card response should be in the following format:

    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              Tags Found
    b8              Tag Number (only one used in this example)
    b9..10          SENS_RES
    b11             SEL_RES
    b12             NFCID Length
    b13..NFCIDLen   NFCID                                      */
  if (pn532_packetbuffer[7] != 1)
    return 0;

  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];

  //Serial.println(pn532_packetbuffer[12], HEX);
  *uidLength = pn532_packetbuffer[12];
  for (uint8_t i=0; i < pn532_packetbuffer[12]; i++)
  {
    uid[i] = pn532_packetbuffer[13+i];
    //Serial.print(pn532_packetbuffer[12], HEX);
  }
  return 1;
}


uint32_t getFirmwareVersion(void) {
  uint32_t response;

  pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (!sendCommandCheckAck(pn532_packetbuffer, 1, 1000)) {
    return 0;
  }

  // read data packet
  readdata(pn532_packetbuffer, 12);

  // check some basic stuff
  if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
    //#ifdef PN532DEBUG
      Serial.println(F("Firmware doesn't match!"));
    //#endif
    return 0;
  }

  int offset = 6;  // Skip a response byte when using I2C to ignore extra data.
  response = pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];

  return response;
}

boolean sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
  uint16_t timer = 0;

  // write the command
  writecommand(cmd, cmdlen);

  // Wait for chip to say its ready!
  if (!waitready(timeout)) {
    return false;
  }
  // read acknowledgement
  if (!readack()) {
      Serial.println(F("No ACK frame received!"));
    return false;
  }

  // Wait for the chip to be ready again.
    if (!waitready(timeout)) {
      return false;
    }
  return true; // ack'd command
}

uint8_t ntag2xx_ReadPage (uint8_t page, uint8_t * buffer)
{
  // TAG Type       PAGES   USER START    USER STOP
  // --------       -----   ----------    ---------
  // NTAG 203       42      4             39
  // NTAG 213       45      4             39
  // NTAG 215       135     4             129
  // NTAG 216       231     4             225

  if (page >= 231)
  {
    return 0;
  }

  /* Prepare the command */
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                   /* Card number */
  pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
  pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

  /* Send the command */
  if (!sendCommandCheckAck(pn532_packetbuffer, 4, 1000))
  {
    return 0;
  }

  /* Read the response packet */
  readdata(pn532_packetbuffer, 26);

  /* If byte 8 isn't 0x00 we probably have an error */
  if (pn532_packetbuffer[7] == 0x00)
  {
    /* Copy the 4 data bytes to the output buffer         */
    /* Block content starts at byte 9 of a valid response */
    /* Note that the command actually reads 16 byte or 4  */
    /* pages at a time ... we simply discard the last 12  */
    /* bytes                                              */
    memcpy (buffer, pn532_packetbuffer+8, 4);
  }
  else
  {
    return 0;
  }

  // Return OK signal
  return 1;
}

void setup(void) {
  Serial.begin(115200);
  //Serial.println("Hello!");
  pinMode(7, INPUT);
  pinMode(31, INPUT);

  NFCbegin();

  uint32_t versiondata = getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  NFCconfig();
  
  //Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success = false;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t pageCount = 7 + 5; 
  char cardName[MAX_STRING_LENGTH];
  uint8_t cardCount = 0;
  bool cardNameReading = false;
  uint8_t j = 0;
    
  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  //Serial.println("Trying to Read");
  sendReadPassiveTargetID(PN532_MIFARE_ISO14443A);
  isready();
  readack();

  delay(20);
  while(digitalRead(31) == HIGH) {
  }
  
  success = readPassiveTargetID(uid, &uidLength, 1000);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    PrintHex(uid, uidLength);
    Serial.println("");
    
    if (uidLength == 7)
    {
      uint8_t data[32];
      
      // We probably have an NTAG2xx card (though it could be Ultralight as well)
      //Serial.println("Seems to be an NTAG2xx tag (7 byte UID)");    
      
      // NTAG2x3 cards have 39*4 bytes of user pages (156 user bytes),
      // starting at page 4 ... larger cards just add pages to the end of
      // this range:
      
      // See: http://www.nxp.com/documents/short_data_sheet/NTAG203_SDS.pdf
      
      // TAG Type       PAGES   USER START    USER STOP
      // --------       -----   ----------    ---------
      // NTAG 203       42      4             39
      // NTAG 213       45      4             39
      // NTAG 215       135     4             129
      // NTAG 216       231     4             225      
      
      for (uint8_t i = 7; i < pageCount; i++) 
      {
        success = ntag2xx_ReadPage(i, data);
        //PrintHexChar(data, 4);
        if (success) 
        {
          if(data[0] == 0x65 && data[1] == 0x6E && i==7)
          {
            cardName[cardCount++] = data[2];
            cardName[cardCount++] = data[3];
            cardNameReading = true;
          }
          else if(cardNameReading)
          {
            for(; j < 4 && data[j] != 0xFE && cardCount < MAX_STRING_LENGTH; j++)
            {
              cardName[cardCount++] = data[j];
            }
            j=0;
            if(data[j] == 0xFE || cardCount==MAX_STRING_LENGTH)
            {
              cardNameReading = false;
            }
          }
        }
        else
        {
          //Serial.println("Unable to read the requested page!");
        }   
      }
      Serial.println(cardName);   
    }
    else
    {
      Serial.println("This doesn't seem to be an NTAG203 tag (UUID length != 7 bytes)!");
    }
  }
}

bool isready() {
    // SPI read status and check if ready.
    SPI.beginTransaction(SPISettings(100000000, LSBFIRST, SPI_MODE0));
    digitalWrite(PN532_SS, LOW);
    delay(2);
    SPI.transfer(PN532_SPI_STATREAD);
    // read byte
    uint8_t x = SPI.transfer(0x00);
    
    digitalWrite(PN532_SS, HIGH);
    SPI.endTransaction();

    // Check if status is ready.
    return x == PN532_SPI_READY;
}

bool waitready(uint16_t timeout) {
  uint16_t timer = 0;
  while(!isready()) {
    if (timeout != 0) {
      timer += 10;
      if (timer > timeout) {
        Serial.println("TIMEOUT!");
        return false;
      }
    }
    delay(10);
  }
  return true;
}

void readdata(uint8_t* buff, uint8_t n) {
    
    // SPI write
    SPI.beginTransaction(SPISettings(100000000, LSBFIRST, SPI_MODE0));

    digitalWrite(PN532_SS, LOW);
    delay(2);
    SPI.transfer(PN532_SPI_DATAREAD);

    //Serial.print(F("Reading: "));
    for (uint8_t i=0; i<n; i++) {
      delay(1);
      buff[i] = SPI.transfer(0x00);
      //Serial.print(F(" 0x"));
      //Serial.print(buff[i], HEX);
    }
    //Serial.println();
    digitalWrite(PN532_SS, HIGH);
    
    SPI.endTransaction();
}

void writecommand(uint8_t* cmd, uint8_t cmdlen) {
    // SPI command write.
    uint8_t checksum;

    cmdlen++;
    
    //Serial.print(F("\nSending Command: "));

    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE0));
    digitalWrite(PN532_SS, LOW);
    delay(2);     // or whatever the delay is for waking up the board
    SPI.transfer(PN532_SPI_DATAWRITE);

    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    SPI.transfer(PN532_PREAMBLE);
    SPI.transfer(PN532_PREAMBLE);
    SPI.transfer(PN532_STARTCODE2);

    SPI.transfer(cmdlen);
    SPI.transfer(~cmdlen + 1);

    SPI.transfer(PN532_HOSTTOPN532);
    checksum += PN532_HOSTTOPN532;

     /*#ifdef false
      Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
      Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
      Serial.print(F(" 0x")); Serial.print(PN532_STARTCODE2, HEX);
      Serial.print(F(" 0x")); Serial.print(cmdlen, HEX);
      Serial.print(F(" 0x")); Serial.print(~cmdlen + 1, HEX);
      Serial.print(F(" 0x")); Serial.print(PN532_HOSTTOPN532, HEX);
    #endif*/

    for (uint8_t i=0; i<cmdlen-1; i++) {
      SPI.transfer(cmd[i]);
      checksum += cmd[i];
       /*#ifdef PN532DEBUG
        Serial.print(F(" 0x")); Serial.print(cmd[i], HEX);
      #endif*/
    }

    SPI.transfer(~checksum);
    SPI.transfer(PN532_POSTAMBLE);
    digitalWrite(PN532_SS, HIGH);
    SPI.endTransaction();
/*
    #ifdef false
      Serial.print(F(" 0x")); Serial.print(~checksum, HEX);
      Serial.print(F(" 0x")); Serial.print(PN532_POSTAMBLE, HEX);
      Serial.println();
    #endif*/
}

void PrintHex(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos]&0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}

void PrintHexChar(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos], HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.print(F("  "));
  for (szPos=0; szPos < numBytes; szPos++)
  {
    if (data[szPos] <= 0x1F)
      Serial.print(F("."));
    else
      Serial.print((char)data[szPos]);
  }
  Serial.println();
}

bool readack() {
  uint8_t ackbuff[6];

  readdata(ackbuff, 6);
  return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));
}



