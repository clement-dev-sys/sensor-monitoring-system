# Makefile pour Sensor Monitoring System

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I./server
LIBS = -lpaho-mqtt3c -ljson-c -lsqlite3 -ltoml

# Dossiers
SRC_DIR = server
BUILD_DIR = build
DATA_DIR = data

# Fichiers
TARGET = $(BUILD_DIR)/mqtt_subscriber
SOURCES = $(SRC_DIR)/mqtt_subscriber.c $(SRC_DIR)/config.c
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean deps run

all: $(TARGET)

# Compilation
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o $(TARGET)
	@echo "Compilation réussie : $(TARGET)"

# Installation des dépendances (Arch Linux)
deps:
	@echo "Installation des dépendances..."
	sudo pacman -S --needed eclipse-paho-mqtt-c json-c sqlite toml-c
	@echo "Dépendances installées"

# Nettoyage
clean:
	@echo "Nettoyage..."
	@rm -rf $(BUILD_DIR)
	@echo "Nettoyage terminé"

# Nettoyage complet
cleanall: clean
	@echo "Nettoyage complet..."
	@rm -f $(DATA_DIR)/*.db $(DATA_DIR)/*.log
	@echo "✅ Nettoyage complet terminé"

# Lancer le serveur
run: $(TARGET)
	@echo "🚀 Lancement du serveur..."
	@./$(TARGET) config.toml

# Aide
help:
	@echo "Commandes disponibles:"
	@echo "  make deps        - Installer les dépendances"
	@echo "  make             - Compiler le projet"
	@echo "  make run         - Compiler et lancer"
	@echo "  make clean       - Nettoyer les binaires"
	@echo "  make cleanall    - Nettoyer tout (BDD incluse)"
