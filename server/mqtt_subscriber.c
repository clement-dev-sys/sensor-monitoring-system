#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <MQTTClient.h>
#include <sqlite3.h>

#define ADDRESS     "tcp://localhost:1883"
#define CLIENTID    "UnixSubscriber"
#define TOPIC       "esp32/data"
#define QOS         1
#define DB_FILE     "donnees_esp32.db"

sqlite3 *db;

int initDatabase() {
    int rc = sqlite3_open(DB_FILE, &db);
    
    if (rc) {
        fprintf(stderr, "Erreur ouverture DB: %s\n", sqlite3_errmsg(db));
        return rc;
    }
    
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS mesures ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "nombre1 INTEGER,"
        "nombre2 INTEGER,"
        "nombre3 INTEGER,"
        "nombre4 INTEGER"
        ");";
    
    char *errMsg = 0;
    rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur création table: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    
    printf("Base de données prête: %s\n", DB_FILE);
    return SQLITE_OK;
}

int insertData(int n1, int n2, int n3, int n4) {
    char sql[256];
    snprintf(sql, sizeof(sql),
        "INSERT INTO mesures (nombre1, nombre2, nombre3, nombre4) "
        "VALUES (%d, %d, %d, %d);",
        n1, n2, n3, n4);
    
    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Erreur insertion: %s\n", errMsg);
        sqlite3_free(errMsg);
        return rc;
    }
    
    return SQLITE_OK;
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char *payload = (char *)message->payload;
    
    printf("\n=== Message reçu ===\n");
    printf("Topic: %s\n", topicName);
    printf("Données: %.*s\n", message->payloadlen, payload);
    
    int nombre1, nombre2, nombre3, nombre4;
    
    if (sscanf(payload, "%d,%d,%d,%d", &nombre1, &nombre2, &nombre3, &nombre4) == 4) {
        
        if (insertData(nombre1, nombre2, nombre3, nombre4) == SQLITE_OK) {
            printf("Enregistré dans la base:\n");
            printf("  Nombre 1: %d\n", nombre1);
            printf("  Nombre 2: %d\n", nombre2);
            printf("  Nombre 3: %d\n", nombre3);
            printf("  Nombre 4: %d\n", nombre4);
        } else {
            printf("Erreur enregistrement\n");
        }
        
    } else {
        printf("Format invalide (attendu: n1,n2,n3,n4)\n");
    }
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    
    return 1;
}

void connectionLost(void *context, char *cause) {
    printf("\nConnexion MQTT perdue: %s\n", cause);
}

int main() {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    
    printf("=== Subscriber MQTT + SQLite ===\n");
    
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