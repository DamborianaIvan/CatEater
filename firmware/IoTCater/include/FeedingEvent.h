#pragma once

#include <Arduino.h>
#include "FeedingSource.h"

struct FeedingEvent
{
    String eventId;
    time_t timestamp;
    uint8_t portions;
    FeedingSource source;
    bool synced;
};