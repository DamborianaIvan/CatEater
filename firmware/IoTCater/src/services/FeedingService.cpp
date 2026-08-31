#include "services/FeedingService.h"
#include "domain/Configuration.h"

FeedingService::FeedingService(Motor& motor, FeedingHistoryService& historyService,
                               TimeService& timeService, DiagnosticService& diagnostics)
    : _motor(motor), _historyService(historyService), _timeService(timeService), _diagnostics(diagnostics)
{
}

bool FeedingService::feed(int portions, FeedingSource source)
{
    if (!Configuration::isValidPortions(portions))
    {
        _diagnostics.record("FEEDING_INVALID_PORTIONS");
        return false;
    }

    if (!_motor.feed(portions))
    {
        _diagnostics.record("FEEDING_MOTOR_ERROR");
        return false;
    }

    const String eventId = _historyService.createEventId();
    if (eventId.isEmpty())
    {
        Serial.println("[FeedingService] Advertencia: no se pudo generar eventId; historial omitido.");
        _diagnostics.record("FEEDING_EVENT_ID_ERROR");
        return true;
    }

    time_t timestamp = _timeService.getTimestamp();
    FeedingEvent event{eventId, timestamp, static_cast<uint8_t>(portions), source, false};

    if (!_historyService.save(event))
    {
        Serial.println("[FeedingService] Advertencia: no se pudo guardar historial.");
        _diagnostics.record("FEEDING_HISTORY_ERROR", eventId.c_str());
    }

    return true;
}

bool FeedingService::isFeeding() const
{
    return _motor.isFeeding();
}
