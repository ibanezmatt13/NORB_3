WARNING - There is a slight design issue with the schematic for the PCB which I had to bodge to get the micro SD working. The VSS pins of the connector should be connected to a GND point on the board, which for some reason it isn't. So, I had to solder a wire going from the connector to a ground point on the board; not pretty but it works.





NORB 3
======
NORB 3 is the third generation in the NORB AVR-based HAB tracking PCB series. 


Main Components:
================

NTX2 434MHz Radio Transmitter

Ublox MAX 7 GPS Module

Amtel ATMEGA328P-AU Microcontroller

SHT11 Temperature/Humidity Sensor

TMP102 I2C Temperature Sensor

Micro SD socket

TPS61201 Step-up Boost Regulator

Taoglas GPS Antenna


Info
====

More information including the development, current status and pictures of the tracker can be found at NORB's website:

                                                      
                                                 
                                                  http://www.norb.co.uk
                                                  
                                                  
                                                  

What NORB does...
=================

The current software that runs the NORB board does several things:

1.  It uses a special routine to search repeatedly for certain characters over a serial connection to the Ublox GPS         module. Once it picks up the correct beginning of what could be a valid NMEA sentence, it checks for the letter G to     to see if it is the correct sentence. If it is, it passes the sentence as a string to a parse_NMEA function.

2.  The parse_NMEA function is designed to carefully assess whether the sentence contains any anomalies in certain          fields. This could be a lost character during transfer or the sentence may not actually reflect a full GPS lock.        Should this be the case, the parsing is discontinued and the search for another sentence restarts. If it does get       through however, a few tasks are performed...

3.  The time is checked to assess what format it is in and to assess if it is a valid time value. Then the                  Latitude/longitude is converted into decimal degrees for uploading to the spacenear.us server after decoding. 

4.  After the sentence has been stripped down and individual fields of data have been analysed, the program will request     information from the SHT11 temperature and humidity sensor where upon it will parse the data into a suitable format.     The voltage across the battery is also analysed by scaling the analogue value on the correct analogue pin to a value     within the range 0v to 3.3v.

5.  Next, two strings are constructed: a log_datastring and a send_datastring. The log_datastring contains absolutely       all data including voltage and humidity, which will be logged to the SD card. The send_datastring contains only the     bare minimum of required data for basic telemetry, but I couldn't resist adding temperature in there too. I have        done this to further reduce the probability of a decoding error on the ground. ie, more data coming down = more         chance of a bad sentence.

6.  Then, we log the log_datastring to the SD card, and send the send_datastring down over the radio link by splitting      it up into binary digits to toggle the transmit pin on our transmitter. And there you have it! :D





