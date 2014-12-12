/*
  ATmega128RFA1 Dev Board Basic Chat
 by: Jim Lindblom
 SparkFun Electronics
 date: July 3, 2012
 License: Beerware. Feel free to use, reuse, and modify this code
 as you please. If you find it useful, you can buy me a beer.
 
 This code sets up the ATmega128RFA1's wireless transciever in
 the most basic way possible to serve as a serial gateway.
 Serial into the ATmega128RFA1's UART0 will go out the RF radio.
 Data into the RF radio will go out the MCU's UART0.
 */

#include <Wire.h>
#include "RadioFunctions.h"
#include "LCD_C0220BiZ.h"
#include <EEPROM.h>
#include <limits.h>

ST7036 lcd = ST7036 ( 2, 20, 0x78 );

/* MultiWii defines the radio channels as follows:
 ROLL,
 PITCH,
 YAW,
 THROTTLE,
 AUX1,
 AUX2,
 AUX3,
 AUX4,
 AUX5,
 AUX6,
 AUX7,
 AUX8
 
 They're all 16 bit integers, the first 4 being 1000-2000 (1500 is middle)
 
 Gimbal channels:
 A0: ROLL
 A1: PITCH
 A2: YAW
 A3: THROTTLE
 
 */

int CH_PINS[] = {
  A0,A1,A2,A3};
unsigned int CH_LOWS[] = {
  270,254,665,713};   //Lowest analogRead value: pushing sticks down or left
unsigned int CH_HIGHS[] = {
  892,780,57,183};  //Highest value
byte CH_SWAPPED[] = {
  0,0,1,1};   //Gimbal values are reversed for some reason
unsigned int ADDR_START = 1337;


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


void setup()
{

  lcd.init ();
  lcd.setContrast(10);  
  delay(300);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Radio test");
  delay(5000);
  lcd.clear();
  Serial.begin(9600);  // Start up serial
  Serial1.begin(115200);
  rfBegin(11);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
  pinMode(6, INPUT); 
  pinMode(7,INPUT);

  for(int i=0;i<4;i++){
    CH_LOWS[i] = EEPROMReadInt(ADDR_START + 4*i);
    CH_HIGHS[i] = EEPROMReadInt(ADDR_START + 4*i+2);
  }

  // Send a message to other RF boards on this channel
  rfPrint("ATmega128RFA1 Dev Board Online!\r\n");
}

int pos = 0;
unsigned long lastMillis = 0;

void loop()
{ 
  if (digitalRead(6))
  {
    rfPrint("Sup!");
  }
  if (rfAvailable())  // If data receievd on radio...
  {
    //    lcd.clear ();
    //    lcd.setCursor ( 0, 0 );
    lcd.print (rfRead());
    lcd.setCursor(1, pos);
    pos = (pos + 1)%20;
  }

  if(buttonWait(7,2000))`
    calibrate();

  if(abs(millis()  - lastMillis) > 200){
    lastMillis = millis();
    lcd.setCursor(0,0);
    lcd.print("                    ");
    for(int i=0;i<4;i++){
      lcd.setCursor(0,i*5);
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
      lcd.print(val);
    }
  }

}












