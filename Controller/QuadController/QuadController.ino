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

void setup()
{
  
  lcd.init ();
  lcd.setContrast(10);  
  delay(300);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz");
  delay(5000);
  Serial.begin(9600);  // Start up serial
  Serial1.begin(115200);
  rfBegin(11);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)
  pinMode(6, INPUT); 
  
  // Send a message to other RF boards on this channel
  rfPrint("ATmega128RFA1 Dev Board Online!\r\n");
}

int pos = 0;

void loop()
{ 
  if (digitalRead(6))  // If serial comes in...
  {
    rfPrint("Sup!"); // ...send it out the radio.
  }
  if (rfAvailable())  // If data receievd on radio...
  {
//    lcd.clear ();
//    lcd.setCursor ( 0, 0 );
    lcd.print (rfRead());
    lcd.setCursor(pos / 20, pos % 20);
    pos++;
    if(pos==40) pos=0;
  }
}





