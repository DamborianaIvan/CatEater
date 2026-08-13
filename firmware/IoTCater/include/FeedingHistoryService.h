#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include "FeedingEvent.h"
#include <vector>

class FeedingHistoryService
{
   public:
    bool begin();
    bool save(const FeedingEvent& event);
    bool markAsSynced(const String& eventId);
    bool updatePendingTimestamps(time_t timestamp);

    std::vector<FeedingEvent> getHistory();
    std::vector<FeedingEvent> getPendingEvents();

   private:
    static constexpr const char* HISTORY_FILE = "/feeding_history.json";
};