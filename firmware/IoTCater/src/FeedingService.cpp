#include "FeedingService.h"

FeedingService::FeedingService(Motor& motor, FeedingHistoryService& historyService)
    : _motor(motor), _historyService(historyService)
{
}

bool FeedingService::feed(int portions, FeedingSource source)
{
    if (portions <= 0)
    {
        return false;
    }

    if (!_motor.feed(portions))
    {
        return false;
    }

    FeedingEvent event{time(nullptr), static_cast<uint8_t>(portions), source};

    if (!_historyService.save(event))
    {
        Serial.println("[FeedingService] Advertencia: no se pudo guardar historial.");
    }

    return true;
}

bool FeedingService::isFeeding() const
{
    return _motor.isFeeding();
}