# Sensor Monitoring System

Système embarqué de monitoring environnemental avec ESP32, transmission Ethernet avec W5500 et interface graphique temps réel.

## Objectifs

Acquisition temps réel de données environnementales : température, humidité, pression et luminosité. Puis transmission réseau et visualisation des données.

## Technologies

- **Embarqué** : ESP32, W5500, capteurs I2C
- **Serveur** : C (sockets réseau, SQLite)
- **Interface** : Python Qt5, matplotlib
- **Système** : Arch Linux

## Structure
```
|--- esp32/     # Code ESP32 (ArduinoIDE)
|--- server/    # Serveur C de réception
|--- data/      # Base de donnée (SQLite3)
|--- gui/       # Interface graphique PyQt
|--- scripts/   # Scripts utilitaires
|--- docs/      # Documentation
```

## Etat du projet

En cours de développement...

## Licence

MIT
