#!/bin/bash

DB_FILE="/home/Arch/Projets/sensor-monitoring-system/data/donnees_esp32.db"
LOG_FILE="/home/Arch/Projets/sensor-monitoring-system/scripts/cleanbd.log"
RETENTION="-3 hours"
BATCH=2000

log() {
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

log "=== Nettoyage ==="

while true; do
  DELETED=$(sqlite3 "$DB_FILE" "
    DELETE FROM mesures
    WHERE rowid IN (
      SELECT rowid FROM mesures
      WHERE timestamp < datetime('now', '$RETENTION')
      LIMIT $BATCH
    );
    SELECT changes();
  ")

  [ "$DELETED" -eq 0 ] && break

  log "Suppression batch: $DELETED"
done

sqlite3 "$DB_FILE" "PRAGMA incremental_vacuum(200);"

log "=== Terminé ==="