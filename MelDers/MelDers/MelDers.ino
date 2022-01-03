/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88



*/

#include <EEPROM.h>
# define aantalSR 2//4 
# define lines 6 //aantal te lezen invoer lijnen A0~A?
//4x6x8=192 melders


unsigned long periode;
byte bitcount = 0;
byte SRcount = 0; //shiftregister count
//byte lastline[lines];
byte melderstatus[aantalSR*lines]; //aantal max melders

void setup() {
	Serial.begin(9600);

	//set pins
	DDRB |= (1 << 3); //pin 11 serial out MOSI
	DDRB |= (1 << 5); //Pin 13 shift clock

	//init

}

void loop() {
	shift();
	if (millis() - periode > 1000) {
		periode = millis();
		activemelders();
	}

}

void activemelders() {
	byte m = 0;
	//debug toont actieve, aangezette melders
	for (byte i = 0; i < aantalSR*lines; i++) { //alle melderstatus bytes (24)
		for (byte b = 0; b < 8; b++) { //bits in byte
			if (melderstatus[i] & (1 << b)) { //hoog
				m = i * aantalSR * 8 + b;
				Serial.println(m);
			}
		}
	}
	clearmelders();
}

void shift() {
	//telkens 1 bit inschuiven of verplaatsen.
	if (bitcount == 7 & SRcount == aantalSR - 1) PORTB |= (1 << 3); //serial pin high

	PINB |= (1 << 5); PINB |= (1 << 5); //make shift 
	PORTB &= ~(1 << 3); //reset serial pin

	//Serial.println(bitcount);

	read();

	bitcount++;
	if (bitcount > 7) {
		bitcount = 0;
		SRcount++;
		if (SRcount == aantalSR)SRcount = 0;
	}
}
void read() {
	//alleen de AAN meten 
	//inputs high-active
	byte p = PINC; //lees port
	byte m = 0;
	for (byte L = 0; L < lines; L++) {

		if (p & (1 << L)) { //line, input staat hoog, dus 1 of meerdere sensors actief

			//Serial.println(bitcount);
			//welke melder was geselecteerd?
			m = SRcount * 8 + bitcount;
			m = m + L * aantalSR * 8;
			setmelder(m);
		}
	}
}

void setmelder(byte melder) {
	//totaal 192 melders /8=24 status bytes nodig(latchen een actieve sensor)
	byte bi = melder;
	byte by = 0;

	while (bi > 7) {
		by++;
		bi -= 8;
	}

	melderstatus[by] |= (1 << bi);

	//Serial.println(melder);
}

void clearmelders() {
	for (byte i = 0; i < lines*aantalSR; i++) {
		melderstatus[i] = 0x00;
	}
}