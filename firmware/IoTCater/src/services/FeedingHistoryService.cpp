#include "services/FeedingHistoryService.h"
#include <ArduinoJson.h>

bool FeedingHistoryService::isValidHistoryFile(const char* path) const
{
    if (!LittleFS.exists(path))
    {
        return false;
    }

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        return false;
    }

    JsonDocument document;

    Serial.printf("[FeedingHistoryService] Heap antes de deserialize validacion: %u\n",
                  ESP.getFreeHeap());

    const DeserializationError error = deserializeJson(document, file);

    Serial.printf("[FeedingHistoryService] Heap despues de deserialize validacion: %u\n",
                  ESP.getFreeHeap());
    if (error)
    {
        Serial.print("[FeedingHistoryService] Error de validacion JSON: ");

        Serial.println(error.c_str());
    }
    file.close();
    return !error && document.is<JsonArray>();
}

bool FeedingHistoryService::preserveCorruptHistoryFile(const char* path, const char* preservedPath)
{
    if (!LittleFS.exists(path))
    {
        return true;
    }

    if (LittleFS.exists(preservedPath))
    {
        Serial.println(
            "[FeedingHistoryService] No se reemplaza un historial corrupto ya preservado.");
        return false;
    }

    return LittleFS.rename(path, preservedPath);
}

bool FeedingHistoryService::replaceHistoryWith(const char* replacementPath)
{
    if (LittleFS.exists(HISTORY_FILE))
    {
        if (LittleFS.exists(HISTORY_BACKUP_FILE) && !LittleFS.remove(HISTORY_BACKUP_FILE))
        {
            return false;
        }

        if (!LittleFS.rename(HISTORY_FILE, HISTORY_BACKUP_FILE))
        {
            return false;
        }
    }

    if (LittleFS.rename(replacementPath, HISTORY_FILE))
    {
        return true;
    }

    if (LittleFS.exists(HISTORY_BACKUP_FILE))
    {
        LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE);
    }
    return false;
}

bool FeedingHistoryService::recoverHistoryFiles()
{
    const bool historyValid = isValidHistoryFile(HISTORY_FILE);
    const bool temporaryValid = isValidHistoryFile(HISTORY_TEMP_FILE);
    const bool backupValid = isValidHistoryFile(HISTORY_BACKUP_FILE);

    if (temporaryValid)
    {
        if (!historyValid && !preserveCorruptHistoryFile(HISTORY_FILE, HISTORY_CORRUPT_FILE))
        {
            return false;
        }

        if (replaceHistoryWith(HISTORY_TEMP_FILE))
        {
            Serial.println("[FeedingHistoryService] Historial recuperado desde temporal.");
            return true;
        }

        Serial.println("[FeedingHistoryService] Error recuperando historial temporal.");
        return false;
    }

    if (historyValid)
    {
        return true;
    }

    if (!LittleFS.exists(HISTORY_FILE))
    {
        if (backupValid && LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE))
        {
            Serial.println("[FeedingHistoryService] Historial recuperado desde backup.");
        }
        return true;
    }

    if (!preserveCorruptHistoryFile(HISTORY_FILE, HISTORY_CORRUPT_FILE))
    {
        return false;
    }

    if (backupValid && LittleFS.rename(HISTORY_BACKUP_FILE, HISTORY_FILE))
    {
        Serial.println("[FeedingHistoryService] Historial corrupto recuperado desde backup.");
    }
    else
    {
        Serial.println(
            "[FeedingHistoryService] Historial corrupto preservado; se inicia cola nueva.");
    }
    return true;
}

bool FeedingHistoryService::writeHistoryTemp(JsonDocument& document)
{
    File file = LittleFS.open(HISTORY_TEMP_FILE, "w");

    if (!file)
    {
        Serial.println("[FeedingHistoryService] Error abriendo archivo temporal.");

        return false;
    }

    const size_t written = serializeJson(document, file);

    const size_t fileSize = file.size();

    file.flush();
    file.close();

    if (written == 0)
    {
        Serial.println("[FeedingHistoryService] Error serializando historial.");

        return false;
    }

    if (fileSize != written)
    {
        Serial.printf("[FeedingHistoryService] Escritura incompleta. Esperados: %u, escritos: %u\n",
                      written, fileSize);

        return false;
    }

    return true;
}

bool FeedingHistoryService::writeHistoryDocument(JsonDocument& document)
{
    if (!writeHistoryTemp(document))
    {
        return false;
    }

    return replaceHistoryWith(HISTORY_TEMP_FILE);
}
bool FeedingHistoryService::begin()
{
    if (!LittleFS.begin())
    {
        Serial.println("[FeedingHistoryService] Error al iniciar LittleFS.");

        return false;
    }

    if (!loadEventSequence())
    {
        Serial.println("[FeedingHistoryService] Error cargando secuencia de eventos.");
        return false;
    }

    if (!recoverHistoryFiles())
    {
        Serial.println("[FeedingHistoryService] Error recuperando historial.");
        return false;
    }

    Serial.println("[FeedingHistoryService] LittleFS iniciado.");

    return true;
}

String FeedingHistoryService::createEventId()
{
    if (!_eventIdGenerationAvailable || _eventSequence == UINT32_MAX)
    {
        return "";
    }

    const uint32_t nextSequence = _eventSequence + 1;

    if (!saveEventSequence(nextSequence))
    {
        Serial.println("[FeedingHistoryService] Error guardando secuencia de eventos.");
        return "";
    }

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
    if (!file)
    {
        return false;
    }

    const String value = file.readString();
    file.close();

    char* end = nullptr;
    const unsigned long sequence = strtoul(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0')
    {
        return false;
    }

    _eventSequence = static_cast<uint32_t>(sequence);
    _eventIdGenerationAvailable = true;
    return true;
}

bool FeedingHistoryService::saveEventSequence(uint32_t sequence)
{
    File file = LittleFS.open(EVENT_SEQUENCE_FILE, "w");
    if (!file)
    {
        return false;
    }

    const size_t written = file.print(sequence);
    file.close();
    return written > 0;
}

bool FeedingHistoryService::trimHistory()
{
    if (!LittleFS.exists(HISTORY_FILE))
    {
        return true;
    }

    {
        File file = LittleFS.open(HISTORY_FILE, "r");

        if (!file)
        {
            Serial.println("[FeedingHistoryService] Error abriendo historial para limpieza.");

            return false;
        }

        JsonDocument document;

        DeserializationError error = deserializeJson(document, file);

        file.close();

        if (error)
        {
            Serial.println("[FeedingHistoryService] Error leyendo historial para limpieza.");

            return false;
        }

        JsonArray history = document.as<JsonArray>();

        if (history.isNull())
        {
            Serial.println("[FeedingHistoryService] Historial invalido para limpieza.");

            return false;
        }

        if (history.size() <= MAX_HISTORY_EVENTS)
        {
            return true;
        }

        Serial.printf("[FeedingHistoryService] Eventos antes de limpieza: %u\n", history.size());

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

            if (oldestSyncedIndex < 0)
            {
                Serial.println(
                    "[FeedingHistoryService] No hay eventos sincronizados para eliminar.");

                return true;
            }

            history.remove(oldestSyncedIndex);
        }

        if (!writeHistoryTemp(document))
        {
            Serial.println("[FeedingHistoryService] Error escribiendo historial limpio.");

            return false;
        }
    }

    /*
     * El JsonDocument ya fue destruido.
     *
     * El archivo temporal fue generado directamente
     * desde un JsonDocument válido y se verificó que
     * todos los bytes fueron escritos.
     *
     * La validación estructural queda para la
     * recuperación de LittleFS durante el boot.
     */
    if (!replaceHistoryWith(HISTORY_TEMP_FILE))
    {
        Serial.println("[FeedingHistoryService] Error reemplazando historial limpio.");

        return false;
    }

    return true;
}
std::vector<FeedingEvent> FeedingHistoryService::getHistory()
{
    std::vector<FeedingEvent> history;

    if (!LittleFS.exists(HISTORY_FILE))
    {
        return history;
    }

    File file = LittleFS.open(HISTORY_FILE, "r");

    if (!file)
    {
        Serial.println("[FeedingHistoryService] Error abriendo historial.");

        return history;
    }

    JsonDocument document;

    DeserializationError error = deserializeJson(document, file);

    file.close();

    if (error)
    {
        Serial.println("[FeedingHistoryService] Error leyendo historial.");

        return history;
    }

    JsonArray array = document.as<JsonArray>();

    for (JsonObject entry : array)
    {
        FeedingEvent event;

        event.eventId = entry["eventId"] | "";
        event.timestamp = entry["timestamp"] | 0;
        event.portions = entry["portions"] | 0;
        event.synced = entry["synced"] | false;
        const char* source = entry["source"] | "";

        if (strcmp(source, "physical") == 0)
        {
            event.source = FeedingSource::Physical;
        }
        else if (strcmp(source, "scheduled") == 0)
        {
            event.source = FeedingSource::Scheduled;
        }
        else if (strcmp(source, "remote") == 0)
        {
            event.source = FeedingSource::Remote;
        }
        else
        {
            continue;
        }

        history.push_back(event);
    }
    Serial.printf("[FeedingHistoryService] Eventos en historial: %u\n", history.size());
    return history;
}

std::vector<FeedingEvent> FeedingHistoryService::getPendingEvents()
{
    std::vector<FeedingEvent> pending;

    const auto history = getHistory();

    for (const auto& event : history)
    {
        if (!event.synced)
        {
            pending.push_back(event);
        }
    }

    return pending;
}

bool FeedingHistoryService::markAsSynced(const String& eventId)
{
    if (!LittleFS.exists(HISTORY_FILE))
    {
        return false;
    }

    File file = LittleFS.open(HISTORY_FILE, "r");

    if (!file)
    {
        return false;
    }

    JsonDocument document;

    if (deserializeJson(document, file))
    {
        file.close();
        return false;
    }

    file.close();

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

    if (!found)
    {
        return false;
    }

    if (!writeHistoryDocument(document))
    {
        return false;
    }
    Serial.println("[FeedingHistoryService] Evento marcado como sincronizado.");
    return true;
}

bool FeedingHistoryService::save(const FeedingEvent& event)
{
    {
        JsonDocument document;

        if (LittleFS.exists(HISTORY_FILE))
        {
            File file = LittleFS.open(HISTORY_FILE, "r");

            if (file)
            {
                DeserializationError error = deserializeJson(document, file);

                file.close();

                if (error)
                {
                    Serial.println("[FeedingHistoryService] Error leyendo historial.");

                    return false;
                }
            }
        }

        JsonArray history;

        if (document.is<JsonArray>())
        {
            history = document.as<JsonArray>();
        }
        else
        {
            history = document.to<JsonArray>();
        }

        JsonObject entry = history.add<JsonObject>();

        entry["eventId"] = event.eventId;
        entry["timestamp"] = event.timestamp;
        entry["portions"] = event.portions;
        entry["synced"] = event.synced;

        switch (event.source)
        {
            case FeedingSource::Physical:
                entry["source"] = "physical";
                break;

            case FeedingSource::Scheduled:
                entry["source"] = "scheduled";
                break;

            case FeedingSource::Remote:
                entry["source"] = "remote";
                break;
        }

        if (!writeHistoryDocument(document))
        {
            Serial.println("[FeedingHistoryService] Error guardando historial.");

            return false;
        }
    }

    // El JsonDocument ya fue destruido y su memoria liberada.
    trimHistory();

    return true;
}