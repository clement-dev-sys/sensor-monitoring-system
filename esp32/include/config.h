/**
 * @file config.h
 * @brief Configuration réseau et MQTT pour ESP32 Publisher
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== CONFIG W5500 =====
#define ETH_CS 5    // Chip Select
#define ETH_MOSI 23 // Master Out Slave In
#define ETH_MISO 19 // Master In Slave Out
#define ETH_SCK 18  // Serial Clock
#define ETH_RST 4   // Reset (optionnel mais recommandé)

// ===== CONFIG RÉSEAU =====
#define MAC_ADDR {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}
#define IP_ESP32 192, 168, 1, 177
#define IP_GATEWAY 192, 168, 1, 1
#define IP_SUBNET 255, 255, 255, 0

// ===== CONFIG MQTT =====
#define MQTT_SERVER 192, 168, 1, 100
#define MQTT_PORT 1883
#define MQTT_TOPIC "esp32/data"
#define DEVICE_ID "ESP32_001"
#define MQTT_QOS 1

// ===== CONFIG DELAI =====
#define SEND_INTERVAL 20000
#define MQTT_RECONNECT_DELAY 5000

// ===== CONFIG SÉRIE =====
#define SERIAL_BAUD 115200

#endif // CONFIG_H
