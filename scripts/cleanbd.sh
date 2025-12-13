#!/bin/bash
# Supprime les values de plus de 3 heures

DB_FILE="/home/arch/Projets/sensor-monitoring-system/data/donnees_esp32.db"
LOG_FILE="/home/arch/Projets/sensor-monitoring-system/scripts/cleanbd.log"

log() {
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

if [ ! -f "$DB_FILE" ]; then
  echo -e "Erreur: Base de données '$DB_FILE' introuvable"
  exit 1
fi

log "===== Début du nettoyage ====="
BEFORE=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM values;")
log "Nombre de 'values' avant: $BEFORE"

TO_DELETE=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM values WHERE timestamp < datetime('now', '-3 hours');")
log "'values' à supprimer (> 3h): $TO_DELETE"

if [ "$TO_DELETE" -eq 0 ]; then
  echo -e "Aucune donnée à supprimer"
  log "Aucune donnée à supprimer"
else
  sqlite3 "$DB_FILE" "DELETE FROM values WHERE timestamp < datetime('now', '-3 hours');"

  sqlite3 "$DB_FILE" "VACUUM;"

  AFTER=$(sqlite3 "$DB_FILE" "SELECT COUNT(*) FROM values;")
  DELETED=$((BEFORE - AFTER))

  log "Nombre de 'values' après: $AFTER"
  log "$DELETED 'values' supprimées"
fi

log "===== Nettoyage terminé ====="
echo ""
