/*
 NORB 3 HAB Tracker
 
 By Matthew Beckett
 
 Thanks and credits:
 -------------------
 
 Massive thanks to Steve Smith, David Akerman, Anthony Stirk, Chris Stubbs, Phil Heron, Ed Moore, mfa298, and all other
 brilliant people who assisted in the creation of this code.
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 See <http://www.gnu.org/licenses/>.
 */
 
 
#include <util/crc16.h>
#include <SHT1x.h>
#include <SD.h>

#define RADIOPIN 9
#define LED_1 5
#define LED_2 6  
#define dataPin A4     
#define clockPin A5

File logfile; // file object for SD

SHT1x sht1x(dataPin, clockPin); 

int counter = 0; // sentence id

// this command will set flight mode
uint8_t setNav[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
    0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC };

   
void rtty_txstring (char * string)
{
 
 /* function to send a each character
   of the string to the rtty_txbyte().
   Each character is one byte (8 bits)
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
  /* function to send each bit of the character to the
     rtty_txbit() All chars should be preceded with a 0 and 
     proceded with a 1. 0 = Start bit; 1 = Stop bit
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
  digitalWrite(RADIOPIN, bit);
  //digitalWrite(LED_2, bit);
  //digitalWrite(LED_1, bit);
 
  //50 baud...
  delayMicroseconds(10000);  
  delayMicroseconds(10000); 
                            
}  

void LED_flash() //flash LED 6 times
{
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  delay(500);
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
 

char *ltrim(char *string) { return(*string == ' ' ? string + 1 : string); }


//function to calculate and store temp/humdity in given location in memory
void SHT11(float* temp, float* hum)
{
  float temp_c;
  float humidity;
  
  temp_c = sht1x.readTemperatureC();
  humidity = sht1x.readHumidity();
  
  *temp = temp_c;
  *hum = humidity;
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
int parse_NMEA(char* mystring, int flightmode)
{
  float new_latitude;
  float new_longitude;
  char new_lat[12];
  char new_lon[12];
  char* trimmed_lon;
  char time[12];
  int time_length;
  char identifier[7];
  char latitude[12];
  char north_south[2];
  char longitude[12];
  char east_west[2];
  int lock;
  int satellites;
  char altitude[10];
  char log_datastring[102] = "";
  char send_datastring[102] = "";
  int check_latitude_error;
  int check_longitude_error;
  char* callsign = "$$$$NORB2";
  float vbatt;
  char voltage[10];
  float temp;
  float hum;
  char temp_string[10];
  char hum_string[10];
  
 
  // split NMEA string into individual data fields and check that a certain number of values have been obtained
  // $GPGGA,212748.000,5056.6505,N,00124.3531,W,2,07,1.8,102.1,M,47.6,M,0.8,0000*6B
  if (sscanf(mystring, "%6[^,],%11[^,],%11[^,],%1[^,],%11[^,],%1[^,],%d,%d,%*[^,],%9[^,]", identifier, time, latitude, north_south, longitude, east_west, &lock, &satellites, altitude) == 9)
  {
    // check for the identifer being invalid
    if (strncmp(identifier, "$GPGGA", 6) != 0)
    {
      return 0;
    }
    
    // check for a valid lock
    if (lock == 0)
    {
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
      return 0;
    }
    
    // store the return value of the latitude/longitude functions accordingly
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
      return 0;
    }
    
    
    vbatt = ((3.3 / 1024)* analogRead(0));
    SHT11(&temp, &hum);
    
    // convert floats to strings
    dtostrf(new_latitude,9,6,new_lat);
    dtostrf(new_longitude,9,6,new_lon);
    dtostrf(vbatt,3,2,voltage);
    dtostrf(temp,3,2,temp_string);
    dtostrf(hum,3,2,hum_string);
    
    trimmed_lon = ltrim(new_lon);
    
    // pull data together into a longer string to log to SD card and a shorter string to transmit to earth
    sprintf(log_datastring, "%s,%d,%s,%s,%s,%s,%d,%d,%d,%s,%s,%s", callsign, counter ,time, new_lat, new_lon, altitude, lock, flightmode, satellites, voltage, temp_string, hum_string);
    sprintf(send_datastring, "%s,%d,%s,%s,%s,%s,%d,%d,%d,%s", callsign, counter ,time, new_lat, new_lon, altitude, lock, flightmode, satellites, temp_string);
    
    
    logfile = SD.open("data.txt", FILE_WRITE); // open logfile
    
    if (logfile){  // if log file opened ok
      logfile.println(log_datastring); //write datastring to file
      logfile.close();
    } else {
      logfile.close();
    }
    
    
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    unsigned int CHECKSUM = gps_CRC16_checksum(send_datastring);  // Calculates the checksum for this datastring
    char checksum_str[7];
    sprintf(checksum_str, "*%04X\n", CHECKSUM);
    strcat(send_datastring,checksum_str);
    rtty_txstring (send_datastring); 
    counter++;
  }
  else
  {
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    return 0;
  }
}


 
void setupGPS()
{
  //disable unwanted NMEA sentences
  Serial.println("$PUBX,40,GLL,0,0,0,0*5C");
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
 
// Send a byte array of UBX protocol to the GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for(int i=0; i<len; i++) {
    Serial.write(MSG[i]);
  }
  Serial.println();
}

uint16_t gps_CRC16_checksum (char *string)
{
  size_t i;
  uint16_t crc;
  uint8_t c;
 
  crc = 0xFFFF;
 
  // Calculate checksum ignoring the first two $s
  for (i = 4; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }
 
  return crc;
}    

int getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;        // header
  ackPacket[1] = 0x62;        // header
  ackPacket[2] = 0x05;        // class
  ackPacket[3] = 0x01;        // id
  ackPacket[4] = 0x02;        // length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];        // ACK class
  ackPacket[7] = MSG[3];        // ACK id
  ackPacket[8] = 0;                // CK_A
  ackPacket[9] = 0;                // CK_B
 
  // Calculate the checksums
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while (1) {
 
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      return 1;
    }
 
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      return 0;
    }
 
    // Make sure data is available to read
    if (Serial.available()) {
      b = Serial.read();
 
      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
      } 
      else {
        ackByteID = 0;        // Reset and look again, invalid order
      }
 
    }
  }
}
 
void setup()
{
  Serial.begin(9600);
  pinMode(RADIOPIN, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(10, OUTPUT); // needed for SD library
  setupGPS();
  SD.begin(7);
  LED_flash();
}
 
void loop()
{
  static char datastring[100];
  char character;
  static int n = 0;
  int flightmode = 0;
 
  if (Serial.available()){
    character = Serial.read();
    
    if (character == '$'){
      n = 0;
      datastring[n] = character;
      n = 1;
    }
    
    else if (character == '\n'){
      datastring[n] = '\0';
      flightmode = 0;
      sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t)); // set flight mode
      flightmode = getUBX_ACK(setNav); // check flight mode enabled
      parse_NMEA(datastring, flightmode);
      n = 0;
      flightmode = 0;
    }
    
    else if (n > 0){
      datastring[n] = character;
      n++;
    }
    
    if (n >= 98){
      n = 0;
    }
  }
}
