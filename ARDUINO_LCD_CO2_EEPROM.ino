/*
 Basic Arduino example for K-Series sensor
 Created by Jason Berger
 Co2meter.com
*/
#include "SoftwareSerial.h"
#include <LiquidCrystal.h> 
#include <EEPROM.h>


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 2);

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;

SoftwareSerial K_30_Serial(12,13); //Sets up a virtual serial port
  //Using pin 12 for Rx and pin 13 for Tx
  byte readCO2[] = {0xFE, 0X44, 0X00, 0X08, 0X02, 0X9F, 0X25}; //Command packet to read Co2 (see app note)
byte response[] = {0,0,0,0,0,0,0}; //create an array to store the response
//multiplier for value. default is 1. set to 3 for K-30 3% and 10 for K-33 ICB
int valMultiplier = 1;


void setup()
{
 // put your setup code here, to run once:
lcd.begin(16, 2); //iniciar LCD
lcd.setCursor(4,0); 
lcd.print("Starting");
delay(500);
 
 // write a 0 to all 4096 bytes of the EEPROM (for ATmega 2560)
 for (int i = 0; i < 4096; i++)
 EEPROM.write(i, 0);
  
 // mensage on LCD   
 lcd.clear();
 lcd.setCursor(2,0);
 lcd.print("EEPROM CLEAR");
 delay(1000);
   
 Serial.begin(9600); //Opens the main serial port to communicate with the computer
 K_30_Serial.begin(9600); //Opens the virtual serial port with a baud of 9600
}
void loop()
{
 sendRequest(readCO2);
 unsigned long valCO2 = getValue(response);
 Serial.print("Co2 ppm = ");
 Serial.println(valCO2);
 
 //write LCD
 lcd.clear();
 lcd.setCursor(4,0);
 lcd.print("CO2 PPM");
 lcd.setCursor(6,1);
 lcd.print(valCO2);
 
 //write EEPROM
 // need to divide by 4 because analog inputs range from
  // 0 to 1023 and each byte of the EEPROM can only hold a
  // value from 0 to 255.
  int val = valCO2 / 20;
  
 // write the value to the appropriate byte of the EEPROM.
 // these values will remain there when the board is
 // turned off.
  EEPROM.write(addr,val);
 
 // advance to the next address.  there are 4096 bytes in
 // the EEPROM, so go back to 0 when we hit 4096.
 addr = addr + 1;
 if (addr == 4096)
 addr = 0;
 
 delay(2000); 
  
 }
void sendRequest(byte packet[])
{
 while(!K_30_Serial.available()) //keep sending request until we start to get a response
 {
 K_30_Serial.write(readCO2,7);
 delay(50);
 }
 int timeout=0; //set a timeoute counter
 while(K_30_Serial.available() < 7 ) //Wait to get a 7 byte response
 {
 timeout++;
 if(timeout > 10) //if it takes to long there was probably an error
 {
 while(K_30_Serial.available()) //flush whatever we have
 K_30_Serial.read();
 break; //exit and try again
 }
 delay(50);
 }
 for (int i=0; i < 7; i++)
 {
 response[i] = K_30_Serial.read();
 }
}
unsigned long getValue(byte packet[])
{
 int high = packet[3]; //high byte for value is 4th byte in packet in the packet
 int low = packet[4]; //low byte for value is 5th byte in the packet
 unsigned long val = high*256 + low;  //Combine high byte and low byte with this formula to get value
 return val* valMultiplier;
}

    

