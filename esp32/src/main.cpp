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
Adafruit_BME280 bme;

unsigned long previousMillis = 0;
const long interval = 20000; // 20 secondes

unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 5000;

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

bool initBME280() {
  Serial.println("Initialisation du BME280...");
  
  if (bme.begin(0x76)) {
    Serial.println("BME280 détecté à l'adresse 0x76");
    
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
    return true;
  }
  
  if (bme.begin(0x77)) {
    Serial.println("BME280 détecté à l'adresse 0x77");
    
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
    return true;
  }
  
  Serial.println("ERREUR: BME280 non détecté!");
  Serial.println("Vérifiez le câblage I2C (SDA, SCL)");
  return false;
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

bool reconnectMQTT() {
  // Reconnexion non-bloquante
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    
    if (now - lastReconnectAttempt > reconnectInterval) {
      lastReconnectAttempt = now;
      Serial.print("Connexion au broker MQTT...");

      if (mqttClient.connect("ESP32_Publisher")) {
        Serial.println(" OK !");
        return true;
      } else {
        Serial.print(" Échec (code ");
        Serial.print(mqttClient.state());
        Serial.println(")");
        return false;
      }
    }
  }
  return mqttClient.connected();
}

bool sendSensorData() {
  bme.takeForcedMeasurement();
  
  float temperature = bme.readTemperature();
  float pression = bme.readPressure() / 100.0F;
  float humidite = bme.readHumidity();

  if (isnan(temperature) || isnan(pression) || isnan(humidite)) {
    Serial.println("ERREUR: Lecture capteur invalide");
    return false;
  }

  StaticJsonDocument<256> doc;
  doc["device_id"] = deviceId;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["pression"] = round(pression * 10) / 10.0;
  doc["humidite"] = round(humidite * 10) / 10.0;

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

  Wire.begin();

  if (!initBME280()) {
    Serial.println("Système en pause - BME280 requis");
    while (true) delay(1000);
  }

  initEthernet();
  displayNetworkInfo();
  setupMQTT();

  Serial.println("Prêt à envoyer des données toutes les 20s");
}

void loop() {
  reconnectMQTT();

  mqttClient.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (mqttClient.connected()) {
      sendSensorData();
    } else {
      Serial.println("MQTT déconnecté - en attente de reconnexion...");
    }
  }
}