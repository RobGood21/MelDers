/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88



*/

#include <EEPROM.h>
#include <FastLED.h>

#define aantalpix 9 //=9x3x6=162 melders?


CRGB pix[aantalpix];

unsigned long periode;
byte pc_count = 0;
byte lastread;
byte melders[21];
boolean clear;

void setup() {
	Serial.begin(9600);
	//FastLED.addLeds<NEOPIXEL, 8>(pix, aantalpix);
	FastLED.addLeds<WS2811, 8>(pix, aantalpix);

	//set pins
	DDRB |= (1 << 0); //pin8 output
	DDRB &= ~(1 << 1); //pin 9 als data in, 
	PORTB &= (~1 << 1); //no pullup, high active
}


void loop() {

	if (millis() - periode > 10) {
		periode = millis();

	
			//readpix();
			pc_count++;
			if (pc_count > aantalpix * 3) {
				pc_count = 0;
				//Serial.print("*");
			}
			setpix();
			readpix();

		
	}
}

void setpix() {
	int number = pc_count;
	byte px = 0;

	//if (number < 3)px = number;

	while (number > 2) {
		px++;
		number -= 3;
	}


	pix[px] = 0x00000;
	pix[px - 1] = 0x000000;

	switch (number) {
	case 0:
		pix[px].r = 255;
		break;
	case 1:
		pix[px].g = 255;
		break;
	case 2:
		pix[px].b = 255;
		break;
	}
	FastLED.show();
}

void readpix() { //leest PortC alleen de On. 
	//alleen aanzetten, uitzetten gebeurt na een uitlezing door de S88
	//bepalen welke melder wordt gelezen
	//pc_count led in de daisy chain 0~3xaantalpix
	byte read = PINC;
	//Serial.println(pc_count);

	//read = read << 2; read = read >> 2; //bit 6 en 7 uitfilteren


	for (byte i = 0; i < 6; i++) { //6 leeslijnen bekijken	

		if (read & (1 << i)) { //melder actief

			//melder = pc_count + (i * aantalpixels * 3)
			setmelder(pc_count + (i * aantalpix * 3));
		}
	}

	//	Serial.println(read,BIN);

	//byte changed = lastread ^ read;
	//if (changed & (1<< 0) && read & (1<<0)) Serial.println(read,BIN);
	//lastread = read;
	//FastLED.clear();
	//FastLED.show();
}

void setmelder(byte melder) {

	Serial.println(melder);

}