#ifndef MQTT_SUBSCRIBER_H
#define MQTT_SUBSCRIBER_H

#include <MQTTClient.h>
#include <json-c/json.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// ===== CONFIG MQTT =====
#define ADDRESS "tcp://localhost:1883"
#define CLIENTID "UnixSubscriber"
#define TOPIC "esp32/data"
#define QOS 1

// ===== CHEMINS FICHIERS =====
#define DB_FILE "/home/Arch/Projets/sensor-monitoring-system/data/donnees_esp32.db"
#define ALERT_FILE "/home/Arch/Projets/sensor-monitoring-system/data/alertes.log"
#define CONFIG_FILE "/home/Arch/Projets/sensor-monitoring-system/server/seuils.conf"

// ===== STRUCTURE SEUILS =====
typedef struct {
  double temp_min;
  double temp_max;
  double press_min;
  double press_max;
  int hum_min;
  int hum_max;
} Seuils;

// ===== VARIABLES GLOBALES =====
extern sqlite3 *db;
extern Seuils seuils;

// ===== DATE UTC =====

/**
 * @brief Récupère le timestamp UTC formaté
 * @param buffer Buffer pour stocker le timestamp
 * @param size Taille du buffer
 */
void getUTCTimestamp(char *buffer, size_t size);

/**
 * @brief Récupère le timestamp Unix UTC
 * @return Timestamp Unix en secondes
 */
long getUnixTimestampUTC(void);

// ===== SEUILS =====

/**
 * @brief Charge les seuils depuis le fichier de configuration
 * @return 1 si succès, 0 sinon
 */
int loadSeuils(void);

/**
 * @brief Affiche les seuils chargés dans la console
 */
void displaySeuils(void);

/**
 * @brief Vérifie si les valeurs dépassent les seuils et génère des alertes
 * @param device_id Identifiant de l'appareil
 * @param temp Température mesurée
 * @param press Pression mesurée
 * @param hum Humidité mesurée
 */
void checkSeuils(const char *device_id, double temp, double press, int hum);

/**
 * @brief Enregistre une alerte dans le fichier de log
 * @param device_id Identifiant de l'appareil
 * @param capteur Nom du capteur
 * @param valeur Valeur mesurée
 * @param type Type d'alerte (MIN ou MAX)
 * @param seuil Valeur du seuil dépassé
 */
void logAlert(const char *device_id, const char *capteur, double valeur,
              const char *type, double seuil);

// ===== BASE DE DONNÉES =====

/**
 * @brief Initialise la base de données SQLite
 * @return SQLITE_OK si succès, code d'erreur sinon
 */
int initDatabase(void);

/**
 * @brief Insère des données dans la base de données
 * @param device_id Identifiant de l'appareil
 * @param temp Température
 * @param press Pression
 * @param hum Humidité
 * @return SQLITE_OK si succès, code d'erreur sinon
 */
int insertData(const char *device_id, double temp, double press, int hum);

// ===== JSON =====

/**
 * @brief Parse le JSON et stocke les données
 * @param jsonString Chaîne JSON à parser
 * @return 0 si succès, -1 en cas d'erreur
 */
int parseAndStore(const char *jsonString);

// ===== MQTT =====

/**
 * @brief Callback appelé lors de la réception d'un message MQTT
 * @param context Contexte utilisateur
 * @param topicName Nom du topic
 * @param topicLen Longueur du nom du topic
 * @param message Message MQTT reçu
 * @return 1 si traité avec succès
 */
int messageArrived(void *context, char *topicName, int topicLen,
                   MQTTClient_message *message);

/**
 * @brief Callback appelé lors de la perte de connexion MQTT
 * @param context Contexte utilisateur
 * @param cause Cause de la déconnexion
 */
void connectionLost(void *context, char *cause);

#endif // MQTT_SUBSCRIBER_H