#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CONFIG_FILE="$PROJECT_ROOT/config.toml"

parse_toml() {
  local key=$1
  grep "^$key" "$CONFIG_FILE" | cut -d'=' -f2 | tr -d ' "' | head -1
}

if [ ! -f "$CONFIG_FILE" ]; then
  echo "ERREUR : Fichier de configuration introuvable: $CONFIG_FILE"
  exit 1
fi

SERVER_INTERFACE=$(parse_toml "interface_server")
SERVER_IP=$(parse_toml "ip_server")
ESP32_IP=$(parse_toml "ip_esp")
MQTT_PORT=$(parse_toml "broker_port")

echo "Configuration chargée : "
echo "  Interface serveur : $SERVER_INTERFACE"
echo "  IP serveur        : $SERVER_IP"
echo "  IP ESP32          : $ESP32_IP"
echo "  Port MQTT         : $MQTT_PORT"
echo ""

ERRORS=0

echo "Vérification interface réseau..."
if ip link show "$SERVER_INTERFACE" >/dev/null 2>&1; then
  echo "  Interface $SERVER_INTERFACE existe"
else
  echo "  ERR : Interface $SERVER_INTERFACE introuvable"
  ((ERRORS++))
fi

echo "Vérification adresse IP..."
CURRENT_IP=$(ip -4 addr show "$SERVER_INTERFACE" 2>/dev/null | grep -oP '(?<=inet\s)\d+(\.\d+){3}' | head -1)

if [ -n "$CURRENT_IP" ]; then
  if [ "$CURRENT_IP" = "$SERVER_IP" ]; then
    echo "  IP correcte : $CURRENT_IP"
  else
    echo "  ERR : IP différente : $CURRENT_IP (attendu : $SERVER_IP)"
    ((ERRORS++))
  fi
else
  echo "  ERR : Aucune IP sur $SERVER_INTERFACE"
  ((ERRORS++))
fi

echo "Test connectivité ESP32..."
if ping -c 2 -W 2 "$ESP32_IP" >/dev/null 2>&1; then
  echo "  ESP32 joignable ($ESP32_IP)"
else
  echo "  ERR : ESP32 injoignable ($ESP32_IP)"
  ((ERRORS++))
fi

echo "Vérification broker MQTT..."
if command -v mosquitto >/dev/null 2>&1 || systemctl list-unit-files mosquitto.service >/dev/null 2>&1; then
  if systemctl is-active --quiet mosquitto 2>/dev/null; then
    echo "  Mosquitto actif"
    
    if ss -tuln | grep -q ":$MQTT_PORT "; then
      echo "  Mosquitto écoute sur port $MQTT_PORT"
    else
      echo "  ERR : Mosquitto n'écoute pas sur port $MQTT_PORT"
      ((ERRORS++))
    fi
  else
    echo "  ERR : Mosquitto installé mais inactif"
    ((ERRORS++))
  fi
else
  echo "  ERR : Mosquitto non installé"
  ((ERRORS++))
fi

echo "Test connexion broker MQTT..."
if command -v mosquitto_sub >/dev/null 2>&1; then
  timeout 2 mosquitto_sub -h "$SERVER_IP" -p "$MQTT_PORT" -t "test" -C 1 >/dev/null 2>&1
  if [ $? -eq 124 ] || [ $? -eq 0 ]; then
    echo "  Connexion MQTT fonctionnelle"
  else
    echo "  ERR : Échec connexion MQTT"
    ((ERRORS++))
  fi
fi

echo ""
if [ $ERRORS -eq 0 ]; then
  echo "Configuration réseau OK"
  exit 0
else
  echo "$ERRORS erreur(s) détectée(s)"
  exit 1
fi