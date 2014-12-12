/*
  ATmega128RFA1 Dev Board Basic Chat
 */

#include "RadioFunctions.h"
#include <Wire.h>
#include "LCD_C0220BiZ.h"
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
ST7036 lcd = ST7036 ( 2, 20, 0x78 );


//Was lazy, copy pasta'd these functions
//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void EEPROMWriteInt(int p_address, int p_value)
{
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int EEPROMReadInt(int p_address)
{
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}  



int buttonWait(int pin, unsigned int time){
  unsigned long start=millis();
  while(digitalRead(pin)){
    if(millis() - start > time)
      return 1;
  }
  return 0;
}


void calibrate(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrating");
  lcd.setCursor(1,0);
  lcd.print("Hold to finish");  
  delay(4000);

  unsigned long lcdMillis=0;
  int i, val;
  for(i=0;i<4;i++){
    CH_LOWS[i]=INT_MAX;
    CH_HIGHS[i]=0;
  }

  while(1){
    for(i=0;i<4;i++){
      val = analogRead(CH_PINS[i]);
      if(val < CH_LOWS[i])
        CH_LOWS[i]=val;
      if(val > CH_HIGHS[i])
        CH_HIGHS[i] = val;
    }

    if(buttonWait(7, 2000)){   //Quit if the button gets held down
      //Write the stuff to EEPROM
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Saving...");
      for(i=0;i<4;i++){
        EEPROMWriteInt(ADDR_START + 4*i,CH_LOWS[i]);
        EEPROMWriteInt(ADDR_START + 4*i + 2,CH_HIGHS[i]);
      }
      delay(2000);
      return;
    }

    if(millis() - lcdMillis > 500){
      lcdMillis = millis();
      lcd.clear();
      for(i=0;i<4;i++){
        lcd.setCursor(0,5*i);
        lcd.print(CH_LOWS[i]);
        lcd.setCursor(1,5*i);
        lcd.print(CH_HIGHS[i]);
      }
    }
  }

}



//The setup function is called once at startup of the sketch
void setup()
{

  lcd.init ();
  lcd.setContrast(10);  
  delay(300);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Don't crash!");
  delay(5000);
  lcd.clear();
  pinMode(6, INPUT); 
  pinMode(7,INPUT);

  for(int i=0;i<4;i++){
    CH_LOWS[i] = EEPROMReadInt(ADDR_START + 4*i);
    CH_HIGHS[i] = EEPROMReadInt(ADDR_START + 4*i+2);
  }  

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
}

unsigned long lastMillis = 0;
uint16_t stick_values[8];

// The loop function is called in an endless loop
void loop()
{
  if(buttonWait(7,2000))
    calibrate();


  if(abs(millis()  - lastMillis) > 150){
    lastMillis = millis();
    lcd.setCursor(0,0);
    lcd.print("                    ");
    for(int i=0;i<4;i++){
      lcd.setCursor(0,i*5);
      lcd.print(stick_struct.rc_channels[i]);
    }
    lcd.setCursor(1,0);
    lcd.print("ROLL PITC YAW  THRO");
  }  


  for(int i=0;i<4;i++){
    int val = analogRead(CH_PINS[i]);
    int low,high;
    if(CH_SWAPPED[i]){
      low = CH_HIGHS[i];
      high = CH_LOWS[i];
    }
    else{
      low = CH_LOWS[i];
      high=CH_HIGHS[i];
    }
    val = map(val,low,high,1000,2000);
    stick_struct.rc_channels[i] = val;
  }


  if(digitalRead(6)){
    rfPrint("sup");
  }


  // Get the values from the joysticks ...


  // Copy the values to transmit buffer
  memcpy(&txData[0], stick_struct.buff, sizeof(stick_struct));
  txData[sizeof(stick_struct)]='\0';

  // Send Data in a serial format with null end
  rfPrint(txData);

  // Wait, don't send data each loop

  if (rfAvailable())  // This is what Im going to receive
  {
    Serial.print("a");
    //    strcpy(rxData, rfRead());
    //
    //    //Check if at least there is enough data.
    //    if(strlen(rxData) >= sizeof(stick_struct)) {
    //
    //      // Pass data to multiwii motors array
    //      memcpy(rcData, rxData, sizeof(stick_struct));
    //
    //      // Print received data
    //      sprintf(tmpBuf, "ROLL = %i \n\r", rcData[0]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "PTICH = %i \n\r", rcData[1]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "YAW = %i \n\r", rcData[2]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "THROTTLE = %i \n\r", rcData[3]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "AUX1 = %i \n\r", rcData[4]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "AUX2 = %i \n\r", rcData[5]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "AUX3 = %i \n\r", rcData[6]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //      sprintf(tmpBuf, "AUX4 = %i \n\r", rcData[7]);
    //      Serial.print(tmpBuf);  // ... send it out serial.
    //    } 
    //    else {
    //      Serial.print("Error data size is incorrect \n\r");
    //    }
  }


}



