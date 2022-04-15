/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88



*/

#include <EEPROM.h>
# define aantalSR 4 //4 
# define lines 6 //aantal te lezen invoer lijnen A0~A?
//4x6x8=192 melders


unsigned long shifttimer;
unsigned long cleartimer;

unsigned long periode;
byte bitcount = 0;
byte SRcount = 0; //shiftregister count
//byte lastline[lines];
byte melderstatus[aantalSR*lines]; //aantal max melders
byte lastport;
int countout = 0; //output teller
byte countread = 0;

byte COM_reg; 
//bit0=true S88 detected 




void setup() {
	Serial.begin(9600);
	//set pins
	DDRB |= (1 << 3); //pin 11 serial out MOSI
	DDRB |= (1 << 5); //Pin 13 shift clock

	DDRB |= (1 << 0); //Pin8 data S88

	//pullups to interupt pins

	PCICR |= (1 << 2); //PCIE2 enabled
	PCMSK2 |= (1 << 7); //clock
	PCMSK2 |= (1 << 6); //reset
}

ISR(PCINT2_vect) {
	byte p = PIND;
	p = p >> 6; p = p << 6; // isolate pin6 and 7
	byte changed = p ^ lastport;
	for (byte i = 7; i > 5; i--) {
		if (changed & (1 << i)) { //is pin changed?
			if (p & (1 << i)) { //is pin high? 
				switch (i) {
				case 7: //clock
					clock();
					break;
				case 6: //reset
					reset();
					break;
				}
				//Serial.println(i);
			}
		}
	}
	lastport = p;
}

void clock() { //called from ISR
	//byte/ bit berekenen
	if (countout < lines*aantalSR*4) { //Melders kan tot 192 melders verwerken, S88 kan er 512
		byte n = countout;
		byte b = 0;
		while (n > 7) {
			b++;
			n -= 8;
		}

		if (melderstatus[b] & (1 << n)) {
			PORTB |= (1 << 0); //zet data
		}
		else {
			PORTB &= ~(1 << 0); //reset data
		}
	}
	else { //>192 S88 kan tot 512
		PORTB &= ~(1 << 0); //reset data
	}

	//melderstatus[b] &=~(1 << n); //clear melder buffer
	countout++;
}
void reset() { //called from ISR
	COM_reg |= (1 << 0); //S88 detected

	countout = 0;
	countread++;
	if (countread > 20) {
		countread = 0;
		clearmelders();
	}
}

void loop() {
	if (micros() - shifttimer > 10) {
		shifttimer = micros();
		GPIOR0 ^= (1 << 0);
		if (GPIOR0 & (1 << 0)) {
			shift();
		}
		else {
			read();
		}
	}

	if (~COM_reg & (1 << 0)) {	
	if (millis() - cleartimer > 500) {
		cleartimer = millis();
		clearmelders();
	}
}


}
void shift() {
	//telkens 1 bit inschuiven of verplaatsen.
	if (bitcount == 7 & SRcount == aantalSR - 1) PORTB |= (1 << 3); //serial pin high
	PINB |= (1 << 5); PINB |= (1 << 5); //make shift 
	PORTB &= ~(1 << 3); //reset serial pin
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
	bitcount++;

	if (bitcount > 7) {
		bitcount = 0;
		SRcount++;
		if (SRcount == aantalSR)SRcount = 0;
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
	if (~melderstatus[by] & (1 << bi)) {
		Serial.print("Melder #");Serial.println(melder);
	}	
	melderstatus[by] |= (1 << bi);
}

void clearmelders() {
	for (byte i = 0; i < lines*aantalSR; i++) {
		melderstatus[i] = 0x00;
	}
}