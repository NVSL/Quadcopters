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

int CH_PINS[] = {A0,A1,A3,A2};
int CH_LOWS[] = {270,254,665,713};   //Lowest analogRead value: pushing sticks down or left
int CH_HIGHS[] = {892,780,57,183};  //Highest value
//int CH_OFFSETS[] = {

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

  if(abs(millis()  - lastMillis) > 300){
    lastMillis = millis();
    lcd.setCursor(0,0);
    lcd.print("                    ");
    for(int i=0;i<4;i++){
      lcd.setCursor(0,i*5);
      int val = analogRead(CH_PINS[i]);
      val = map(val,CH_LOWS[i],CH_HIGHS[i],1000,2000);
      lcd.print(val);
    }
  }

}






