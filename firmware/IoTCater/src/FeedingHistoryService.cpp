#include "FeedingHistoryService.h"
#include <ArduinoJson.h>

bool FeedingHistoryService::begin()
{
    if (!LittleFS.begin())
    {
        Serial.println("[FeedingHistoryService] Error al iniciar LittleFS.");

        return false;
    }

    Serial.println("[FeedingHistoryService] LittleFS iniciado.");

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

bool FeedingHistoryService::markAsSynced(time_t timestamp)
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

    for (JsonObject entry : history)
    {
        if (entry["timestamp"] == timestamp)
        {
            entry["synced"] = true;
            break;
        }
    }

    file = LittleFS.open(HISTORY_FILE, "w");

    if (!file)
    {
        return false;
    }

    serializeJson(document, file);
    file.close();

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

    return true;
}