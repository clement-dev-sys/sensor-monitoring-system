#include <Arduino.h>
#include <SPI.h>
#include <Ethernet3.h>

#define ETH_CS_PIN 15
#define ETH_RST_PIN 22

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);
//IPAddress dns(192, 168, 69, 1);

void setup() {
	Serial.begin(115200);
	delay(2000);
	Serial.println("\n=== Test W5500 Ethernet ===");

	//SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
	pinMode(ETH_CS_PIN, OUTPUT);
	digitalWrite(ETH_CS_PIN, HIGH);

	pinMode(ETH_RST_PIN, OUTPUT);
	digitalWrite(ETH_RST_PIN, LOW);
	delay(100);
	digitalWrite(ETH_RST_PIN, HIGH);
	delay(1500);

	SPI.begin();
	SPI.setClockDivider(SPI_CLOCK_DIV16);

	/*Serial.println("Test des pins...");
	pinMode(ETH_CS_PIN, OUTPUT);
	digitalWrite(ETH_CS_PIN, HIGH);
	Serial.println("CS initialisé");

	pinMode(ETH_RST_PIN, HIGH);
	digitalWrite(ETH_RST_PIN, HIGH);
	Serial.println("RST initialisé");*/

	Serial.println("Initialisation W5500...");
	Ethernet.begin(mac, ip, ETH_CS_PIN);

	delay(2000);

	Serial.print("Adresse MAC : ");
	for (int i = 0; i < 6; i++) {
		if (mac[i] < 16) Serial.print("0");
		Serial.print(mac[i], HEX);
		if (i < 5) Serial.print(":");
	}
	Serial.println();

	Serial.print("Adresse IP : ");
	Serial.println(Ethernet.localIP());

	if (Ethernet.localIP()[0] = 255) {
		Serial.println("ERREUR : Echec initialisation !");
	} else {
		Serial.println("Bonne adresse IP");
	}

	Serial.println("\n=== W5500 initialisé ! ===\n");
}

void loop() {
	Serial.print("Ethernet actif - IP : ");
	Serial.println(Ethernet.localIP());

	delay(5000);
}
