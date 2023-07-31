/*
 Name:		LaserSensor.ino
 Created:	4/30/2022 11:10:30 AM


 aruino   >  VL053LXX
 5V naar VIN
 GND > GND
 SCL > A05
 SDA > SDA



*/


#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

int oldafstand;
unsigned long tijd;
int afstand; int dist;



void setup()



{
	Serial.begin(9600);
	Wire.begin();

	sensor.setTimeout(500);

	if (!sensor.init())
	{
		Serial.println("Failed to detect and initialize sensor!");
		while (1) {}
	}
	afstand = sensor.readRangeContinuousMillimeters();
	// Start continuous back-to-back mode (take readings as
	// fast as possible).  To use continuous timed mode
	// instead, provide a desired inter-measurement period in
	// ms (e.g. sensor.startContinuous(100)).
	sensor.startContinuous();
}

void loop() {



	if (millis() - tijd > 100) {
		tijd = millis();



		afstand = sensor.readRangeContinuousMillimeters();
		dist = 0;
		if (oldafstand > afstand) {
			dist = oldafstand - afstand;
		}
		else {
			dist = afstand - oldafstand;
		}

		if (dist > 10) Serial.println(sensor.readRangeContinuousMillimeters());
		oldafstand = afstand;

		//if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

		//Serial.println();
	}
}