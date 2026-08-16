#include "storage/ConfigurationRevisionStorage.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

constexpr char ConfigurationRevisionStorage::REVISION_FILE[];

bool ConfigurationRevisionStorage::load(uint32_t& revision)
{
    revision = 0;

    if (!LittleFS.exists(REVISION_FILE))
    {
        return true;
    }

    File file = LittleFS.open(REVISION_FILE, "r");

    if (!file)
    {
        Serial.println("[ConfigurationRevisionStorage] Error abriendo revision.");

        return false;
    }

    JsonDocument document;

    const DeserializationError error = deserializeJson(document, file);

    file.close();

    if (error)
    {
        Serial.println("[ConfigurationRevisionStorage] JSON de revision invalido.");

        return false;
    }

    if (!document["revision"].is<uint32_t>())
    {
        Serial.println("[ConfigurationRevisionStorage] Revision invalida.");

        return false;
    }

    revision = document["revision"].as<uint32_t>();

    return true;
}

bool ConfigurationRevisionStorage::save(uint32_t revision)
{
    JsonDocument document;

    document["revision"] = revision;

    File file = LittleFS.open(REVISION_FILE, "w");

    if (!file)
    {
        Serial.println("[ConfigurationRevisionStorage] Error abriendo archivo.");

        return false;
    }

    const size_t written = serializeJson(document, file);

    file.flush();
    file.close();

    if (written == 0)
    {
        Serial.println("[ConfigurationRevisionStorage] Error guardando revision.");

        return false;
    }

    return true;
}

bool ConfigurationRevisionStorage::clear()
{
    if (!LittleFS.exists(REVISION_FILE))
    {
        return true;
    }

    return LittleFS.remove(REVISION_FILE);
}