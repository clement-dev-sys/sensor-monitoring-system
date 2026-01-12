#include "main.h"

// ===== DÉFINITION DES VARIABLES GLOBALES =====
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress mqttServer(192, 168, 69, 1);
const int mqttPort = 1883;
const char *mqttTopic = "esp32/data";
const int mqttQoS = 1;

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
Adafruit_BME280 bme;

unsigned long previousMillis = 0;
const long interval = 5000;
unsigned long lastReconnectAttempt = 0;
const long reconnectInterval = 15000;

unsigned long consecutiveFailures = 0;
const unsigned long max_failures = 3;

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

void checkNetworkStatus()
{
  Serial.println("\n=== Diagnostic Réseau ===");

  auto link = Ethernet.linkStatus();
  Serial.print("Lien Ethernet : ");
  if (link == LinkON)
  {
    Serial.println("  Connecté");
  }
  else if (link == LinkOFF)
  {
    Serial.println("  Déconnecté - Câble débranché ?");
  }
  else
  {
    Serial.println("  Inconnu");
  }

  Serial.print("IP actuelle   : ");
  IPAddress currentIP = Ethernet.localIP();
  Serial.println(currentIP);

  if (currentIP == IPAddress(0, 0, 0, 0))
  {
    Serial.println("  IP invalide - Problème config");
  }

  Serial.print("W5500 Hardware: ");
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("  Non détecté");
  }
  else
  {
    Serial.println("  Détecté");
  }

  Serial.println("======================\n");
}

void resetSystem()
{
  Serial.println("\nRESET COMPLET DU SYSTÈME");

  Serial.println("  Fermeture MQTT...");
  mqttClient.disconnect();
  delay(500);

  Serial.println("  Reset W5500...");
  digitalWrite(ETH_RST, LOW);
  delay(200);
  digitalWrite(ETH_RST, HIGH);
  delay(500);

  Serial.println("  Redémarrage ESP32...");
  delay(2000);

  ESP.restart();
}

bool reconnectMQTT()
{
  if (!mqttClient.connected())
  {
    unsigned long now = millis();

    if (now - lastReconnectAttempt > reconnectInterval)
    {
      lastReconnectAttempt = now;

      checkNetworkStatus();
      Serial.print("Connexion au broker MQTT...");

      if (mqttClient.connect("ESP32_Publisher"))
      {
        Serial.println("OK !");
        consecutiveFailures = 0;
        return true;
      }
      else
      {
        Serial.print(" Échec (code ");
        Serial.print(mqttClient.state());
        Serial.println(")");

        consecutiveFailures++;
        Serial.printf("Échecs consécutifs : %lu/%lu\n", consecutiveFailures, max_failures);

        if (consecutiveFailures >= max_failures)
        {
          Serial.println("\nTrop d'échecs - Reset système requis");
          resetSystem();
        }

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
    return true;
  }
  else
  {
    Serial.println("=== Échec ===");
    return false;
  }
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

  Serial.printf("\nConfiguration :\n");
  Serial.printf("  - Intervalle envoi : %lu ms\n", interval);
  Serial.printf("  - Intervalle reconnexion : %lu ms\n", reconnectInterval);
  Serial.printf("  - Max échecs avant reset : %lu\n\n", max_failures);

  Serial.println("Prêt à envoyer des données...");
}

void loop()
{
  Ethernet.maintain();

  reconnectMQTT();

  mqttClient.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    if (mqttClient.connected())
    {
      sendSensorData();
    }
    else
    {
      Serial.println("MQTT déconnecté - en attente de reconnexion...");
    }
  }
}