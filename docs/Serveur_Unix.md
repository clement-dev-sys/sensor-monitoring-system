# Documentation Serveur Unix

## Vue d'ensemble

Le serveur Unix reçoit les données MQTT de l'ESP32, vérifie les seuils d'alerte, et stocke tout dans une base de données SQLite.

---

## Architecture

```
ESP32 -- MQTT -- Broker Mosquitto -- MQTT -- Subscriber -- SQLite
                 (localhost:1883)           (Unix server)  (data/)
                                                |
                                                |--- Alertes (alertes.log)
```

---

## Structure des dossiers

```
projet/
|-- server/
|     |-- make.sh                 # Script de compilation
│     |-- mqtt_subscriber.c       # Code source
|     |-- mqtt_subscriber.h       # Header
│     |-- mqtt_subscriber         # Exécutable compilé
│     |-- seuils.conf             # Configuration des seuils
|-- data/
│     |-- donnees_esp32.db        # Base SQLite
│     |-- alertes.log             # Log des alertes
|--scripts/
      |-- cleanup.sh              # Nettoyage auto (> 3h)
      |-- cleanup.log             # Log du nettoyage
```

---

## Installation

### Dépendances (Arch Linux)
```bash
sudo pacman -S mosquitto paho-mqtt-c sqlite json-c gcc make
```

### Broker MQTT (Mosquitto)

**Configuration :**
```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

Contenu minimal :
```
listener 1883
allow_anonymous true
```

---

## Compilation

```bash
cd server/
./make.sh
```

---

## Configuration des seuils

Fichier : `server/seuils.conf`

```ini
[temperature]
min = 15.0
max = 30.0
description = Température

[pression]
min = 980.0
max = 1030.0
description = Pression atmosphérique

[humidite]
min = 40
max = 70
description = Humidité
```

**Modifier les seuils** : Éditer le fichier et relancer le subscriber.

---

## 🚀 Utilisation

### Lancer le subscriber
```bash
cd server/
./mqtt_subscriber
```

**Sortie attendue :**
```
=== Message reçu ===
Date, Heure : 2025-12-21 12:04:12
Topic : esp32/env
Données parsées :
 - Température : 22.0 °C
 - Pression : 984.3 hPa
 - Humidité : 56 %
=== Message enregistré ===
```

---

## Base de données SQLite

### Structure de la table

```sql
CREATE TABLE mesures (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,        -- UTC : "2025-12-08 14:33:54"
    temperature REAL,               -- °C
    pression REAL,                  -- hPa
    humidite INTEGER,               -- %
);
```

---

## Fichier d'alertes

Fichier : `data/alertes.log`

**Format :**
```
[2025-12-08 14:33:54] ALERTE MAX : Température = 38.5 (seuil MAX : 35.0)
[2025-12-08 14:34:14] ALERTE MIN : Humidité = 28.0 (seuil MIN : 40.0)
```

**Consulter :**
```bash
# Voir toutes les alertes
cat data/alertes.log

# Suivre en temps réel
tail -f data/alertes.log
```

---

## Nettoyage automatique

### Script : `scripts/cleanup.sh`

Supprime automatiquement les données de plus de **3 heures**.

**Utilisation manuelle :**
```bash
cd scripts/
./cleanup.sh
```

**Utilisation automatique :**
```bash
crontab -e

# Ajouter :
# Exécuter toutes les heures
0 * * * * ./home/arch/Projets/sensor-monitoring-system/scripts/cleandb.sh
```

**Log du nettoyage :**
```bash
cat scripts/cleandb.log
```

---

### Réinitialiser la base

```bash
# Sauvegarder d'abord
cp data/donnees_esp32.db data/backup_$(date +%Y%m%d).db

# Supprimer
rm data/donnees_esp32.db

# Relancer le subscriber (recrée auto)
cd server/
./mqtt_subscriber
```

### Réinitialiser les alertes

```bash
# Archiver
mv data/alertes.log data/alertes_archive_$(date +%Y%m%d).log

# Le fichier sera recréé au prochain dépassement de seuil
```

---
