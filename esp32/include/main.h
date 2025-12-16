#ifndef ESP32_MQTT_PUBLISHER_H
#define ESP32_MQTT_PUBLISHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>

// ===== CONFIG W5500 =====
#define ETH_CS 5
#define ETH_MOSI 23
#define ETH_MISO 19
#define ETH_SCK 18
#define ETH_RST 4

// ===== CONFIG RÉSEAU =====
extern byte mac[6];
extern IPAddress ip;
extern IPAddress gateway;
extern IPAddress subnet;

// ===== CONFIG MQTT =====
extern IPAddress mqttServer;
extern const int mqttPort;
extern const char *mqttTopic;
extern const char *deviceId;

// ===== OBJETS =====
extern EthernetClient ethClient;
extern PubSubClient mqttClient;

// ===== TIMING =====
extern unsigned long previousMillis;
extern const long interval;

// ===== FONCTIONS =====

/**
 * @brief Initialise la connexion Ethernet avec le module W5500
 */
void initEthernet();

/**
 * @brief Configure et connecte le client MQTT
 */
void setupMQTT();

/**
 * @brief Reconnecte au broker MQTT si la connexion est perdue
 */
void reconnectMQTT();

/**
 * @brief Génère et envoie des données de capteurs via MQTT
 * @return true si l'envoi a réussi, false sinon
 */
bool sendSensorData();

/**
 * @brief Affiche les informations réseau dans le Serial
 */
void displayNetworkInfo();

#endif // ESP32_MQTT_PUBLISHER_H