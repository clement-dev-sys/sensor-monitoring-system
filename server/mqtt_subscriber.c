#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>
#include <sqlite3.h>
#include <json-c/json.h>

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "UnixSubscriber"
#define TOPIC       "esp32/data"
#define QOS         1
#define DB_FILE     "/home/arch/projets/sensor-monitoring-system/data/donnees_esp32.db"

sqlite3 *db;

int initDatabase() {
    int rc = sqlite3_open(DB_FILE, &db);
    
    if (rc) {
        fprintf(stderr, "Erreur ouverture DB : %s\n", sqlite3_errmsg(db));
        return rc;
    }
    
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS mesures ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "device_id TEXT,"
	      "timestamp INTEGER,"
	      "temperature REAL,"
        "pression REAL,"
        "humidite INTEGER,"
        "luminosite REAL"
        ");";
    
    char *errMsg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur création table : %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    
    printf("Base de données prête : %s\n", DB_FILE);
    return SQLITE_OK;
}

int insertData(const char* device_id, long timestamp, double temp, double press, int hum, double lux) {
    char sql[512];
    snprintf(sql, sizeof(sql),
        "INSERT INTO mesures (device_id, timestamp, temperature, pression, humidite, luminosite) "
        "VALUES ('%s', %ld, %.2f, %.2f, %d, %.2f);",
        device_id, timestamp, temp, press, hum, lux);
    
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur insertion : %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    
    return SQLITE_OK;
}

int parseAndStore(const char* jsonString) {
    struct json_object *parsed_json;
    struct json_object *device_id_obj, *timestamp_obj;
    struct json_object *temp_obj, *press_obj, *hum_obj, *lux_obj;

    parsed_json = json_tokener_parse(jsonString);

    if (parsed_json == NULL) {
        printf("Erreur parsing JSON\n");
        return -1;
    }

    json_object_object_get_ex(parsed_json, "device_id", &device_id_obj);
    json_object_object_get_ex(parsed_json, "timestamp", &timestamp_obj);
    json_object_object_get_ex(parsed_json, "temperature", &temp_obj);
    json_object_object_get_ex(parsed_json, "pression", &press_obj);
    json_object_object_get_ex(parsed_json, "humidite", &hum_obj);
    json_object_object_get_ex(parsed_json, "luminosite", &lux_obj);

    if (!device_id_obj || !timestamp_obj || !temp_obj || !press_obj || !hum_obj || !lux_obj) {
        printf("JSON incomplet\n");
        json_object_put(parsed_json);
        return -1;
    }

    const char* device_id = json_object_get_string(device_id_obj);
    long timestamp = json_object_get_int64(timestamp_obj);
    double temperature = json_object_get_double(temp_obj);
    double pression = json_object_get_double(press_obj);
    int humidite = json_object_get_int(hum_obj);
    double luminosite = json_object_get_double(lux_obj);

    printf("Données parsées :\n");
    printf("  Device ID :    %s\n", device_id);
    printf("  Timestamp :    %ld\n", timestamp);
    printf("  Température :  %.1f°C\n", temperature);
    printf("  Pression :     %.1f hPa\n", pression);
    printf("  Humidité :     %d%%\n", humidite);
    printf("  Luminosité :   %.1f lux\n", luminosite);

    int result = insertData(device_id, timestamp, temperature, pression, humidite, luminosite);

    if (result == SQLITE_OK) {
        printf("Enregistré dans la base\n");
    }

    json_object_put(parsed_json);

    return result;
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *payload = (char *)message->payload;
    
    printf("\n=== Message reçu ===\n");
    printf("Topic : %s\n", topicName);
    printf("Données : %.*s\n", message->payloadlen, payload);
    
    parseAndStore(payload);    

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

void connectionLost(void *context, char *cause) {
    printf("\nConnexion MQTT perdue : %s\n", cause);
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    
    printf("=== Subscriber MQTT ===\n");
    
    if (initDatabase() != SQLITE_OK) {
        exit(EXIT_FAILURE);
    }
    
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, connectionLost, messageArrived, NULL);
    
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    
    printf("Connexion au broker MQTT...\n");
    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        printf("Échec connexion broker\n");
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }
    
    printf("Connecté au broker\n");
    
    printf("Abonnement à: %s\n", TOPIC);
    MQTTClient_subscribe(client, TOPIC, QOS);
    printf("Abonné\n\n");
    
    printf("En attente des données ESP32...\n");
    
    while (1) {
        sleep(1);
    }

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    sqlite3_close(db);
    
    return 0;
}
