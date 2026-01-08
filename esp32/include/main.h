#ifndef ESP32_MQTT_PUBLISHER_H
#define ESP32_MQTT_PUBLISHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <time.h>
#include <WiFiUdp.h>

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
extern const int mqttQos;

// ===== OBJETS =====
extern EthernetClient ethClient;
extern PubSubClient mqttClient;
extern Adafruit_BME280 bme;

// ===== TIMING =====
extern unsigned long previousMillis;
extern const long interval;
extern unsigned long lastReconnectAttempt;
extern const long reconnectInterval;

// ===== FAILURE =====
extern unsigned long consecutiveFailures;
extern const unsigned long max_failures;

// ===== FONCTIONS =====

/**
 * @brief Initialise la connexion Ethernet avec le module W5500
 * @return true si l'initialisation est réussite, false sinon
 */
bool initEthernet();

/**
 * @brief Initialise la connexion avec le capteur BME280
 * @return true si l'initialisation est réussite, false sinon
 */
bool initBME280();

/**
 * @brief Configuration NTP
 */
void configureTime();

/**
 * @brief Récupération timestamp UTC
 */
void getUTCTimestamp();

/**
 * @brief Configure et connecte le client MQTT
 */
void setupMQTT();

/**
 * @brief Affiche les informations de configurations réseau
 */
void displayNetworkInfo();

/**
 * @brief Vérifie le réseau en cas d'erreur MQTT
 */
void checkNetworkStatus();

/**
 * @brief Réinitialisation du système si trop d'erreur MQTT
 */
void resetSystem();

/**
 * @brief Reconnection au broker
 * @return true si la reconnection est réussite, false sinon
 */
bool reconnectMQTT();

/**
 * @brief Génère et envoie des données de capteurs via MQTT
 * @return true si l'envoi a réussi, false sinon
 */
bool sendSensorData();

#endif // ESP32_MQTT_PUBLISHER_H