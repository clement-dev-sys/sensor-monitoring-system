#include "mqtt_subscriber.h"

// ===== VARIABLES GLOBALES =====
sqlite3 *db = NULL;
sqlite3_stmt *insert_stmt = NULL;
Config app_config = {0};
time_t last_message_time = 0;

// ===== DATE UTC =====

void getUTCTimestamp(char *buffer, size_t size)
{
  time_t now = time(NULL);
  struct tm *utc_time = gmtime(&now);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", utc_time);
}

// ===== SEUILS =====

void displaySeuils(const Config *cfg)
{
  printf("\nSeuils d'alerte configurés:\n");
  printf("Température : %.1f°C à %.1f°C\n",
         cfg->thresholds.temp_min, cfg->thresholds.temp_max);
  printf("Pression : %.1f à %.1f hPa\n",
         cfg->thresholds.press_min, cfg->thresholds.press_max);
  printf("Humidité : %d%% à %d%%\n",
         cfg->thresholds.hum_min, cfg->thresholds.hum_max);
}

void logAlert(const Config *cfg, const char *capteur, double valeur, const char *type, double seuil)
{
  char alert_path[1024];
  config_resolve_path(cfg, cfg->logging.alert_file, alert_path, sizeof(alert_path));

  FILE *f = fopen(alert_path, "a");
  if (!f)
  {
    fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", alert_path);
    return;
  }

  char timeStr[64];
  getUTCTimestamp(timeStr, sizeof(timeStr));

  fprintf(f, "[%s] ALERTE %s : %s = %.1f (seuil %s : %.1f)\n",
          timeStr, type, capteur, valeur, type, seuil);
  fclose(f);

  printf("ALERTE %s : %s = %.1f (seuil %s : %.1f)\n",
         type, capteur, valeur, type, seuil);
}

void checkSeuils(const Config *cfg, double temp, double press, int hum)
{
  if (temp < cfg->thresholds.temp_min)
  {
    logAlert(cfg, "Température", temp, "MIN", cfg->thresholds.temp_min);
  }
  else if (temp > cfg->thresholds.temp_max)
  {
    logAlert(cfg, "Température", temp, "MAX", cfg->thresholds.temp_max);
  }

  if (press < cfg->thresholds.press_min)
  {
    logAlert(cfg, "Pression", press, "MIN", cfg->thresholds.press_min);
  }
  else if (press > cfg->thresholds.press_max)
  {
    logAlert(cfg, "Pression", press, "MAX", cfg->thresholds.press_max);
  }

  if (hum < cfg->thresholds.hum_min)
  {
    logAlert(cfg, "Humidité", (double)hum, "MIN", (double)cfg->thresholds.hum_min);
  }
  else if (hum > cfg->thresholds.hum_max)
  {
    logAlert(cfg, "Humidité", (double)hum, "MAX", (double)cfg->thresholds.hum_max);
  }
}

// ===== BASE DE DONNÉES =====

int initDatabase(const Config *cfg)
{
  int rc;
  struct stat buffer;

  char db_path[1024];
  config_resolve_path(cfg, cfg->database.path, db_path, sizeof(db_path));

  int new_db = (stat(db_path, &buffer) != 0);

  rc = sqlite3_open(db_path, &db);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Erreur ouverture DB : %s\n", sqlite3_errmsg(db));
    return rc;
  }

  char *errMsg = NULL;

  sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA temp_store=MEMORY;", NULL, NULL, NULL);
  sqlite3_exec(db, "PRAGMA auto_vacuum=INCREMENTAL;", NULL, NULL, NULL);

  const char *table_sql =
      "CREATE TABLE IF NOT EXISTS mesures ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "timestamp TEXT NOT NULL,"
      "temperature REAL,"
      "pression REAL,"
      "humidite INTEGER"
      ");";

  rc = sqlite3_exec(db, table_sql, NULL, NULL, &errMsg);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Erreur création table : %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  const char *index_sql =
      "CREATE INDEX IF NOT EXISTS idx_mesures_timestamp "
      "ON mesures(timestamp);";

  rc = sqlite3_exec(db, index_sql, NULL, NULL, &errMsg);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Erreur création index : %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  if (new_db)
  {
    rc = sqlite3_exec(db, "VACUUM;", NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
      fprintf(stderr, "Erreur VACUUM initial : %s\n", errMsg);
      sqlite3_free(errMsg);
      return rc;
    }
  }

  const char *insert_sql =
      "INSERT INTO mesures (timestamp, temperature, pression, humidite) "
      "VALUES (?, ?, ?, ?);";

  rc = sqlite3_prepare_v2(db, insert_sql, -1, &insert_stmt, NULL);
  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Erreur préparation statement : %s\n", sqlite3_errmsg(db));
    return rc;
  }

  printf("Base de données prête (%s)\n", db_path);
  return SQLITE_OK;
}

int insertData(double temp, double press, int hum)
{
  if (!insert_stmt)
  {
    fprintf(stderr, "Statement non initialisé\n");
    return SQLITE_ERROR;
  }

  char timestamp[64];
  getUTCTimestamp(timestamp, sizeof(timestamp));

  sqlite3_bind_text(insert_stmt, 1, timestamp, -1, SQLITE_TRANSIENT);
  sqlite3_bind_double(insert_stmt, 2, temp);
  sqlite3_bind_double(insert_stmt, 3, press);
  sqlite3_bind_int(insert_stmt, 4, hum);

  int rc = sqlite3_step(insert_stmt);

  sqlite3_reset(insert_stmt);
  sqlite3_clear_bindings(insert_stmt);

  if (rc != SQLITE_DONE)
  {
    fprintf(stderr, "Erreur insertion : %s\n", sqlite3_errmsg(db));
    return rc;
  }

  return SQLITE_OK;
}

void closeDatabase(void)
{
  if (insert_stmt)
  {
    sqlite3_finalize(insert_stmt);
    insert_stmt = NULL;
  }

  if (db)
  {
    sqlite3_close(db);
    db = NULL;
  }
}

// ===== JSON =====

int parseAndStore(const Config *cfg, const char *jsonString)
{
  struct json_object *parsed_json;
  struct json_object *temp_obj, *press_obj, *hum_obj;

  parsed_json = json_tokener_parse(jsonString);

  if (parsed_json == NULL)
  {
    printf("Erreur parsing JSON\n");
    return -1;
  }

  if (!json_object_object_get_ex(parsed_json, "temperature", &temp_obj) ||
      !json_object_object_get_ex(parsed_json, "pression", &press_obj) ||
      !json_object_object_get_ex(parsed_json, "humidite", &hum_obj))
  {
    printf("JSON incomplet\n");
    json_object_put(parsed_json);
    return -1;
  }

  double temperature = json_object_get_double(temp_obj);
  double pression = json_object_get_double(press_obj);
  int humidite = json_object_get_int(hum_obj);

  printf("Données parsées :\n");
  printf(" - Température : %.1f °C\n", temperature);
  printf(" - Pression : %.1f hPa\n", pression);
  printf(" - Humidité : %d %%\n", humidite);

  checkSeuils(cfg, temperature, pression, humidite);

  int result = insertData(temperature, pression, humidite);

  if (result == SQLITE_OK)
  {
    printf("Message enregistré\n");
  }

  json_object_put(parsed_json);
  return result;
}

// ===== MQTT =====

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
  char *payload = (char *)message->payload;

  printf("\n=== Message reçu ===\n");

  char readable_ts[64];
  getUTCTimestamp(readable_ts, sizeof(readable_ts));

  printf("Date, Heure : %s\n", readable_ts);
  printf("Topic : %s\n", topicName);

  last_message_time = time(NULL);

  parseAndStore(&app_config, payload);

  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);

  return 1;
}

void connectionLost(void *context, char *cause)
{
  printf("\nConnexion MQTT perdue : %s\n", cause);
  printf("Tentative de reconnexion automatique...\n");
}

// ===== WATCHDOG =====

int checkMessageTimeout(int timeout_seconds)
{
  if (last_message_time == 0)
  {
    return 0;
  }

  time_t now = time(NULL);
  time_t elapsed = now - last_message_time;

  if (elapsed > timeout_seconds)
  {
    printf("\nTIMEOUT : Aucun message depuis %ld secondes\n", elapsed);
    return 1;
  }

  return 0;
}

// ===== MAIN =====

int main(int argc, char *argv[])
{
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

  printf("=== Subscriber MQTT ===\n");

  const char *config_file = (argc > 1) ? argv[1] : "config.toml";

  if (config_load(&app_config, config_file) != 0)
  {
    fprintf(stderr, "Erreur : impossible de charger %s\n", config_file);
    exit(EXIT_FAILURE);
  }

  config_display(&app_config);

  char current_time[64];
  getUTCTimestamp(current_time, sizeof(current_time));
  printf("Heure système UTC : %s\n\n", current_time);

  displaySeuils(&app_config);

  if (initDatabase(&app_config) != SQLITE_OK)
  {
    exit(EXIT_FAILURE);
  }

  MQTTClient_create(&client, app_config.mqtt.broker_address,
                    app_config.mqtt.client_id,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);

  conn_opts.keepAliveInterval = app_config.mqtt.keepalive_interval;
  conn_opts.cleansession = 1;

  printf("Connexion au broker MQTT (%s)...\n", app_config.mqtt.broker_address);
  if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS)
  {
    printf("Échec connexion broker\n");
    closeDatabase();
    exit(EXIT_FAILURE);
  }

  printf("Connecté au broker\n");

  printf("Abonnement à : %s (QoS %d)\n", app_config.mqtt.topic, app_config.mqtt.qos);
  MQTTClient_subscribe(client, app_config.mqtt.topic, app_config.mqtt.qos);
  printf("Abonné\n\n");

  printf("En attente des données ESP32...\n");

  int watchdog_timeout = 60;

  while (1)
  {
    sleep(10);

    if (checkMessageTimeout(watchdog_timeout))
    {
      char alert_path[1024];
      config_resolve_path(&app_config, app_config.logging.alert_file, alert_path, sizeof(alert_path));

      FILE *f = fopen(alert_path, "a");
      if (f)
      {
        char ts[64];
        getUTCTimestamp(ts, sizeof(ts));
        fprintf(f, "[%s] WATCHDOG : ESP32 ne répond plus\n", ts);
        fclose(f);
      }

      last_message_time = time(NULL);
    }
  }

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  closeDatabase();

  return 0;
}