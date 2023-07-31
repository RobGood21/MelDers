/*
 Name:		MelDers.ino
 Created:	12/29/2021 12:05:23 PM
 Author:	Rob Antonisse

 Modelspoor terugmelding via S88


 Via interrupts en een ISR komen de clock en reset van de S88 bus. 
 De Melders module leest geheel onafhankelijk van de S88 de 192 melders en plaatst dit 
 in de buffer melderstatus.
 Met clock wordt 1 voor 1 de melder status uitgezelen en geplaatst op de data pin van de S88
Reset wist de buffer melderstatus. cyclus begint opnieuw. 

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

//V2.0  toevoeging met WMapp
byte CommandCount = 0;
byte AantalBytes = 0;
byte Command[16]; //buffer opslag start met 16bytes

//timer voor de serial read 1ms? 
unsigned long SerialTimer = 0;



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

void SerialRead() {
	//een connected boolean is niet nodig omdat alleen bij een connection met een DCCmonitor WMapp verder gaat.
	int inData;
	while (Serial.available() > 0) {
		inData = Serial.read();

		if (CommandCount > 0) {
			if (CommandCount == 1)AantalBytes = inData;
			Command[CommandCount - 1] = inData;
			CommandCount++;
			if (CommandCount > AantalBytes) { //volledig command
				CommandCount = 0;
				Command_exe(); //voer commando uit
				//Commandclear();
			}
		}
		else if (inData == 255) { //dus niet groter dan 0 dus 0 en 0xFF
			//start byte voor command, niks mee doen dus alleen teller omhoog
			CommandCount++; //volgende doorloop counter op 1			
		}
	}
}
void Command_exe() {
	//eerste byte 255 wordt niet ingelezen, tweede byte = aantalbytes en naar command[0]
    //command[0] = aantalbytes
	//command[1]  1=request data
	//command[2]  1=ProductID
	switch (Command[1]){
	case 1:  //request data 
		switch (Command[1]) {
		case 1: //request voor de productid
			//Stuur productId naar WMapp
			Command[0] = 255; //start com
			Command[1] = 3; //aantal bytes van de boodschap
			Command[2] = 101; //data is product id
			Command[3] = 30; //productId van MelDers
			Serial.write(Command, 4); //aantal te zenden bytes incl. de 'attentie'  byte 255

			break;
		}
		break;


	}
}
void SendMelder(byte melder) {
	Command[0] = 255; //start byte
	Command[1] = 3; //aantal bytes
	Command[2] = 110; //stuur actieve melder
	Command[3] = melder; 
	Serial.write(Command, 4);
}



void clock() { //called from ISR, van S88 poort centrale
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
void reset() { //called from ISR, van S88 centrale

	COM_reg |= (1 << 0); //S88 detected

	countout = 0;
	countread++;
	if (countread > 20) {
		countread = 0;
		clearmelders();
	}
}

void loop() {
	
	//serial zonder timer? 
	SerialRead(); //V2.0



	if (micros() - shifttimer > 10) { //timer lezen shiftregisters 10us per melder
		shifttimer = micros(); //reset timer

		GPIOR0 ^= (1 << 0); //toggle flag
		if (GPIOR0 & (1 << 0)) {
			shift(); //schuif laatste 
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
	//alle melders in 1 lijn gelezen
	if (bitcount == 7 && SRcount == aantalSR - 1) PORTB |= (1 << 3); //serial pin high, een true in de eerste van de lijn

	PINB |= (1 << 5); PINB |= (1 << 5); //make shift 1 stap  
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

	//	Serial.print("Melder #");Serial.println(melder);
		SendMelder(melder);
	}	
	melderstatus[by] |= (1 << bi);
}

void clearmelders() {
	for (byte i = 0; i < lines*aantalSR; i++) {
		melderstatus[i] = 0x00;
	}
}