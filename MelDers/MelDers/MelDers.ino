/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88



*/

#include <EEPROM.h>




unsigned long periode;
byte bitcount=0;
byte bytecount=0;
byte aantalbytes=1;

void setup() {
	Serial.begin(9600);

	//set pins
	DDRB |= (1 << 3); //pin 11 serial out MOSI
	DDRB |= (1 << 5); //Pin 13 shift clock

	//init

}

void loop() {
	if (millis() - periode > 1) {
		periode = millis();		
		shift();

	}


}

void shift() {
//telkens 1 bit inschuiven of verplaatsen.
	if (bitcount == 0 && bytecount == 0)PORTB |= (1 << 3); //serial pin high
	PINB |= (1 << 5); PINB |= (1 << 5); //make shift 
	bitcount++;
	if (bitcount > 7) {
		bitcount = 0;
		bytecount++;
		if (bytecount == aantalbytes)bytecount = 0;
	}
	PORTB &= ~(1 << 3); //reset serial pin
}


void readMelder() { 

}

void setmelder(byte melder) {

	Serial.println(melder);

}