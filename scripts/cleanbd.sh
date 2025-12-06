#!/bin/bash
# Supprime les mesures de plus de 3 heures

DB_FILE="/home/arch/projets/sensor-monitoring-system/data/donnees_esp32.db"
LOG_FILE="cleanup.log"

log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

if [ ! -f "$DB_FILE" ]; then
    echo -e "Erreur: Base de données '$DB_FILE' introuvable"
    exit 1
fi

log "===== Début du nettoyage ====="
BEFORE=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM mesures;")
log "Nombre de mesures avant: $BEFORE"

TO_DELETE=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM mesures WHERE timestamp < datetime('now', '-3 hours');")
log "Mesures à supprimer (> 3h): $TO_DELETE"

if [ "$TO_DELETE" -eq 0 ]; then
    echo -e "Aucune donnée à supprimer"
    log "Aucune donnée à supprimer"
else
    sqlite3 "$DB_FILE" "DELETE FROM mesures WHERE timestamp < datetime('now', '-3 hours');"
    
    sqlite3 "$DB_FILE" "VACUUM;"

    AFTER=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM mesures;")
    DELETED=$((BEFORE - AFTER))
    
    log "Nombre de mesures après: $AFTER"
    log "$DELETED mesures supprimées"
fi

log "===== Nettoyage terminé ====="
echo ""
