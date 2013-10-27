#include <SHT1x.h>
 
#define RADIOPIN 9
 
// Specify data and clock connections and instantiate SHT1x object
 
#define dataPin A4
#define clockPin A5
SHT1x sht1x(dataPin, clockPin);
 
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
 
  //50 baud...
  delayMicroseconds(10000);  
  delayMicroseconds(10000); 
                            
} 
 
void setup()
{
  pinMode(RADIOPIN, OUTPUT);
}
 
void loop()
{
  float temp_c;
  float humidity;
  char temp_string[10];
  char humidity_string[10];
  char datastring[40];
 
  // Read values from the sensor
  temp_c = sht1x.readTemperatureC();
  humidity = sht1x.readHumidity();
  
  dtostrf(temp_c,2,2,temp_string);
  dtostrf(humidity,2,2,humidity_string);
  
  sprintf(datastring, "Temperature: %s, Humidity: %s\n", temp_string, humidity_string);
  rtty_txstring(datastring);
 
}
