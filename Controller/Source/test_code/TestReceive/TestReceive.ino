/*
  ATmega128RFA1 Dev Board Basic Chat
 */

#include "RadioFunctions.h"
#include <Wire.h>
#include <EEPROM.h>
#include <limits.h>


// Joystick values
union {
  struct stick{
    uint16_t ROLL;
    uint16_t PITCH;
    uint16_t YAW;
    uint16_t THROTTLE;
    uint16_t AUX1;
    uint16_t AUX2;
    uint16_t AUX3;
    uint16_t AUX4;
  } 
  m;
  uint16_t rc_channels[8];
  char buff[sizeof(struct stick)];
} 
stick_struct;

char txData[RF_BUFFER_SIZE];
char rxData[RF_BUFFER_SIZE];
char tmpBuf[32];	  // Just for printing

//Multiwii rx data
#define RC_CHANS 8
int16_t rcData[RC_CHANS];
int CH_PINS[] = {
  A0,A1,A2,A3};
unsigned int CH_LOWS[] = {
  270,254,665,713};   //Lowest analogRead value: pushing sticks down or left
unsigned int CH_HIGHS[] = {
  892,780,57,183};  //Highest value
byte CH_SWAPPED[] = {
  0,0,1,1};   //Gimbal values are reversed for some reason
unsigned int ADDR_START = 1337;



int buttonWait(int pin, unsigned int time){
  unsigned long start=millis();
  while(digitalRead(pin)){
    if(millis() - start > time)
      return 1;
  }
  return 0;
}

//In range 1000-2000?
int checkRC(unsigned int rcval){
  if(rcval >= 1000 && rcval <= 2000){
    return 0;
  }
  else{
    return 1;
  } 
}


//The setup function is called once at startup of the sketch
void setup()
{

  pinMode(6, INPUT); 
  pinMode(7,INPUT);

  // Add your initialization code here
  Serial.begin(9600);  // Start up serial
  Serial1.begin(9600); // Start up serial1 -- used for debug.
  rfBegin(21);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)

  //Initialize values with some data
  stick_struct.m.ROLL = 1500;
  stick_struct.m.PITCH = 1600;
  stick_struct.m.YAW = 1500;
  stick_struct.m.THROTTLE = 1700;
  stick_struct.m.AUX1 = 1800;
  stick_struct.m.AUX2 = 1900;
  stick_struct.m.AUX3 = 2000;
  stick_struct.m.AUX4 = 2100;

  Serial.print("bit:   ");
  Serial.println(RX_CRC_VALID);
  delay(3000);

}

unsigned long lastMillis = 0;
uint16_t stick_values[8];

unsigned long good=0;

// The loop function is called in an endless loop
void loop()
{

  if (rfAvailable())  // This is what Im going to receive
  {
    //Check if at least there is enough data.
    if((rfAvailable()-2) == sizeof(stick_struct)) {

      // Pass data to multiwii motors array
      memcpy(rcData, rfRead(), sizeof(stick_struct));
      // Print received data


      int bad=0;
      for(int i=0;i<8;i++){
        bad |= checkRC(rcData[i]);
      }

      if(bad){
        sprintf(tmpBuf, "ROLL = %i \n\r", rcData[0]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "PTICH = %i \n\r", rcData[1]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "YAW = %i \n\r", rcData[2]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "THROTTLE = %i \n\r", rcData[3]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "AUX1 = %i \n\r", rcData[4]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "AUX2 = %i \n\r", rcData[5]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "AUX3 = %i \n\r", rcData[6]);
        Serial.print(tmpBuf);  // ... send it out serial.
        sprintf(tmpBuf, "AUX4 = %i \n\r", rcData[7]);
        Serial.print(tmpBuf);  // ... send it out serial.
        delay(500);   
      }
      else{
        good++;
      }

      if(millis() - lastMillis > 1000){
        Serial.println(good);
        lastMillis = millis();
      }

    } 
    else {
//      Serial.print("Error data size is incorrect: ");
//      Serial.println(rfAvailable());
    }
  }
}






