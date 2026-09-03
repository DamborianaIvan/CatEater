#include "services/FeedingHistoryService.h"
#include <ArduinoJson.h>
#include <vector>

bool FeedingHistoryService::isValidHistoryFile(const char* path) const
{
    if (!LittleFS.exists(path)) return false;

    File file = LittleFS.open(path, "r");
    if (!file) return false;

    String object;
    bool endOfArray = false;
    size_t count = 0;

    while (readNextHistoryObject(file, object, endOfArray))
    {
        JsonDocument document;
        const DeserializationError error = deserializeJson(document, object);
        if (error || !document.is<JsonObject>())
        {
            Serial.printf("[FeedingHistory] JSON invalido en %s: %s\n", path, error ? error.c_str() : "objeto invalido");
            file.close();
            return false;
        }
        ++count;
    }

    file.close();
    Serial.printf("[FeedingHistory] Validacion streaming: archivo=%s, objetos=%u, heap=%u bytes.\n",
                  path,
                  static_cast<unsigned>(count),
                  static_cast<unsigned>(ESP.getFreeHeap()));
    return endOfArray;
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

bool FeedingHistoryService::readNextHistoryObject(File& file, String& object, bool& endOfArray) const
{
    object = "";
    endOfArray = false;

    char current = 0;
    bool started = false;
    bool inString = false;
    bool escaped = false;
    int depth = 0;

    while (file.available())
    {
        current = static_cast<char>(file.read());

        if (!started)
        {
            if (current == '[' || current == ',' || current == ' ' || current == '\n' || current == '\r' || current == '\t')
                continue;

            if (current == ']')
            {
                endOfArray = true;
                return false;
            }

            if (current != '{') return false;
            started = true;
            depth = 1;
            object += current;
            continue;
        }

        object += current;

        if (inString)
        {
            if (escaped)
            {
                escaped = false;
            }
            else if (current == '\\')
            {
                escaped = true;
            }
            else if (current == '"')
            {
                inString = false;
            }
            continue;
        }

        if (current == '"')
        {
            inString = true;
        }
        else if (current == '{')
        {
            ++depth;
        }
        else if (current == '}')
        {
            --depth;
            if (depth == 0) return true;
        }
    }

    return false;
}

bool FeedingHistoryService::parseHistoryEvent(const String& object, FeedingEvent& event) const
{
    JsonDocument document;
    const DeserializationError error = deserializeJson(document, object);
    if (error || !document.is<JsonObject>()) return false;

    JsonObject entry = document.as<JsonObject>();
    event.eventId = entry["eventId"] | "";
    event.timestamp = entry["timestamp"] | 0;
    event.portions = entry["portions"] | 0;
    event.synced = entry["synced"] | false;

    const char* source = entry["source"] | "";
    if (strcmp(source, "physical") == 0) event.source = FeedingSource::Physical;
    else if (strcmp(source, "scheduled") == 0) event.source = FeedingSource::Scheduled;
    else if (strcmp(source, "remote") == 0) event.source = FeedingSource::Remote;
    else return false;

    return true;
}

bool FeedingHistoryService::serializeEvent(const FeedingEvent& event, Print& output) const
{
    JsonDocument document;
    JsonObject entry = document.to<JsonObject>();

    entry["eventId"] = event.eventId;
    entry["timestamp"] = event.timestamp;
    entry["portions"] = event.portions;
    entry["synced"] = event.synced;

    switch (event.source)
    {
        case FeedingSource::Physical: entry["source"] = "physical"; break;
        case FeedingSource::Scheduled: entry["source"] = "scheduled"; break;
        case FeedingSource::Remote: entry["source"] = "remote"; break;
        default: return false;
    }

    return serializeJson(document, output) > 0;
}

bool FeedingHistoryService::writeHistoryTempWithAppend(const FeedingEvent& event)
{
    File output = LittleFS.open(HISTORY_TEMP_FILE, "w");
    if (!output) return false;

    const size_t heapStart = ESP.getFreeHeap();
    output.print('[');

    bool first = true;
    size_t copied = 0;

    if (LittleFS.exists(HISTORY_FILE))
    {
        File input = LittleFS.open(HISTORY_FILE, "r");
        if (!input)
        {
            output.close();
            return false;
        }

        String object;
        bool endOfArray = false;
        while (readNextHistoryObject(input, object, endOfArray))
        {
            FeedingEvent existingEvent;
            if (!parseHistoryEvent(object, existingEvent))
            {
                input.close();
                output.close();
                return false;
            }

            if (!first) output.print(',');
            output.print(object);
            first = false;
            ++copied;
        }
        input.close();

        if (!endOfArray)
        {
            output.close();
            return false;
        }
    }

    if (!first) output.print(',');
    if (!serializeEvent(event, output))
    {
        output.close();
        return false;
    }

    output.print(']');
    output.flush();
    const size_t fileSize = output.size();
    output.close();

    Serial.printf("[FeedingHistory] SAVE streaming: eventos existentes=%u, JSON=%u bytes, heap usado max aprox=%d bytes.\n",
                  static_cast<unsigned>(copied),
                  static_cast<unsigned>(fileSize),
                  static_cast<int>(heapStart) - static_cast<int>(ESP.getFreeHeap()));

    return fileSize > 0;
}

bool FeedingHistoryService::writeHistoryTempWithoutEvent(const String& eventId, bool& found)
{
    found = false;

    File input = LittleFS.open(HISTORY_FILE, "r");
    if (!input) return false;

    File output = LittleFS.open(HISTORY_TEMP_FILE, "w");
    if (!output)
    {
        input.close();
        return false;
    }

    output.print('[');
    bool first = true;
    size_t copied = 0;

    String object;
    bool endOfArray = false;
    while (readNextHistoryObject(input, object, endOfArray))
    {
        FeedingEvent existingEvent;
        if (!parseHistoryEvent(object, existingEvent))
        {
            input.close();
            output.close();
            return false;
        }

        if (existingEvent.eventId == eventId)
        {
            found = true;
            continue;
        }

        if (!first) output.print(',');
        output.print(object);
        first = false;
        ++copied;
    }

    input.close();
    if (!endOfArray)
    {
        output.close();
        return false;
    }

    output.print(']');
    output.flush();
    const size_t fileSize = output.size();
    output.close();

    Serial.printf("[FeedingHistory] Limpieza streaming: evento=%s, eliminado=%s, restantes=%u, JSON=%u bytes.\n",
                  eventId.c_str(),
                  found ? "si" : "no",
                  static_cast<unsigned>(copied),
                  static_cast<unsigned>(fileSize));

    return fileSize > 0;
}

bool FeedingHistoryService::writeHistoryDocument(JsonDocument& document)
{
    const size_t jsonSize = measureJson(document);
    Serial.printf("[FeedingHistory] Escritura legacy: JSON=%u bytes, heap=%u bytes.\n",
                  static_cast<unsigned>(jsonSize),
                  static_cast<unsigned>(ESP.getFreeHeap()));

    File file = LittleFS.open(HISTORY_TEMP_FILE, "w");
    if (!file) return false;

    const size_t written = serializeJson(document, file);
    file.flush();
    const size_t fileSize = file.size();
    file.close();

    if (written != jsonSize || fileSize != written) return false;
    return replaceHistoryWith(HISTORY_TEMP_FILE);
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

    if (!loadEventSequence()) return false;
    if (!recoverHistoryFiles()) return false;

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
    if (!file) return false;

    const size_t written = file.print(sequence);
    file.flush();
    file.close();
    return written > 0;
}

bool FeedingHistoryService::trimHistory()
{
    if (!LittleFS.exists(HISTORY_FILE)) return true;

    File input = LittleFS.open(HISTORY_FILE, "r");
    if (!input) return false;
    File output = LittleFS.open(HISTORY_TEMP_FILE, "w");
    if (!output)
    {
        input.close();
        return false;
    }

    output.print('[');
    bool first = true;
    size_t kept = 0;
    size_t removed = 0;
    String object;
    bool endOfArray = false;

    while (readNextHistoryObject(input, object, endOfArray))
    {
        FeedingEvent event;
        if (!parseHistoryEvent(object, event))
        {
            input.close();
            output.close();
            return false;
        }

        if (event.synced && removed < 1 && kept >= MAX_HISTORY_EVENTS)
        {
            ++removed;
            continue;
        }

        if (!first) output.print(',');
        output.print(object);
        first = false;
        ++kept;
    }

    input.close();
    if (!endOfArray)
    {
        output.close();
        return false;
    }

    output.print(']');
    output.flush();
    output.close();

    if (removed == 0) return true;
    return replaceHistoryWith(HISTORY_TEMP_FILE);
}

std::vector<FeedingEvent> FeedingHistoryService::getHistory()
{
    std::vector<FeedingEvent> history;
    if (!LittleFS.exists(HISTORY_FILE)) return history;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return history;

    String object;
    bool endOfArray = false;
    while (readNextHistoryObject(file, object, endOfArray))
    {
        FeedingEvent event;
        if (parseHistoryEvent(object, event)) history.push_back(event);
    }

    file.close();
    return history;
}

bool FeedingHistoryService::getNextPendingEvent(FeedingEvent& event)
{
    if (!LittleFS.exists(HISTORY_FILE)) return false;

    File file = LittleFS.open(HISTORY_FILE, "r");
    if (!file) return false;

    String object;
    bool endOfArray = false;
    while (readNextHistoryObject(file, object, endOfArray))
    {
        FeedingEvent candidate;
        if (!parseHistoryEvent(object, candidate))
        {
            file.close();
            return false;
        }

        if (!candidate.synced)
        {
            event = candidate;
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

bool FeedingHistoryService::markAsSynced(const String& eventId)
{
    if (!LittleFS.exists(HISTORY_FILE)) return false;

    bool found = false;
    if (!writeHistoryTempWithoutEvent(eventId, found)) return false;
    if (!found) return false;

    return replaceHistoryWith(HISTORY_TEMP_FILE);
}

bool FeedingHistoryService::save(const FeedingEvent& event)
{
    const size_t heapStart = ESP.getFreeHeap();
    Serial.printf("[FeedingHistory] SAVE streaming inicio: heap=%u bytes, evento=%s.\n",
                  static_cast<unsigned>(heapStart),
                  event.eventId.c_str());

    if (!writeHistoryTempWithAppend(event))
    {
        Serial.printf("[FeedingHistory] SAVE streaming ERROR: heap final=%u bytes.\n",
                      static_cast<unsigned>(ESP.getFreeHeap()));
        return false;
    }

    const bool result = replaceHistoryWith(HISTORY_TEMP_FILE);
    Serial.printf("[FeedingHistory] SAVE streaming fin: resultado=%s, heap=%u bytes, delta=%d bytes.\n",
                  result ? "OK" : "ERROR",
                  static_cast<unsigned>(ESP.getFreeHeap()),
                  static_cast<int>(heapStart) - static_cast<int>(ESP.getFreeHeap()));
    return result;
}
