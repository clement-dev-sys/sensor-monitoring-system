# Sensor Monitoring System

Système embarqué de monitoring environnemental avec ESP32 et capteur BME280 ainsi que transmission Ethernet avec W5500.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Language: C/C++](https://img.shields.io/badge/Language-C%2FC%2B%2B-orange.svg)](https://isocpp.org/)

## Objectifs

Acquisition en temps réel de données environnementales : température, humidité, pression. Puis transmission réseau et stockage des données avec système d'alertes automatique.

## Technologies

- **Embarqué** : ESP32, W5500 (Ethernet), BME280, C++, PlatformIO
- **Serveur** : C, MQTT (Paho), SQLite3, TOML, Makefile
- **Système** : Arch Linux, Mosquitto broker, Bash

## Architecture

```
ESP32 --- MQTT --- Broker Mosquitto --- MQTT --- Subscriber --- SQLite
(BME280)           (localhost:1883)              (Unix server)  (data/)
(W5500)            (192.168.69.0/24)                               |
                                                                   |--- Alertes (alertes.log)
```

## Câblage

### W5500 - ESP32

| W5500 | ESP32 GPIO | Description  |
|-------|------------|--------------|
| 3.3V  | 3.3V       | Alimentation |
| GND   | GND        | Masse        |
| MOSI  | 23         | SPI Data Out |
| MISO  | 19         | SPI Data In  |
| SCLK  | 18         | SPI Clock    |
| SCS   | 5          | Chip Select  |
| RST   | 4          | Reset        |

### BME280 - ESP32

| BME280 | ESP32 GPIO | Description  |
|--------|------------|--------------|
| VCC    | 3.3V       | Alimentation |
| GND    | GND        | Masse        |
| SCL    | 22         | I2C Clock    |
| SDA    | 21         | I2C Data     |

## Structure du projet

```
sensor-monitoring-system/
|-- esp32/                        # Code C++ ESP32 (PlatformIO)
|   |-- main.cpp                    # Publisher MQTT + gestion réseau
|   |-- main.h                      # Configurations et définitions
|   |-- platformio.ini              # Config PlatformIO
|-- server/                       # Serveur C de réception
|   |-- mqtt_subscriber.c           # Subscriber MQTT + stockage
|   |-- config.c                    # Parser configuration TOML
|   |-- mqtt_subscriber.h           # Configurations et définitions
|   |-- config.h                    # Configurations et définitions
|-- data/                         # Base de données (SQLite3)
|   |-- donnees_esp32.db            # Mesures environnementales
|   |-- alertes.log                 # Journal des alertes
|-- scripts/                      # Scripts utilitaires
|   |-- network.sh                  # Validation configuration réseau
|   |-- cleanbd.sh                  # Cleanup base de données
|   |-- cleanbd.log                 # Journal de rotation des données
|-- Makefile                      # Build automatique pour le serveur
|-- config.toml                   # Fichier de configuration centralisé
|-- Sensor_monitoring_system.pdf  # Documentation complète
|-- README.md
```

## Installation

### Prérequis système

**OS requis :** Arch Linux (ou dérivé avec `pacman` et `yay`)

> **Note :** Le projet peut fonctionner sur d'autres distributions Linux, mais les commandes d'installation devront être adaptées (apt, dnf, etc.)

### 1. Configuration réseau

Le système utilise un réseau Ethernet local statique. Si votre configuration diffère, modifiez :

**Dans `config.toml`** :
```toml
[network]
interface_server = "enp0s25"    # Interface Ethernet
ip_server = "192.168.69.1"      # IP du serveur
ip_esp = "192.168.69.2"         # IP de l'ESP32

[mqtt]
broker_address = "tcp://localhost:1883"
```

**Dans `esp32/main.cpp`** :
```cpp
IPAddress ip(192, 168, 69, 2);           // IP ESP32
IPAddress gateway(192, 168, 69, 1);      // Gateway (serveur)
IPAddress subnet(255, 255, 255, 0);      
IPAddress mqttServer(192, 168, 69, 1);   // IP du broker MQTT
```

### 2. Installation PlatformIO (pour ESP32)

```bash
# Via yay
yay -S platformio
```

Les bibliothèques ESP32 nécessaires sont automatiquement gérées par PlatformIO :
- `Adafruit BME280 Library` (BME280)
- `Ethernet` (support W5500)
- `PubSubClient` (MQTT)
- `ArduinoJson` (Json)

Elles sont déclarées dans `esp32/platformio.ini` et installées au premier build.

### 3. Installation Mosquitto (broker MQTT)

```bash
# Installation
sudo pacman -S mosquitto

# Utilisation
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Vérification
systemctl status mosquitto
```

### 4. Installation des dépendances serveur

Le projet fournit un Makefile avec installation automatique :

```bash
# Installation automatique de toutes les dépendances
make deps
```

Cette commande installe :
- **Via pacman :** `mosquitto`, `json-c`, `sqlite`
- **Via yay (AUR) :** `paho-mqtt-c`, `tomlc99`

> **Note :** Si `yay` n'est pas installé, installez-le d'abord ou installez manuellement `paho-mqtt-c` et `tomlc99` depuis AUR.

### 5. Compilation

```bash
# Compiler le serveur
make

# Ou voir toutes les options
make help
```

## Utilisation

### Commandes Makefile

```bash
make help        # Afficher l'aide
make deps        # Installer les dépendances
make             # Compiler le projet
make run         # Compiler et lancer
make clean       # Nettoyer build/
make cleanall    # Nettoyer tout (data/ inclus)
```

### Configuration principale (`config.toml`)

**Seuils d'alerte** (ajustez selon vos besoins) :
```toml
[thresholds.temperature]
min = 17.0    # Alerte si < 17°C
max = 24.0    # Alerte si > 24°C

[thresholds.pression]
min = 980.0   # Alerte si < 980 hPa
max = 1030.0  # Alerte si > 1030 hPa

[thresholds.humidite]
min = 40      # Alerte si < 40%
max = 70      # Alerte si > 70%
```

**Rétention des données** :
```toml
[database]
retention_hours = 3          # Garder 3h de données
cleanup_batch_size = 2000    # Supprimer par lots de 2000
```

**Affichage** :
```toml
[affichage]
display = false    # true pour afficher les messages reçus sur le terminal serveur
```

### Démarrage du système

#### 1. Démarrer le serveur

```bash
# Validation réseau + lancement
make run

# Ou manuellement
bash scripts/network.sh    # Valider la config réseau
./build/mqtt_subscriber config.toml
```

#### 2. Flasher l'ESP32

```bash
cd esp32

# Compiler et flasher
pio run -t upload

# Vérification : moniteur série (optionnel)
pio run -t monitor
```

### Consultation des données

#### Base de données SQLite

```bash
# Ouvrir la base
sqlite3 data/donnees_esp32.db

# Dernières mesures
SELECT * FROM mesures ORDER BY timestamp DESC LIMIT 10;

# Moyennes sur la dernière heure
SELECT 
  AVG(temperature) as temp_moy,
  AVG(pression) as press_moy,
  AVG(humidite) as hum_moy
FROM mesures 
WHERE timestamp > datetime('now', '-1 hour');

# Quitter
.quit
```

**Structure de la table `mesures` :**
```sql
CREATE TABLE mesures (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp TEXT NOT NULL,           -- Format: "YYYY-MM-DD HH:MM:SS" (UTC)
  temperature REAL,                  -- °C
  pression REAL,                     -- hPa
  humidite INTEGER                   -- %
);
```

#### Fichier d'alertes

```bash
# Voir les dernières alertes
tail -f data/alertes.log
```

**Format des alertes :**
```
[2025-01-02 14:23:15] ALERTE : Température trop basse (16.8°C < 17.0°C)
[2025-01-02 14:28:20] Température revenue à la normale (17.1°C)
[2025-01-02 15:10:45] ALERTE : Humidité trop élevée (72% > 70%)
```

### Maintenance automatique

#### Cleanup manuel

```bash
# Supprimer les données de plus de 3h
bash scripts/cleanbd.sh
```

**Sortie attendue :**
```
[2025-01-02 14:30:00] === Nettoyage ===
[2025-01-02 14:30:01] Batch supprimé : 2000 lignes
[2025-01-02 14:30:02] Batch supprimé : 160 lignes
[2025-01-02 14:30:03] Total supprimé : 2160 lignes
[2025-01-02 14:30:04] === Terminé ===
```

#### Automation avec crontab

Pour un cleanup automatique toutes les heures :

```bash
# Éditer crontab
crontab -e

# Ajouter cette ligne (adapter le chemin)
0 * * * * /path/to/sensor-monitoring-system/scripts/cleanbd.sh
```

#### Journalisation

```bash
# La rotation automatique est journalisée, pour la consulter voir 
cat scripts/cleanbd.log
```

### Validation réseau

En cas de problème de connexion :

```bash
# Vérification de la communication réseau entre l'ESP et le Serveur
bash scripts/network.sh
```

## Documentation complète

Voir `Sensor_monitoring_system.pdf` pour la documentation technique détaillée.

## Licence

MIT License - voir [LICENSE](LICENSE)
