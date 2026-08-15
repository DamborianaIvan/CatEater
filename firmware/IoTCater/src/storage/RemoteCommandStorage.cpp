#include "storage/RemoteCommandStorage.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

constexpr char RemoteCommandStorage::COMMAND_FILE[];

bool RemoteCommandStorage::begin()
{
    if (!LittleFS.begin())
    {
        Serial.println("[RemoteCommandStorage] Error al iniciar LittleFS.");
        return false;
    }

    return true;
}

bool RemoteCommandStorage::load(RemoteCommand& command)
{
    if (!LittleFS.exists(COMMAND_FILE))
    {
        return false;
    }

    File file = LittleFS.open(COMMAND_FILE, "r");

    if (!file)
    {
        return false;
    }

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, file);

    file.close();

    if (error)
    {
        Serial.println("[RemoteCommandStorage] Error leyendo comando.");
        return false;
    }

    command.commandId = document["commandId"] | "";
    command.portions = document["portions"] | 1;

    const String status = document["status"] | "";

    if (status == "pending")
    {
        command.status = RemoteCommandStatus::Pending;
    }
    else if (status == "executing")
    {
        command.status = RemoteCommandStatus::Executing;
    }
    else if (status == "completed")
    {
        command.status = RemoteCommandStatus::Completed;
    }
    else
    {
        return false;
    }

    return !command.commandId.isEmpty();
}

bool RemoteCommandStorage::save(const RemoteCommand& command)
{
    JsonDocument document;

    document["commandId"] = command.commandId;
    document["portions"] = command.portions;

    switch (command.status)
    {
        case RemoteCommandStatus::Pending:
            document["status"] = "pending";
            break;

        case RemoteCommandStatus::Executing:
            document["status"] = "executing";
            break;

        case RemoteCommandStatus::Completed:
            document["status"] = "completed";
            break;
    }

    File file = LittleFS.open(COMMAND_FILE, "w");

    if (!file)
    {
        Serial.println("[RemoteCommandStorage] Error abriendo archivo.");
        return false;
    }

    const size_t written = serializeJson(document, file);

    file.flush();
    file.close();

    if (written == 0)
    {
        Serial.println("[RemoteCommandStorage] Error guardando comando.");
        return false;
    }

    return true;
}

bool RemoteCommandStorage::clear()
{
    if (!LittleFS.exists(COMMAND_FILE))
    {
        return true;
    }

    if (!LittleFS.remove(COMMAND_FILE))
    {
        Serial.println("[RemoteCommandStorage] Error eliminando comando.");
        return false;
    }

    return true;
}