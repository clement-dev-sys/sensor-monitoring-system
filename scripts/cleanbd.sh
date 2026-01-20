#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CONFIG_FILE="$PROJECT_ROOT/config.toml"

parse_toml() {
  local key=$1
  grep "^$key" "$CONFIG_FILE" | cut -d'=' -f2 | tr -d ' "' | head -1
}

DB_PATH=$(parse_toml "path")
LOG_PATH=$(parse_toml "cleanup_log")
RETENTION=$(parse_toml "retention_hours")
BATCH=$(parse_toml "cleanup_batch_size")

DB_FILE="$PROJECT_ROOT/$DB_PATH"
LOG_FILE="$PROJECT_ROOT/$LOG_PATH"
RETENTION_STR="-${RETENTION} hours"

if [ ! -f "$CONFIG_FILE" ]; then
  echo "ERREUR : Fichier de configuration introuvable: $CONFIG_FILE"
  exit 1
fi

log() {
  echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

if [ ! -f "$DB_FILE" ]; then
  log "ERREUR : Base de données introuvable: $DB_FILE"
  exit 1
fi

TOTAL_DELETED=0

while true; do
  DELETED=$(sqlite3 "$DB_FILE" "
    DELETE FROM mesures
    WHERE rowid IN (
      SELECT rowid FROM mesures
      WHERE timestamp < datetime('now', '$RETENTION_STR')
      LIMIT $BATCH
    );
    SELECT changes();
  ")

  [ "$DELETED" -eq 0 ] && break

  TOTAL_DELETED=$((TOTAL_DELETED + DELETED))
  log "Batch supprimé : $DELETED lignes"
done

if [ "$TOTAL_DELETED" -gt 0 ]; then
  log "=== Nettoyage ==="
  log "Total supprimé : $TOTAL_DELETED lignes"
  log "=== Terminé ==="
  
  sqlite3 "$DB_FILE" "PRAGMA incremental_vacuum(200);"
fi