#!/bin/bash

rm -f /home/Arch/Projets/sensor-monitoring-system/server/mqtt_subscriber
gcc /home/Arch/Projets/sensor-monitoring-system/server/mqtt_subscriber.c -o /home/Arch/Projets/sensor-monitoring-system/server/mqtt_subscriber -lpaho-mqtt3c -lsqlite3 -ljson-c
