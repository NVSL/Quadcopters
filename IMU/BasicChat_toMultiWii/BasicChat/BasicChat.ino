/*
  ATmega128RFA1 Dev Board Basic Chat
*/

#include "RadioFunctions.h"

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
    } m;
    char buff[sizeof(struct stick)];
} stick_struct;

char txData[RF_BUFFER_SIZE];
char rxData[RF_BUFFER_SIZE];
char tmpBuf[32];	  // Just for printing

//Multiwii rx data
#define RC_CHANS 8
int16_t rcData[RC_CHANS];


//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here
	Serial.begin(9600);  // Start up serial
	Serial1.begin(9600); // Start up serial1 -- used for debug.
	rfBegin(11);  // Initialize ATmega128RFA1 radio on channel 11 (can be 11-26)

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

// The loop function is called in an endless loop
void loop()
{
	// Get the values from the joysticks ...

	// Copy the values to transmit buffer
	memcpy(&txData[0], stick_struct.buff, sizeof(stick_struct));
	txData[sizeof(stick_struct)]='\0';

	// Send Data in a serial format with null end
	rfPrint(txData);

	// Wait, don't send data each loop
	delay(1000);

	if (rfAvailable())  // This is what Im going to receive
	{
		strcpy(rxData, rfRead());

		//Check if at least there is enough data.
		if(strlen(rxData) >= sizeof(stick_struct)) {

			// Pass data to multiwii motors array
			memcpy(rcData, rxData, sizeof(stick_struct));

			// Print received data
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
		} else {
			Serial.print("Error data size is incorrect \n\r");
		}
	}


}
