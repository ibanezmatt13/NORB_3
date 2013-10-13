/*  All code for NORB_v3 AVR HAB Tracker
    Copyright (C) 2013 Matthew Beckett

    Credits:
      	
      	mfa298_M1ARI provided invaluable help in the understanding of how to create the code
      	Functions for setting and checking flight mode from Anthony Stirk (M0UPU)
      	A special thanks to Ed Moore for providing the massive encouragement and general assistance in the making of this code
      	Thanks to the many people on #highaltitude UKHAS who assisted in all other aspects of assistance.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details. */
    
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <util/crc16.h>

#define RADIOPIN 13

// this command will set flight mode
uint8_t setNav[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };



uint16_t gps_CRC16_checksum (char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;
 
  crc = 0xFFFF;
 
  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }
 
  return crc;
}  
 

 
void rtty_txstring (char * string)
{
 
  /* Simple function to sent a char at a time to 
   	** rtty_txbyte function. 
   	** NB Each char is one byte (8 Bits)
   	*/
 
  char c;
 
  c = *string++;
 
  while ( c != '\0')
  {
    rtty_txbyte (c);
    c = *string++;
  }
}
 
 
 
void rtty_txbyte (char c)
{
  /* Simple function to sent each bit of a char to 
   	** rtty_txbit function. 
   	** NB The bits are sent Least Significant Bit first
   	**
   	** All chars should be preceded with a 0 and 
   	** proceded with a 1. 0 = Start bit; 1 = Stop bit
   	**
   	*/
 
  int i;
 
  rtty_txbit (0); // Start bit
 
  // Send bits for for char LSB first	
 
  for (i=0;i<7;i++) // Change this here 7 or 8 for ASCII-7 / ASCII-8
  {
    if (c & 1) rtty_txbit(1); 
 
    else rtty_txbit(0);	
 
    c = c >> 1;
 
  }
 
  rtty_txbit (1); // Stop bit
  rtty_txbit (1); // Stop bit
}
 
 
 
 
void rtty_txbit (int bit)
{
  if (bit)
  {
    // high
    digitalWrite(RADIOPIN, HIGH);
  }
  else
  {
    // low
    digitalWrite(RADIOPIN, LOW);
 
  }
 
  //50 baud...
  delayMicroseconds(10000);  
  delayMicroseconds(10000); 
                            
} 
 
 
 
// function to convert latitude into decimal degrees
int check_latitude(char* latitude, char* ind, float* new_latitude)
{
  float result = 0;
 
  if (strlen(ind)==1)
  {
    result+=(latitude[0]-'0')*10;
    result+=(latitude[1]-'0');
    result+=(strtod(&latitude[2],NULL) /60);
	
  if (strcmp(ind,"S")==0)
  {
    result*=-1;
  }
	
  *new_latitude = result;
  return 1;
  }
  else
  {
    return -1;
  }
}
 
 
 
 
 
 
// function to convert longitude into decimal degrees
int check_longitude(char* longitude, char* ind, float* new_longitude)
{
  float result = 0;
  
  if (strlen(ind)==1)
  {
    result+=(longitude[0]-'0')*100;
    result+=(longitude[1]-'0')*10;
    result+=(longitude[2]-'0');
    result+=(strtod(&longitude[3],NULL) /60);

  if (strcmp(ind,"W")==0)
  {
    result*=-1;
  }
	
  *new_longitude = result;
  return 1;
  }
  else
  {
    return -1;
  }
}
 
 
 
// function to parse NMEA data and send to NTX2
int parse_NMEA(char* mystring, byte flightmode)
{
  int counter = 0; // sentence id
  float new_latitude;
  float new_longitude;
  char time[12];
  int time_length;
  char identifier[7];
  char latitude[12];
  char north_south[2];
  char longitude[12];
  char east_west[2];
  int lock;
  char satellites[3];
  char altitude[10];
  char datastring[100] = "";
  int check_latitude_error;
  int check_longitude_error;
 
 
  // split NMEA string into individual data fields and check that a certain number of values have been obtained
  // $GPGGA,212748.000,5056.6505,N,00124.3531,W,2,07,1.8,102.1,M,47.6,M,0.8,0000*6B
  if (sscanf(mystring, "%6[^,],%[^,],%9[^,],%1[^,],%[^,],%1[^,],%d,%2[^,],%*[^,],%[^,]", identifier, time, latitude, north_south, longitude, east_west, &lock, satellites, altitude) == 9)
  {
    // check for the identifer being invalid
    if (strncmp(identifier, "$GPGGA", 6) != 0)
    {
      printf("Invalid NMEA String...1\n");
      return 0;
    }
    
    // check for a valid lock
    if (lock == 0)
    {
      printf("No Lock...\n");
      return 0;
    }
    
    time_length = strlen(time); //get length of time
  
    // check time is in the format: HHMMSS unless either already set or NMEA input was invalid
    if (time_length > 6)
    {
      sscanf(time, "%[^.]", time);
    }
    else if (time_length < 6)
    {  
      printf("Invalid NMEA String...2\n");
      return 0;
    }
    
    // store the return value of the latitude/longitude functions accordinly
    check_latitude_error = check_latitude(latitude, north_south, &new_latitude);
    check_longitude_error = check_longitude(longitude, east_west, &new_longitude);
    
    // if check_latitude() failed
    if (check_latitude_error == -1)
    {
      return 0;
    }
    
    // if check_longitude() failed
    if (check_longitude_error == -1)
    {
      printf("Invalid NMEA String...4\n");
      return 0;
    }
    
    
    // pull everything together into a datastring and print
    sprintf(datastring, "%s,%s,%d,%f,%s,%f,%s,%d,%s,%d,%s", identifier, time, counter, new_latitude, north_south, new_longitude, east_west, lock, satellites, flightmode, altitude);
    unsigned int CHECKSUM = gps_CRC16_checksum(datastring);  // Calculates the checksum for this datastring
    char checksum_str[6];
    sprintf(checksum_str, "*%04X\n", CHECKSUM);
    strcat(datastring,checksum_str);
    rtty_txstring (datastring); 
  }
  else
  {
    return 0;
  }
}
  
// Send a byte array of UBX protocol to the GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for(int i=0; i<len; i++) {
    Serial.write(MSG[i]);
  }
  Serial.println();
}


boolean getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;	// header
  ackPacket[1] = 0x62;	// header
  ackPacket[2] = 0x05;	// class
  ackPacket[3] = 0x01;	// id
  ackPacket[4] = 0x02;	// length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];	// ACK class
  ackPacket[7] = MSG[3];	// ACK id
  ackPacket[8] = 0;		// CK_A
  ackPacket[9] = 0;		// CK_B
 
  // Calculate the checksums
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while (1) {
 
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      return true;
    }
 
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      return false;
    }
 
    // Make sure data is available to read
    if (Serial.available()) {
      b = Serial.read();
 
      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
      } 
      else {
        ackByteID = 0;	// Reset and look again, invalid order
      }
 
    }
  }
}

// disables all NMEA sentences except $GPGGA
void setupGPS()
{
  Serial.println("$PUBX,40,GLL,0,0,0,0*5C");
  delay(1000);
  Serial.println("$PUBX,40,GGA,0,0,0,0*5A");
  delay(1000);
  Serial.println("$PUBX,40,GSA,0,0,0,0*4E");
  delay(1000);
  Serial.println("$PUBX,40,RMC,0,0,0,0*47");
  delay(1000);
  Serial.println("$PUBX,40,GSV,0,0,0,0*59");
  delay(1000);
  Serial.println("$PUBX,40,VTG,0,0,0,0*5E");
  delay(1000);
}

 
void setup()
{
  Serial.begin(9600);
  pinMode(RADIOPIN, OUTPUT);
  setupGPS();
}

void loop()
{
  char NMEA_string[70] = "";
  char buffer = 0;
  int n = 0;
  int state = 0;
  byte flightmode_status = 0;
 
  while (1){
    buffer = Serial.read(); // Read Character
 
    switch(state){
      case 0: // Waiting for a $
        if (buffer == '$'){
          state=1;
          n=0;
          memset(NMEA_string,0,70); // Clear NMEA_sentence
        }
      case 1:   // Got a string
        if (buffer == '\r'){  // End of string
          flightmode_status = 0;
          sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));
          flightmode_status = getUBX_ACK(setNav);
          parse_NMEA(NMEA_string, flightmode_status);
          state=0;
        } else {
          NMEA_string[n++]=buffer;
        }
        if (buffer >=68) {  // Stop at 68 (69 chars) so there's still a NULL at the end.
          state=0;
        }
      break;
    } // Close switch
  } // Close while
}
 