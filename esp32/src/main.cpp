#include "main.h"

// ===== DÉFINITION DES VARIABLES GLOBALES =====
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress mqttServer(192, 168, 69, 1);
const int mqttPort = 1883;
const char *mqttTopic = "esp32/env";
const int mqttQoS = 1;

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Adafruit_BME280 bme;

unsigned long previousMillis = 0;
const long interval = 5000; // 5 secondes

unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 10000;

unsigned long messagesEnvoyes = 0;
unsigned long messagesEchoues = 0;

// ===== IMPLÉMENTATION DES FONCTIONS =====

bool initEthernet()
{
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

  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("ERREUR : W5500 non détecté!");
    return false;
  }

  return true;
}

bool initBME280()
{
  Serial.println("Initialisation du BME280...");

  if (bme.begin(0x76))
  {
    Serial.println("BME280 détecté à l'adresse 0x76");

    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
    return true;
  }

  if (bme.begin(0x77))
  {
    Serial.println("BME280 détecté à l'adresse 0x77");

    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::FILTER_OFF);
    return true;
  }

  Serial.println("ERREUR : BME280 non détecté!");
  return false;
}

void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);

  mqttClient.setBufferSize(512);

  Serial.println("MQTT configuré (QoS 1)");
}

void displayNetworkInfo()
{
  Serial.print("IP ESP32 : ");
  Serial.println(Ethernet.localIP());
}

bool reconnectMQTT()
{
  if (!mqttClient.connected())
  {
    unsigned long now = millis();

    if (now - lastReconnectAttempt > reconnectInterval)
    {
      lastReconnectAttempt = now;
      Serial.print("Connexion au broker MQTT...");

      if (mqttClient.connect("ESP32_Publisher"))
      {
        Serial.println("OK !");
        if (messagesEnvoyes > 0 || messagesEchoues > 0)
          Serial.printf("Stats: %lu envoyés, %lu échecs\n", messagesEnvoyes, messagesEchoues);
        return true;
      }
      else
      {
        Serial.print(" Échec (code ");
        Serial.print(mqttClient.state());
        Serial.println(")");
        return false;
      }
    }
  }
  return mqttClient.connected();
}

bool sendSensorData()
{
  bme.takeForcedMeasurement();

  float temperature = bme.readTemperature();
  float pression = bme.readPressure() / 100.0F;
  float humidite = bme.readHumidity();

  if (isnan(temperature) || isnan(pression) || isnan(humidite))
  {
    Serial.println("ERREUR : Lecture capteur invalide");
    return false;
  }

  JsonDocument doc;
  doc["temperature"] = round(temperature * 10) / 10.0;
  doc["pression"] = round(pression * 10) / 10.0;
  doc["humidite"] = round(humidite * 10) / 10.0;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  Serial.print("\n=== Envoi ===\n");
  Serial.println(jsonBuffer);

  bool success = mqttClient.publish(mqttTopic, jsonBuffer, false);

  if (success)
  {
    Serial.println("=== Envoyé ===");
    messagesEnvoyes++;
    return true;
  }
  else
  {
    Serial.println("=== Échec ===");
    messagesEchoues++;
    return false;
  }
}

void displayStats()
{
  unsigned long uptime = millis() / 1000;

  Serial.println("\n=== Statistiques ===");
  Serial.printf("Uptime: %lu s\n", uptime);
  Serial.printf("Messages envoyés: %lu\n", messagesEnvoyes);
  Serial.printf("Messages échoués: %lu\n", messagesEchoues);

  if (messagesEnvoyes + messagesEchoues > 0)
  {
    float successRate = (float)messagesEnvoyes / (messagesEnvoyes + messagesEchoues) * 100.0;
    Serial.printf("Taux de succès: %.1f%%\n", successRate);
  }

  Serial.println("====================\n");
}

// ===== SETUP ET LOOP =====

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 Publisher MQTT ===");

  Wire.begin();

  if (!initBME280())
  {
    Serial.println("Système en pause - BME280 requis");
    while (true)
      delay(1000);
  }

  if (!initEthernet())
  {
    Serial.println("Système en pause - W5500 requis");
    while (true)
      delay(1000);
  }

  displayNetworkInfo();
  setupMQTT();

  Serial.println("Prêt à envoyer des données...");
}

void loop()
{
  reconnectMQTT();

  mqttClient.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (mqttClient.connected())
    {
      sendSensorData();
      if (messagesEnvoyes % 30 == 0 && messagesEnvoyes > 0)
        displayStats();
    }
    else
    {
      Serial.println("MQTT déconnecté - en attente de reconnexion...");
    }
  }
}