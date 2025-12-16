# Sensor Monitoring System

Système embarqué de monitoring environnemental avec : ESP32 et Capteur BME280, transmission Ethernet avec W5500 et interface graphique temps réel.

## Objectifs

Acquisition en temps réel de données environnementales : température, humidité, pression. Puis transmission réseau et visualisation des données.

## Technologies

- **Embarqué** : ESP32, W5500, capteur BME280, C++
- **Serveur** : C, MQTT, SQLite3
- **Interface** : Python Qt5, matplotlib
- **Système** : Arch Linux

## Structure
```
|--- esp32/     # Code C++ ESP32 (PlatformIO)
|--- server/    # Serveur C de réception
|--- data/      # Base de donnée (SQLite3)
|--- gui/       # Interface graphique (PyQt5)
|--- scripts/   # Scripts utilitaires
|--- docs/      # Documentation
```

## Documentation

- [Documentation ESP32 Publisher](docs/ESP32_Publisher.md)
- [Documentation Serveur Unix](docs/Serveur_Unix.md)
- [Documentation Interface Graphique](docs/GUI_PyQt5.md)

## Etat du projet

En cours de développement...

## Licence

MIT
