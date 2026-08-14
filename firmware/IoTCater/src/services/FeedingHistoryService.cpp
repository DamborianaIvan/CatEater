#include "services/FeedingHistoryService.h"
#include <ArduinoJson.h>

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

    File file = LittleFS.open(HISTORY_FILE, "r");

    if (!file)
    {
        Serial.println("[FeedingHistoryService] Error abriendo historial para limpieza.");
        return false;
    }

    JsonDocument document;

    if (deserializeJson(document, file))
    {
        file.close();

        Serial.println("[FeedingHistoryService] Error leyendo historial para limpieza.");

        return false;
    }

    file.close();

    JsonArray history = document.as<JsonArray>();

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
            return false;
        }

        history.remove(oldestSyncedIndex);
    }

    file = LittleFS.open(HISTORY_FILE, "w");

    if (!file)
    {
        Serial.println("[FeedingHistoryService] Error guardando historial limpio.");
        return false;
    }

    serializeJson(document, file);
    file.close();

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

    file = LittleFS.open(HISTORY_FILE, "w");

    if (!file)
    {
        return false;
    }

    serializeJson(document, file);
    file.close();
    Serial.println("[FeedingHistoryService] Evento marcado como sincronizado.");
    return true;
}

bool FeedingHistoryService::save(const FeedingEvent& event)
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

    File file = LittleFS.open(HISTORY_FILE, "w");

    if (!file)
    {
        Serial.println("[FeedingHistoryService] Error abriendo historial.");

        return false;
    }

    serializeJson(document, file);
    file.close();

    trimHistory();

    return true;
}
