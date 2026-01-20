#include "config.h"

void config_init_defaults(Config *cfg)
{
  memset(cfg, 0, sizeof(Config));

  // MQTT
  strcpy(cfg->mqtt.broker_address, "tcp://localhost:1883");
  strcpy(cfg->mqtt.topic, "esp32/data");
  strcpy(cfg->mqtt.topic, "server/data");
  strcpy(cfg->mqtt.client_id, "UnixSubscriber");
  cfg->mqtt.qos = 1;
  cfg->mqtt.keepalive_interval = 60;

  // Database
  strcpy(cfg->database.path, "data/donnees_esp32.db");
  cfg->database.retention_hours = 3;
  cfg->database.cleanup_batch_size = 2000;

  // Logging
  strcpy(cfg->logging.cleanup_log, "scripts/cleanbd.log");
  cfg->logging.display_messages = 1;

  // Paths
  strcpy(cfg->paths.data_dir, "data");
  strcpy(cfg->paths.scripts_dir, "scripts");
  strcpy(cfg->paths.server_dir, "server");
}

static void get_project_root(const char *config_file, char *root, size_t root_size)
{
  char abs_path[PATH_SIZE];
  if (realpath(config_file, abs_path) == NULL)
  {
    getcwd(root, root_size);
    return;
  }

  char *dir = dirname(abs_path);
  snprintf(root, root_size, "%s", dir);
}

int config_load(Config *cfg, const char *config_file)
{
  FILE *fp;
  char errbuf[200];

  config_init_defaults(cfg);

  fp = fopen(config_file, "r");
  if (!fp)
  {
    fprintf(stderr, "Erreur: impossible d'ouvrir %s\n", config_file);
    return -1;
  }

  toml_table_t *conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
  fclose(fp);

  if (!conf)
  {
    fprintf(stderr, "Erreur parsing TOML: %s\n", errbuf);
    return -1;
  }

  get_project_root(config_file, cfg->project_root, sizeof(cfg->project_root));

  // ===== SECTION [mqtt] =====
  toml_table_t *mqtt = toml_table_in(conf, "mqtt");
  if (mqtt)
  {
    toml_datum_t broker = toml_string_in(mqtt, "broker_address");
    if (broker.ok)
    {
      strncpy(cfg->mqtt.broker_address, broker.u.s, sizeof(cfg->mqtt.broker_address) - 1);
      free(broker.u.s);
    }

    toml_datum_t topic = toml_string_in(mqtt, "topic");
    if (topic.ok)
    {
      strncpy(cfg->mqtt.topic, topic.u.s, sizeof(cfg->mqtt.topic) - 1);
      free(topic.u.s);
    }

    toml_datum_t topic_republish = toml_string_in(mqtt, "topic_republish");
    if (topic_republish.ok)
    {
      strncpy(cfg->mqtt.topic_republish, topic.u.s, sizeof(cfg->mqtt.topic_republish) - 1);
      free(topic_republish.u.s);
    }

    toml_datum_t client_id = toml_string_in(mqtt, "client_id");
    if (client_id.ok)
    {
      strncpy(cfg->mqtt.client_id, client_id.u.s, sizeof(cfg->mqtt.client_id) - 1);
      free(client_id.u.s);
    }

    toml_datum_t qos = toml_int_in(mqtt, "qos");
    if (qos.ok)
      cfg->mqtt.qos = (int)qos.u.i;

    toml_datum_t keepalive = toml_int_in(mqtt, "keepalive_interval");
    if (keepalive.ok)
      cfg->mqtt.keepalive_interval = (int)keepalive.u.i;
  }

  // ===== SECTION [database] =====
  toml_table_t *database = toml_table_in(conf, "database");
  if (database)
  {
    toml_datum_t path = toml_string_in(database, "path");
    if (path.ok)
    {
      strncpy(cfg->database.path, path.u.s, sizeof(cfg->database.path) - 1);
      free(path.u.s);
    }

    toml_datum_t retention = toml_int_in(database, "retention_hours");
    if (retention.ok)
      cfg->database.retention_hours = (int)retention.u.i;

    toml_datum_t batch = toml_int_in(database, "cleanup_batch_size");
    if (batch.ok)
      cfg->database.cleanup_batch_size = (int)batch.u.i;
  }

  // ===== SECTION [logging] =====
  toml_table_t *logging = toml_table_in(conf, "logging");
  if (logging)
  {
    toml_datum_t cleanup = toml_string_in(logging, "cleanup_log");
    if (cleanup.ok)
    {
      strncpy(cfg->logging.cleanup_log, cleanup.u.s, sizeof(cfg->logging.cleanup_log) - 1);
      free(cleanup.u.s);
    }

    toml_datum_t display = toml_bool_in(logging, "display");
    if (display.ok)
    {
      cfg->logging.display_messages = display.u.b;
    }
  }

  // ===== SECTION [paths] =====
  toml_table_t *paths = toml_table_in(conf, "paths");
  if (paths)
  {
    toml_datum_t data_dir = toml_string_in(paths, "data_dir");
    if (data_dir.ok)
    {
      strncpy(cfg->paths.data_dir, data_dir.u.s, sizeof(cfg->paths.data_dir) - 1);
      free(data_dir.u.s);
    }

    toml_datum_t scripts_dir = toml_string_in(paths, "scripts_dir");
    if (scripts_dir.ok)
    {
      strncpy(cfg->paths.scripts_dir, scripts_dir.u.s, sizeof(cfg->paths.scripts_dir) - 1);
      free(scripts_dir.u.s);
    }

    toml_datum_t server_dir = toml_string_in(paths, "server_dir");
    if (server_dir.ok)
    {
      strncpy(cfg->paths.server_dir, server_dir.u.s, sizeof(cfg->paths.server_dir) - 1);
      free(server_dir.u.s);
    }
  }

  toml_free(conf);
  return 0;
}

void config_resolve_path(const Config *cfg, const char *relative_path, char *output, size_t output_size)
{
  if (relative_path[0] == '/')
  {
    snprintf(output, output_size, "%s", relative_path);
  }
  else
  {
    snprintf(output, output_size, "%s/%s", cfg->project_root, relative_path);
  }
}

void config_display(const Config *cfg)
{
  printf("\n=== Configuration chargée ===\n");
  printf("Racine projet : %s\n\n", cfg->project_root);

  printf("[MQTT]\n");
  printf("  Broker : %s\n", cfg->mqtt.broker_address);
  printf("  Topic : %s\n", cfg->mqtt.topic);
  printf("  Topic republish : %s\n", cfg->mqtt.topic_republish);
  printf("  Client ID : %s\n", cfg->mqtt.client_id);
  printf("  QoS : %d\n", cfg->mqtt.qos);
  printf("  Keepalive : %d s\n", cfg->mqtt.keepalive_interval);

  printf("\n[Database]\n");
  printf("  Path : %s\n", cfg->database.path);
  printf("  Rétention : %d heures\n", cfg->database.retention_hours);
  printf("  Batch cleanup : %d\n", cfg->database.cleanup_batch_size);

  printf("\n[Logging]\n");
  printf("  Cleanup : %s\n", cfg->logging.cleanup_log);
  printf("  Messages : %s\n", cfg->logging.display_messages ? "activé" : "désactivé");

  printf("\n=============================\n\n");
}
