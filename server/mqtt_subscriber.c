#include "mqtt_subscriber.h"

// ===== VARIABLES GLOBALES =====
sqlite3 *db = NULL;
sqlite3_stmt *insert_stmt = NULL;
Config app_config = {0};
MQTTClient mqtt_client = NULL;

// ===== DATE UTC =====

void getUTCTimestamp(char *buffer, size_t size)
{
  time_t now = time(NULL);
  struct tm *utc_time = gmtime(&now);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", utc_time);
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
      "humidite REAL"
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

int insertData(const char *timestamp, double temp, double press, double hum)
{
  if (!insert_stmt)
  {
    fprintf(stderr, "Statement non initialisé\n");
    return SQLITE_ERROR;
  }

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
  double humidite = json_object_get_double(hum_obj);

  if (app_config.logging.display_messages)
  {
    printf("Données parsées :\n");
    printf(" - Température : %.1f °C\n", temperature);
    printf(" - Pression : %.1f hPa\n", pression);
    printf(" - Humidité : %.1f %%\n", humidite);
  }

  char timestamp[64];
  getUTCTimestamp(timestamp, sizeof(timestamp));

  int result = insertData(timestamp, temperature, pression, humidite);

  if (result == SQLITE_OK)
  {
    if (app_config.logging.display_messages)
    {
      printf("=== Message enregistré ===\n");
    }

    republishWithTimestamp(timestamp, temperature, pression, humidite);
  }

  json_object_put(parsed_json);
  return result;
}

int republishWithTimestamp(const char *timestamp, double temp, double press, double hum)
{
  const char *republish_topic = app_config.mqtt.topic_republish;

  struct json_object *json_obj = json_object_new_object();

  json_object_object_add(json_obj, "timestamp", json_object_new_string(timestamp));

  char temp_str[16];
  snprintf(temp_str, sizeof(temp_str), "%.1f", temp);
  json_object_object_add(json_obj, "temperature", json_object_new_string(temp_str));

  char press_str[16];
  snprintf(press_str, sizeof(press_str), "%.1f", press);
  json_object_object_add(json_obj, "pression", json_object_new_string(press_str));

  char hum_str[8];
  snprintf(hum_str, sizeof(hum_str), "%.1f", hum);
  json_object_object_add(json_obj, "humidite", json_object_new_string(hum_str));

  const char *json_string = json_object_to_json_string(json_obj);

  if (app_config.logging.display_messages)
  {
    printf("Republication sur %s : %s\n", republish_topic, json_string);
  }

  MQTTClient_message pubmsg = MQTTClient_message_initializer;
  pubmsg.payload = (void *)json_string;
  pubmsg.payloadlen = strlen(json_string);
  pubmsg.qos = 1;
  pubmsg.retained = 0;

  MQTTClient_deliveryToken token;
  int rc = MQTTClient_publishMessage(mqtt_client, republish_topic, &pubmsg, &token);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    fprintf(stderr, "Erreur republication MQTT: %d\n", rc);
    json_object_put(json_obj);
    return -1;
  }
  else
  {
    printf("=== Message republié ===\n");
  }

  json_object_put(json_obj);

  return (rc == MQTTCLIENT_SUCCESS) ? 0 : -1;
}

// ===== MQTT =====

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
  (void)context;
  (void)topicLen;

  char *payload = (char *)message->payload;

  if (app_config.logging.display_messages)
  {
    printf("\n=== Message reçu ===\n");

    char readable_ts[64];
    getUTCTimestamp(readable_ts, sizeof(readable_ts));

    printf("Date, Heure : %s\n", readable_ts);
    printf("Topic : %s\n", topicName);
  }

  parseAndStore(&app_config, payload);

  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);

  return 1;
}

void connectionLost(void *context, char *cause)
{
  (void)context;

  printf("\nConnexion MQTT perdue : %s\n", cause);
  printf("Tentative de reconnexion automatique...\n");
}

// ===== MAIN =====

int main(int argc, char *argv[])
{
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

  printf("\n=== Subscriber MQTT ===\n");

  const char *config_file = (argc > 1) ? argv[1] : "config.toml";

  if (config_load(&app_config, config_file) != 0)
  {
    fprintf(stderr, "Erreur : impossible de charger %s\n", config_file);
    exit(EXIT_FAILURE);
  }

  char current_time[64];
  getUTCTimestamp(current_time, sizeof(current_time));
  printf("Heure système UTC : %s\n\n", current_time);

  if (initDatabase(&app_config) != SQLITE_OK)
  {
    exit(EXIT_FAILURE);
  }

  MQTTClient_create(&mqtt_client, app_config.mqtt.broker_address,
                    app_config.mqtt.client_id,
                    MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(mqtt_client, NULL, connectionLost, messageArrived, NULL);

  conn_opts.keepAliveInterval = app_config.mqtt.keepalive_interval;
  conn_opts.cleansession = 1;

  printf("Connexion au broker MQTT (%s)...\n", app_config.mqtt.broker_address);
  if (MQTTClient_connect(mqtt_client, &conn_opts) != MQTTCLIENT_SUCCESS)
  {
    printf("Échec connexion broker\n");
    closeDatabase();
    exit(EXIT_FAILURE);
  }

  printf("  Connecté au broker\n");

  printf("Abonnement à %s\n", app_config.mqtt.topic);
  MQTTClient_subscribe(mqtt_client, app_config.mqtt.topic, app_config.mqtt.qos);
  printf("  Abonné\n\n");

  if (app_config.logging.display_messages)
  {
    printf("En attente des données ESP32...\n");
  }

  while (1)
  {
    sleep(10);
  }

  MQTTClient_disconnect(mqtt_client, 10000);
  MQTTClient_destroy(&mqtt_client);
  closeDatabase();

  return 0;
}
