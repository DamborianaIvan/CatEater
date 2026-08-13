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
    std::vector<FeedingEvent> getHistory();
    bool synced;

   private:
    static constexpr const char* HISTORY_FILE = "/feeding_history.json";
};