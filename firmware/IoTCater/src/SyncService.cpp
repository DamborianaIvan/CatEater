#include "SyncService.h"

SyncService::SyncService(ApiClient& apiClient, FeedingHistoryService& historyService)
    : _apiClient(apiClient), _historyService(historyService)
{
}

void SyncService::begin()
{
    _lastSync = 0;

    Serial.println("[SyncService] Iniciado.");
}

void SyncService::update()
{
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
            _historyService.markAsSynced(event.timestamp);

            Serial.println("[SyncService] Evento sincronizado.");
        }
        else
        {
            Serial.println("[SyncService] Error sincronizando evento.");

            break;
        }
    }
}