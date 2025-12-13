# Sensor Monitoring System

Système embarqué de monitoring environnemental avec : ESP32 et Capteurs I2C, transmission Ethernet avec W5500 et interface graphique temps réel.

## Objectifs

Acquisition temps réel de données environnementales : température, humidité, pression et luminosité. Puis transmission réseau et visualisation des données.

## Technologies

- **Embarqué** : ESP32, W5500, capteurs I2C
- **Serveur** : C, MQTT, SQLite3
- **Interface** : Python Qt5, matplotlib
- **Système** : Arch Linux

## Structure
```
|--- esp32/     # Code ESP32 (PlatformIO)
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
