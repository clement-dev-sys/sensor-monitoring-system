#ifndef MQTT_SUBSCRIBER_H
#define MQTT_SUBSCRIBER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <MQTTClient.h>
#include <json-c/json.h>
#include <sqlite3.h>
#include "config.h"

// ===== VARIABLES GLOBALES =====
extern sqlite3 *db;
extern sqlite3_stmt *insert_stmt;
extern Config app_config;

typedef struct
{
    int temp_low_active;
    int temp_high_active;
    int press_low_active;
    int press_high_active;
    int hum_low_active;
    int hum_high_active;
} AlertState;

// ===== DATE UTC =====

/**
 * @brief Récupère le timestamp UTC formaté
 * @param buffer Buffer pour stocker le timestamp
 * @param size Taille du buffer
 */
void getUTCTimestamp(char *buffer, size_t size);

// ===== SEUILS =====

/**
 * @brief Affiche les seuils chargés dans la console
 * @param cfg Configuration contenant les seuils
 */
void displaySeuils(const Config *cfg);

/**
 * @brief Vérifie si les valeurs dépassent les seuils et génère des alertes sur transition
 * @param cfg Configuration
 * @param temp Température mesurée
 * @param press Pression mesurée
 * @param hum Humidité mesurée
 */
void checkSeuils(const Config *cfg, double temp, double press, int hum);

/**
 * @brief Enregistre une alerte ou un retour à la normale dans le fichier de log
 * @param cfg Configuration
 * @param message Message à logger
 */
void logAlert(const Config *cfg, const char *message);

// ===== BASE DE DONNÉES =====

/**
 * @brief Initialise la base de données SQLite et prépare les statements
 * @param cfg Configuration
 * @return SQLITE_OK si succès, code d'erreur sinon
 */
int initDatabase(const Config *cfg);

/**
 * @brief Insère des données dans la base de données
 * @param timestamp Timestamp de l'envoi des données
 * @param temp Température
 * @param press Pression
 * @param hum Humidité
 * @return SQLITE_OK si succès, code d'erreur sinon
 */
int insertData(const char *timestamp, double temp, double press, int hum);

/**
 * @brief Ferme proprement la base de données et libère les statements
 */
void closeDatabase(void);

// ===== JSON =====

/**
 * @brief Parse le JSON et stocke les données
 * @param cfg Configuration
 * @param jsonString Chaîne JSON à parser
 * @return 0 si succès, -1 en cas d'erreur
 */
int parseAndStore(const Config *cfg, const char *jsonString);

// ===== MQTT =====

/**
 * @brief Callback appelé lors de la réception d'un message MQTT
 * @param context Contexte utilisateur
 * @param topicName Nom du topic
 * @param topicLen Longueur du nom du topic
 * @param message Message MQTT reçu
 * @return 1 si traité avec succès
 */
int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);

/**
 * @brief Callback appelé lors de la perte de connexion MQTT
 * @param context Contexte utilisateur
 * @param cause Cause de la déconnexion
 */
void connectionLost(void *context, char *cause);

#endif // MQTT_SUBSCRIBER_H
