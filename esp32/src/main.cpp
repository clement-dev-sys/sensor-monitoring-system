#include "main.h"

// ===== DÉFINITION DES VARIABLES GLOBALES =====
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress mqttServer(192, 168, 69, 1);
const int mqttPort = 1883;
const char *mqttTopic = "esp32/data";
const char *deviceId = "ESP32_001";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

unsigned long previousMillis = 0;
const long interval = 20000; // 20 secondes

// ===== IMPLÉMENTATION DES FONCTIONS =====

void initEthernet() {
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(100);
  digitalWrite(ETH_RST, HIGH);
  delay(200);

  SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
  Ethernet.init(ETH_CS);

  Serial.println("Connexion Ethernet...");
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(2000);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("ERREUR: W5500 non détecté!");
    while (true)
      delay(1);
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
}

void displayNetworkInfo() {
  Serial.print("IP ESP32 : ");
  Serial.println(Ethernet.localIP());
  Serial.print("ID ESP32 : ");
  Serial.println(deviceId);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connexion au broker MQTT...");

    if (mqttClient.connect("ESP32_Publisher")) {
      Serial.println(" OK !");
    } else {
      Serial.print(" Échec (code ");
      Serial.print(mqttClient.state());
      Serial.println("). Nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

bool sendSensorData() {
  float temperature = random(100, 400) / 10.0;
  float pression = random(9500, 10500) / 10.0;
  int humidite = random(30, 90);

  StaticJsonDocument<256> doc;
  doc["device_id"] = deviceId;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["pression"] = round(pression * 10) / 10.0;
  doc["humidite"] = humidite;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  Serial.print("\n=== Envoi ===\n");
  Serial.println(jsonBuffer);

  if (mqttClient.publish(mqttTopic, jsonBuffer)) {
    Serial.println("=== Envoyé ===");
    return true;
  } else {
    Serial.println("=== Échec ===");
    return false;
  }
}

// ===== SETUP ET LOOP =====

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 Publisher MQTT ===");

  initEthernet();
  displayNetworkInfo();
  setupMQTT();

  Serial.println("Prêt à envoyer des données toutes les 20s");
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    sendSensorData();
  }
}