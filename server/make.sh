rm -f /home/arch/Projets/sensor-monitoring-system/server/mqtt_subscriber
gcc /home/arch/Projets/sensor-monitoring-system/server/mqtt_subscriber.c -o /home/arch/Projets/sensor-monitoring-system/server/mqtt_subscriber -lpaho-mqtt3c -lsqlite3 -ljson-c
