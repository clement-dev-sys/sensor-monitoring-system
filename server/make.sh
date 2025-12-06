rm -f mqtt_subscriber
gcc mqtt_subscriber.c -o mqtt_subscriber -lpaho-mqtt3c -lsqlite3
