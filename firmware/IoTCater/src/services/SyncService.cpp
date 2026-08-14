#include "services/SyncService.h"

SyncService::SyncService(ApiClient& apiClient, FeedingHistoryService& historyService,
                         TimeService& timeService, WiFiService& wifiService)
    : _apiClient(apiClient),
      _historyService(historyService),
      _timeService(timeService),
      _wifiService(wifiService)
{
}

void SyncService::begin()
{
    _lastSync = 0;
}

void SyncService::update()
{
    if (!_wifiService.isConnected())
    {
        return;
    }

    if (millis() - _lastSync < SYNC_INTERVAL)
    {
        return;
    }

    _lastSync = millis();

    const auto pendingEvents = _historyService.getPendingEvents();

    if (pendingEvents.empty())
    {
        return;
    }

    for (const auto& event : pendingEvents)
    {
        if (_apiClient.syncFeedingEvent(event))
        {
            if (_historyService.markAsSynced(event.eventId))
            {
                Serial.println("[SyncService] Evento sincronizado.");
            }
        }
        else
        {
            Serial.println("[SyncService] Error sincronizando evento.");
            break;
        }
    }
}
