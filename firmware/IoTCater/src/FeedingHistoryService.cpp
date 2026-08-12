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