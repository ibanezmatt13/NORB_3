#include <stdio.h>
#include <string.h>
#include <stdlib.h>
 
 

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
int parse_NMEA(char* mystring, int flightmode)
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
  if (sscanf(mystring, "%6[^,],%11[^,],%11[^,],%1[^,],%11[^,],%1[^,],%d,%2[^,],%*[^,],%9[^,]", identifier, time, latitude, north_south, longitude, east_west, &lock, satellites, altitude) == 9)
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
      printf("Invalid NMEA String...4\n");
      return 0;
    }
    
    
    // pull everything together into a datastring and print
    sprintf(datastring, "%s,%s,%d,%f,%f,%d,%s,%d,%s", identifier, time, counter, new_latitude, new_longitude, lock, satellites, flightmode, altitude);
    printf(datastring);
  }
  else
  {
	printf("sscanf() function failure");
    return 0;
  }
}
  








int main(void)
{
  char NMEA_string[90] = "";
  char buffer = 0;
  int n = 0;
  int state = 0;
  int flightmode_status = 0;
 
  while (1){
    buffer = getchar(); // Read Character
 
    switch(state){
      case 0: // Waiting for a $
        if (buffer == '$'){
          state=1;
          n=0;
          memset(NMEA_string,0,90); // Clear NMEA_sentence
          NMEA_string[n++]=buffer;
        }
        break;
      case 1:   // Got a string
        if ((buffer=='\r') || (buffer == '\n')){  // End of string
          flightmode_status = 0;
          parse_NMEA(NMEA_string, flightmode_status);
          state=0;
        } else {
          NMEA_string[n++]=buffer;
        }
        if (buffer >= 88) {  // Stop at 88 (89 chars) so there's still a NULL at the end.
            printf("Buffer Overflow");
			state=0;
        }
      break;
    } // Close switch
  } // Close while
}
