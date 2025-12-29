#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>
#include <libgen.h>
#include <unistd.h>
#include <limits.h>

#define PATH 256

// ===== STRUCTURES DE CONFIGURATION =====

typedef struct
{
  char broker_address[256];
  char topic[128];
  char client_id[64];
  int qos;
  int keepalive_interval;
} MqttConfig;

typedef struct
{
  char path[512];
  int retention_hours;
  int cleanup_batch_size;
} DatabaseConfig;

typedef struct
{
  char alert_file[512];
  char cleanup_log[512];
} LoggingConfig;

typedef struct
{
  double temp_min;
  double temp_max;
  double press_min;
  double press_max;
  int hum_min;
  int hum_max;
} ThresholdsConfig;

typedef struct
{
  char data_dir[256];
  char scripts_dir[256];
  char server_dir[256];
} PathsConfig;

// Structure principale
typedef struct
{
  MqttConfig mqtt;
  DatabaseConfig database;
  LoggingConfig logging;
  ThresholdsConfig thresholds;
  PathsConfig paths;
  char project_root[512];
  int display_messages;
} Config;

// ===== FONCTIONS =====

/**
 * @brief Initialise la configuration avec des valeurs par défaut
 * @param cfg Structure de configuration à initialiser
 */
void config_init_defaults(Config *cfg);

/**
 * @brief Charge la configuration depuis un fichier TOML
 * @param cfg Structure de configuration à remplir
 * @param config_file Chemin du fichier de configuration
 * @return 0 si succès, -1 en cas d'erreur
 */
int config_load(Config *cfg, const char *config_file);

/**
 * @brief Résout un chemin relatif par rapport à la racine du projet
 * @param cfg Configuration contenant la racine du projet
 * @param relative_path Chemin relatif
 * @param output Buffer pour stocker le chemin absolu
 * @param output_size Taille du buffer
 */
void config_resolve_path(const Config *cfg, const char *relative_path, char *output, size_t output_size);

/**
 * @brief Affiche la configuration chargée
 * @param cfg Configuration à afficher
 */
void config_display(const Config *cfg);

#endif // CONFIG_H