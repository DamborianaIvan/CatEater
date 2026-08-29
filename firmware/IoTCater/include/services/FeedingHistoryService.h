#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#include "domain/FeedingEvent.h"

class FeedingHistoryService
{
   public:
    bool begin();
    String createEventId();
    bool save(const FeedingEvent& event);
    bool markAsSynced(const String& eventId);
    bool trimHistory();

    std::vector<FeedingEvent> getHistory();
    bool getNextPendingEvent(FeedingEvent& event);

   private:
    static constexpr const char* HISTORY_FILE = "/feeding_history.json";
    static constexpr const char* HISTORY_TEMP_FILE = "/feeding_history.tmp";
    static constexpr const char* HISTORY_BACKUP_FILE = "/feeding_history.bak";
    static constexpr const char* HISTORY_CORRUPT_FILE = "/feeding_history.corrupt";
    static constexpr const char* EVENT_SEQUENCE_FILE = "/feeding_event_sequence.txt";
    static constexpr size_t MAX_HISTORY_EVENTS = 100;

    uint32_t _eventSequence = 0;
    bool _eventIdGenerationAvailable = false;

    bool loadEventSequence();
    bool saveEventSequence(uint32_t sequence);
    bool recoverHistoryFiles();
    bool isValidHistoryFile(const char* path) const;
    bool preserveCorruptHistoryFile(const char* path, const char* preservedPath);
    bool replaceHistoryWith(const char* replacementPath);
    bool writeHistoryTemp(JsonDocument& document);
    bool writeHistoryDocument(JsonDocument& document);
};
