#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <vector>

#include "FeedingEvent.h"

class FeedingHistoryService
{
   public:
    bool begin();
    bool save(const FeedingEvent& event);
    bool markAsSynced(const String& eventId);
    bool trimHistory();

    std::vector<FeedingEvent> getHistory();
    std::vector<FeedingEvent> getPendingEvents();

   private:
    static constexpr const char* HISTORY_FILE = "/feeding_history.json";
    static constexpr size_t MAX_HISTORY_EVENTS = 100;
};