#include "services/FeedingHistoryService.h"
#include <ArduinoJson.h>
#include <vector>

bool FeedingHistoryService::isValidHistoryFile(const char* path) const
{
    if (!LittleFS.exists(path)) return false;
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.printf("[FeedingHistory] No se pudo abrir %s para lectura.\n", path);
        return false;
    }

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, file);
    file.close();

    if (error)
    {
        Serial.printf("[FeedingHistory] JSON invalido en %s: %s\n", path, error.c_str());
        return false;
    }

    return document.is<JsonArray>();
}

bool FeedingHistoryService::preserveCorruptHistoryFile(const char* path, const char* preservedPath)
{
    if (!LittleFS.exists(path)) return true;
    if (LittleFS.exists(preservedPath))
    {
        Serial.printf("[FeedingHistory] Ya existe %s; no se preserva %s.\n", preservedPath, path);
        return false;
    }

    if (!LittleFS.rename(path, preservedPath))
    {
        Serial.printf("[FeedingHistory] No se pudo preservar %s como %s.\n", path, preservedPath);
        return false;
    }

    return true;
}

bool FeedingHistoryService::replaceHistoryWith(const char* replacementPath)
{
    if (LittleFS.exists(HISTORY_FILE))
    {
        if (LittleFS.exists(HISTORY_BACKUP_FILE) && !LittleFS.remove(HISTORY_BACKUP_FILE))
        {
            Serial.println("[FeedingHistory] No se pudo eliminar el backup anterior.");
            return false;
        }

        if (!LittleFS.rename(HISTORY_FILE, HISTORY_BACKUP_FILE))
        {
            Serial.println("[FeedingHistory] No se pudo mover el historial actual al backup.");
            return false;
        }
    }

    if (LittleFS.rename(replacementPath, HISTORY_FILE)) return true;

    Serial.printf("[FeedingHistory] No se pudo renombrar %s a %s.\n", replacementPath, HISTORY_FILE);

    if (LittleFS.exists(HISTORY_BACKUP_FILE) && !LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE))
        Serial.println("[FeedingHistory] ADVERTENCIA: tampoco se pudo restaurar el backup.");

    return false;
}

bool FeedingHistoryService::recoverHistoryFiles()
{
    const bool historyValid = isValidHistoryFile(HISTORY_FILE);
    const bool temporaryValid = isValidHistoryFile(HISTORY_TEMP_FILE);
    const bool backupValid = isValidHistoryFile(HISTORY_BACKUP_FILE);

    if (temporaryValid)
    {
        if (!historyValid && !preserveCorruptHistoryFile(HISTORY_FILE, HISTORY_CORRUPT_FILE)) return false;
        return replaceHistoryWith(HISTORY_TEMP_FILE);
    }

    if (historyValid) return true;

    if (!LittleFS.exists(HISTORY_FILE))
    {
        if (backupValid && !LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE))
            Serial.println("[FeedingHistory] No se pudo restaurar el backup del historial.");
        return true;
    }

    if (!preserveCorruptHistoryFile(HISTORY_FILE, HISTORY_CORRUPT_FILE)) return false;
    if (backupValid) return LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE);
    return true;
}

bool FeedingHistoryService::writeHistoryTemp(JsonDocument& document)
{
    const size_t jsonSize = measureJson(document);
    FSInfo fsInfo;
    const bool fsInfoAvailable = LittleFS.info(fsInfo);
    const size_t freeBytes = fsInfoAvailable ? fsInfo.totalBytes - fsInfo.usedBytes : 0;

    Serial.printf("[FeedingHistory] Escritura: JSON=%u bytes, FS libre=%u bytes.\n",
                  static_cast<unsigned>(jsonSize),
                  static_cast<unsigned>(freeBytes));

    File file = LittleFS.open(HISTORY_TEMP_FILE, "w");
    if (!file)
    {
        Serial.println("[FeedingHistory] ERROR: no se pudo abrir el archivo temporal para escritura.");
        return false;
    }

    const size_t written = serializeJson(document, file);
    if (written != jsonSize)
        Serial.printf("[FeedingHistory] ERROR: serializeJson escribio %u de %u bytes.\n",
                      static_cast<unsigned>(written),
                      static_cast<unsigned>(jsonSize));

    file.flush();
    const size_t fileSize = file.size();
    file.close();

    if (written != jsonSize || fileSize != written)
    {
        Serial.printf("[FeedingHistory] ERROR: escritura inconsistente. written=%u, fileSize=%u, esperado=%u.\n",
                      static_cast<unsigned>(written),
                      static_cast<unsigned>(fileSize),
                      static_cast<unsigned>(jsonSize));
        return false;
    }

    return true;
}

bool FeedingHistoryService::writeHistoryDocument(JsonDocument& document)
{
    if (!writeHistoryTemp(document)) return false;

    if (!replaceHistoryWith(HISTORY_TEMP_FILE))
    {
        Serial.println("[FeedingHistory] ERROR: no se pudo reemplazar el historial con el temporal.");
        return false;
    }

    return true;
}

bool FeedingHistoryService::begin()
{
    if (!LittleFS.begin())
    {
        Serial.println("[FeedingHistory] ERROR: LittleFS.begin() fallo.");
        return false;
    }

    FSInfo fsInfo;
    if (LittleFS.info(fsInfo))
    {
        Serial.printf("[FeedingHistory] LittleFS OK. Total=%u, usado=%u, libre=%u bytes.\n",
                      static_cast<unsigned>(fsInfo.totalBytes),
                      static_cast<unsigned>(fsInfo.usedBytes),
                      static_cast<unsigned>(fsInfo.totalBytes - fsInfo.usedBytes));
    }
    else
    {
        Serial.println("[FeedingHistory] LittleFS OK. No se pudo obtener informacion de capacidad.");
    }

    if (!loadEventSequence())
    {
        Serial.println("[FeedingHistory] ERROR: no se pudo cargar la secuencia de eventos.");
        return false;
    }

    if (!recoverHistoryFiles())
    {
        Serial.println("[FeedingHistory] ERROR: no se pudo recuperar el historial.");
        return false;
    }

    Serial.println("[FeedingHistory] Historial inicializado correctamente.");
    return true;
}

String FeedingHistoryService::createEventId()
{
    if (!_eventIdGenerationAvailable || _eventSequence == UINT32_MAX) return "";

    const uint32_t nextSequence = _eventSequence + 1;
    if (!saveEventSequence(nextSequence)) return "";

    _eventSequence = nextSequence;
    return String(ESP.getChipId(), HEX) + "-" + String(_eventSequence);
}

bool FeedingHistoryService::loadEventSequence()
{
    if (!LittleFS.exists(EVENT_SEQUENCE_FILE))
    {
        _eventSequence = 0;
        _eventIdGenerationAvailable = true;
        return true;
    }

    File file = LittleFS.open(EVENT_SEQUENCE_FILE, "r");
    if (!file) return false;

    const String value = file.readString();
    file.close();

    char* end = nullptr;
    const unsigned long sequence = strtoul(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0') return false;

    _eventSequence = static_cast<uint32_t>(sequence);
    _eventIdGenerationAvailable = true;
    return true;
}

bool FeedingHistoryService::saveEventSequence(uint32_t sequence)
{
    File file = LittleFS.open(EVENT_SEQUENCE_FILE, "w");
    if (!file)
    {
        Serial.println("[FeedingHistory] ERROR: no se pudo abrir la secuencia para escritura.");
        return false;
    }

    const size_t written = file.print(sequence);
    file.flush();
    file.close();
    return written > 0;
}

bool FeedingHistoryService::trimHistory()
{
    if (!LittleFS.exists(HISTORY_FILE)) return true;

    JsonDocument document;
    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return false;

    const DeserializationError error = deserializeJson(document, file);
    file.close();
    if (error) return false;

    JsonArray history = document.as<JsonArray>();
    if (history.isNull()) return false;

    bool changed = false;
    while (history.size() > MAX_HISTORY_EVENTS)
    {
        int oldestSyncedIndex = -1;
        for (size_t i = 0; i < history.size(); ++i)
        {
            if (history[i]["synced"] | false)
            {
                oldestSyncedIndex = static_cast<int>(i);
                break;
            }
        }

        if (oldestSyncedIndex < 0) break;
        history.remove(oldestSyncedIndex);
        changed = true;
    }

    if (!changed) return true;
    return writeHistoryDocument(document);
}

std::vector<FeedingEvent> FeedingHistoryService::getHistory()
{
    std::vector<FeedingEvent> history;
    if (!LittleFS.exists(HISTORY_FILE)) return history;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return history;

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, file);
    file.close();
    if (error) return history;

    JsonArray array = document.as<JsonArray>();
    for (JsonObject entry : array)
    {
        FeedingEvent event;
        event.eventId = entry["eventId"] | "";
        event.timestamp = entry["timestamp"] | 0;
        event.portions = entry["portions"] | 0;
        event.synced = entry["synced"] | false;

        const char* source = entry["source"] | "";
        if (strcmp(source, "physical") == 0) event.source = FeedingSource::Physical;
        else if (strcmp(source, "scheduled") == 0) event.source = FeedingSource::Scheduled;
        else if (strcmp(source, "remote") == 0) event.source = FeedingSource::Remote;
        else continue;

        history.push_back(event);
    }
    return history;
}

bool FeedingHistoryService::getNextPendingEvent(FeedingEvent& event)
{
    if (!LittleFS.exists(HISTORY_FILE)) return false;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return false;

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, file);
    file.close();
    if (error) return false;

    JsonArray array = document.as<JsonArray>();
    for (JsonObject entry : array)
    {
        if (entry["synced"] | false) continue;

        const char* source = entry["source"] | "";
        FeedingSource parsedSource;
        if (strcmp(source, "physical") == 0) parsedSource = FeedingSource::Physical;
        else if (strcmp(source, "scheduled") == 0) parsedSource = FeedingSource::Scheduled;
        else if (strcmp(source, "remote") == 0) parsedSource = FeedingSource::Remote;
        else continue;

        event.eventId = entry["eventId"] | "";
        event.timestamp = entry["timestamp"] | 0;
        event.portions = entry["portions"] | 0;
        event.source = parsedSource;
        event.synced = false;
        return true;
    }
    return false;
}

bool FeedingHistoryService::markAsSynced(const String& eventId)
{
    if (!LittleFS.exists(HISTORY_FILE)) return false;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return false;

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, file);
    file.close();
    if (error) return false;

    JsonArray history = document.as<JsonArray>();
    bool found = false;
    for (JsonObject entry : history)
    {
        if (entry["eventId"] == eventId)
        {
            entry["synced"] = true;
            found = true;
            break;
        }
    }

    if (!found) return false;
    return writeHistoryDocument(document);
}

bool FeedingHistoryService::save(const FeedingEvent& event)
{
    JsonDocument document;

    if (LittleFS.exists(HISTORY_FILE))
    {
        File file = LittleFS.open(HISTORY_FILE, "r");
        if (!file)
        {
            Serial.println("[FeedingHistory] ERROR: no se pudo abrir el historial existente.");
            return false;
        }

        const DeserializationError error = deserializeJson(document, file);
        file.close();
        if (error)
        {
            Serial.printf("[FeedingHistory] ERROR: historial existente invalido: %s\n", error.c_str());
            return false;
        }
    }

    JsonArray history = document.is<JsonArray>() ? document.as<JsonArray>() : document.to<JsonArray>();
    JsonObject entry = history.add<JsonObject>();

    entry["eventId"] = event.eventId;
    entry["timestamp"] = event.timestamp;
    entry["portions"] = event.portions;
    entry["synced"] = event.synced;

    switch (event.source)
    {
        case FeedingSource::Physical: entry["source"] = "physical"; break;
        case FeedingSource::Scheduled: entry["source"] = "scheduled"; break;
        case FeedingSource::Remote: entry["source"] = "remote"; break;
    }

    while (history.size() > MAX_HISTORY_EVENTS)
    {
        int oldestSyncedIndex = -1;
        for (size_t i = 0; i < history.size(); ++i)
        {
            if (history[i]["synced"] | false)
            {
                oldestSyncedIndex = static_cast<int>(i);
                break;
            }
        }

        if (oldestSyncedIndex < 0) break;
        history.remove(oldestSyncedIndex);
    }

    return writeHistoryDocument(document);
}
