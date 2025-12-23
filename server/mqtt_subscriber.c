#include "mqtt_subscriber.h"

// ===== VARIABLES GLOBALES =====
sqlite3 *db = NULL;
Seuils seuils = {0};

// ===== DATE UTC =====

void getUTCTimestamp(char *buffer, size_t size)
{
  time_t now = time(NULL);
  struct tm *utc_time = gmtime(&now);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", utc_time);
}

// ===== SEUILS =====

int loadSeuils(void)
{
  FILE *f = fopen(CONFIG_FILE, "r");
  if (!f)
  {
    fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", CONFIG_FILE);
    return 0;
  }

  char line[256];
  char section[64] = "";

  while (fgets(line, sizeof(line), f))
  {
    if (line[0] == '#' || line[0] == '\n')
      continue;

    if (line[0] == '[')
    {
      sscanf(line, "[%63[^]]]", section);
      continue;
    }

    char key[64];
    double value;
    if (sscanf(line, "%63s = %lf", key, &value) == 2)
    {
      if (strcmp(section, "temperature") == 0)
      {
        if (strcmp(key, "min") == 0)
          seuils.temp_min = value;
        if (strcmp(key, "max") == 0)
          seuils.temp_max = value;
      }
      else if (strcmp(section, "pression") == 0)
      {
        if (strcmp(key, "min") == 0)
          seuils.press_min = value;
        if (strcmp(key, "max") == 0)
          seuils.press_max = value;
      }
      else if (strcmp(section, "humidite") == 0)
      {
        if (strcmp(key, "min") == 0)
          seuils.hum_min = (int)value;
        if (strcmp(key, "max") == 0)
          seuils.hum_max = (int)value;
      }
    }
  }

  fclose(f);
  printf("Seuils chargés depuis %s\n", CONFIG_FILE);
  return 1;
}

void displaySeuils(void)
{
  printf("\nSeuils d'alerte configurés:\n");
  printf("Température : %.1f°C à %.1f°C\n", seuils.temp_min, seuils.temp_max);
  printf("Pression : %.1f à %.1f hPa\n", seuils.press_min, seuils.press_max);
  printf("Humidité : %d%% à %d%%\n", seuils.hum_min, seuils.hum_max);
}

void logAlert(const char *capteur, double valeur, const char *type, double seuil)
{
  FILE *f = fopen(ALERT_FILE, "a");
  if (!f)
  {
    fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", ALERT_FILE);
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

void checkSeuils(double temp, double press, int hum)
{
  if (temp < seuils.temp_min)
  {
    logAlert("Température", temp, "MIN", seuils.temp_min);
  }
  else if (temp > seuils.temp_max)
  {
    logAlert("Température", temp, "MAX", seuils.temp_max);
  }

  if (press < seuils.press_min)
  {
    logAlert("Pression", press, "MIN", seuils.press_min);
  }
  else if (press > seuils.press_max)
  {
    logAlert("Pression", press, "MAX", seuils.press_max);
  }

  if (hum < seuils.hum_min)
  {
    logAlert("Humidité", (double)hum, "MIN", (double)seuils.hum_min);
  }
  else if (hum > seuils.hum_max)
  {
    logAlert("Humidité", (double)hum, "MAX", (double)seuils.hum_max);
  }
}

// ===== BASE DE DONNÉES =====

int initDatabase(void)
{
  int rc;
  struct stat buffer;
  int new_db = (stat(DB_FILE, &buffer) != 0);

  rc = sqlite3_open(DB_FILE, &db);
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

  printf("Base de données prête (WAL + index + auto_vacuum)\n");
  return SQLITE_OK;
}

int insertData(double temp, double press, int hum)
{
  char sql[512];

  char timestamp[64];
  getUTCTimestamp(timestamp, sizeof(timestamp));

  snprintf(sql, sizeof(sql),
           "INSERT INTO mesures (timestamp, temperature, pression, humidite) "
           "VALUES ('%s', %.2f, %.2f, %d);",
           timestamp, temp, press, hum);

  char *errMsg = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "Erreur insertion : %s\n", errMsg);
    sqlite3_free(errMsg);
    return rc;
  }

  return SQLITE_OK;
}

// ===== JSON =====

int parseAndStore(const char *jsonString)
{
  struct json_object *parsed_json;
  struct json_object *device_id_obj, *temp_obj, *press_obj, *hum_obj;

  parsed_json = json_tokener_parse(jsonString);

  if (parsed_json == NULL)
  {
    printf("Erreur parsing JSON\n");
    return -1;
  }

  json_object_object_get_ex(parsed_json, "temperature", &temp_obj);
  json_object_object_get_ex(parsed_json, "pression", &press_obj);
  json_object_object_get_ex(parsed_json, "humidite", &hum_obj);

  if (!temp_obj || !press_obj || !hum_obj)
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

  checkSeuils(temperature, pression, humidite);

  int result = insertData(temperature, pression, humidite);

  if (result == SQLITE_OK)
  {
    printf("=== Message enregistré ===\n");
  }

  json_object_put(parsed_json);
  return result;
}

// ===== MQTT =====

int messageArrived(void *context, char *topicName, int topicLen,
                   MQTTClient_message *message)
{
  char *payload = (char *)message->payload;

  printf("\n=== Message reçu ===\n");

  char readable_ts[64];
  getUTCTimestamp(readable_ts, sizeof(readable_ts));

  printf("Date, Heure : %s\n", readable_ts);
  printf("Topic : %s\n", topicName);
  // printf("Données : %.*s\n", message->payloadlen, payload);

  parseAndStore(payload);

  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);

  return 1;
}

void connectionLost(void *context, char *cause)
{
  printf("\nConnexion MQTT perdue : %s\n", cause);
}

// ===== MAIN =====

int main(void)
{
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

  printf("=== Subscriber MQTT ===\n");

  char current_time[64];
  getUTCTimestamp(current_time, sizeof(current_time));
  printf("Heure système UTC : %s\n\n", current_time);

  loadSeuils();
  displaySeuils();

  if (initDatabase() != SQLITE_OK)
  {
    exit(EXIT_FAILURE);
  }

  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);

  conn_opts.keepAliveInterval = 6;
  conn_opts.cleansession = 1;

  printf("Connexion au broker MQTT...\n");
  if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS)
  {
    printf("Échec connexion broker\n");
    sqlite3_close(db);
    exit(EXIT_FAILURE);
  }

  printf("Connecté au broker\n");

  printf("Abonnement à : %s\n", TOPIC);
  MQTTClient_subscribe(client, TOPIC, QOS);
  printf("Abonné\n\n");

  printf("En attente des données ESP32...\n");

  while (1)
  {
    sleep(1);
  }

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);
  sqlite3_close(db);

  return 0;
}