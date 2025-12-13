# Documentation ESP32 Publisher MQTT

## Vue d'ensemble

L'ESP32 collecte des données environnementales (température, pression, humidité, luminosité) via des capteurs I2C et envoie les données via MQTT toutes les 20 secondes vers le serveur Unix.

---

## Matériel requis

- **ESP32 DevKit V1**
- **Module W5500**
- **Capteur BME280**
- **Capteur BH1750**
- **Câbles de connexion Dupont**
- **Câble Ethernet**
- **Câble USB**

### Câblage W5500 - ESP32

| W5500 | ESP32 GPIO | Description  |
|-------|------------|--------------|
| 3.3V  | 3.3V       | Alimentation |
| GND   | GND        | Masse        |
| MOSI  | 23         | SPI Data Out |
| MISO  | 19         | SPI Data In  |
| SCLK  | 18         | SPI Clock    |
| SCS   | 5          | Chip Select  |
| RST   | 4          | Reset        |

### Câblage BME280 - ESP32
TODO

### Câblage BH1750 - ESP32
TODO

---

## Logiciels requis

### PlatformIO

### Bibliothèques nécessaires

- Ethernet - Pour W5500
- PubSubClient - Pour MQTT
- ArduinoJson - Pour JSON
- TODO

---

## Configuration

```cpp
// Configuration réseau
IPAddress ip(192, 168, 69, 2);              // IP de l'ESP32
IPAddress gateway(192, 168, 69, 1);         // IP du PC serveur
IPAddress mqttServer(192, 168, 69, 1);      // IP du PC serveur

// Configuration MQTT
const char* deviceId = "ESP32_001";       // Identifiant unique
const char* mqttTopic = "esp32/data";     // Topic MQTT
const long interval = 20000;              // Fréquence (20s)
```

---

## Format des données JSON

```json
{
  "device_id": "ESP32_001",
  "temperature": 22.5,
  "pression": 1013.2,
  "humidite": 65,
  "luminosite": 45.3
}
```

---
