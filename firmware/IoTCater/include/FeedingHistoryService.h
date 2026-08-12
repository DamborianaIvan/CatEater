#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include "FeedingEvent.h"

class FeedingHistoryService
{
   public:
    bool begin();
    bool save(const FeedingEvent& event);

   private:
    static constexpr const char* HISTORY_FILE = "/feeding_history.json";
};