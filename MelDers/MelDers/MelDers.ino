/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88



*/

#include <EEPROM.h>
#include <FastLED.h>
#define aantalpix 3


CRGB pix[aantalpix]; //aantal pixels totaal dus 300 melders

unsigned long periode;
int count = 0;

void setup() {
	Serial.begin(9600);
	FastLED.addLeds<NEOPIXEL, 8>(pix, aantalpix);
}


void loop() {

	if (millis() - periode > 1) {
		periode = millis();
		setpix();
		count++;
		if (count > (aantalpix * 3)) count = 0;
		FastLED.show();
	}
}

void setpix() {
	
	int number = count;
	byte px = 0;

	while (number > 2) {
		px++;
		number = number - 3;
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
}