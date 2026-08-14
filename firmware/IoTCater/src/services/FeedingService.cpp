#include "services/FeedingService.h"

FeedingService::FeedingService(Motor& motor, FeedingHistoryService& historyService,
                               TimeService& timeService)
    : _motor(motor), _historyService(historyService), _timeService(timeService)
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

    const String eventId = _historyService.createEventId();
    if (eventId.isEmpty())
    {
        Serial.println("[FeedingService] Advertencia: no se pudo generar eventId; historial omitido.");
        return true;
    }

    time_t timestamp = _timeService.getTimestamp();
    FeedingEvent event{eventId, timestamp, static_cast<uint8_t>(portions), source, false};

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
