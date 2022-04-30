/*
 Name:		IRsensor.ino
 Created:	4/15/2022 9:25:52 AM
 Author:	RobAntonisse

 IR sensoren hebben veel last van zonlicht.
 Test project om te zien of er algoritmische oplossingen zijn te maken om dit verschijnsel tegen te gaan.



*/



//tijdelijke variabele
unsigned long tt;
int bt = 500; //bt=brandtijd
bool sensoraan;
//GPIOR0 bit0 =leled aan/uit

void setup() {
	Serial.begin(9600);
	//poorten instellen
	DDRB |= (1 << 0); //pin8 output controle led
	DDRB |= (1 << 1);// PORTB |= (1 << 1);//pin9 output TX led, temp constant aan

	PORTC |= (1 << 0); //pullup op pin A0, is al input


}


void loop() {
	//gpior0 bit0=txled aan of uit
	bool sensor = false;

	if (millis() - tt > 0) { //iedere millisec
		tt = millis();
		if (PINC & (1 << 0)) sensor = true;
		GPIOR0 ^= (1 << 0);

		if (GPIOR0 & (1 << 0)) { //tx aan
			if (sensor == true) {
				sensoraan = true;
			}
			PORTB &= ~(1 << 1); //TXled uit
		}
		else { //txled uit
			if (sensor == false && sensoraan == true) {
				bt = 600; PORTB |= (1 << 0); //indicatie led aan
			}
			sensoraan = false;
			PORTB |= (1 << 1); //TXled aan	
		}

		if (bt > 1) {
			bt--;
		}
		else {
			PORTB &= ~(1 << 0); //led uit
		}
	}

}
