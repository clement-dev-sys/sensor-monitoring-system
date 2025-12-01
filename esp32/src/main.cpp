#include <Arduino.h>

#define LED_PIN 2

void setup() {
	Serial.begin(115200);		// INIT
	delay(1000);
	Serial.println("\n=== ESP32 démarré ! ===");

	pinMode(LED_PIN, OUTPUT);	// LED
}

void loop() {
	digitalWrite(LED_PIN, HIGH);
	Serial.println("LED ON");
	delay(1000);

	digitalWrite(LED_PIN, LOW);
	Serial.println("LED OFF");
	delay(1000);
}
