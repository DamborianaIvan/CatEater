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

    FeedingEvent event;
    while (_historyService.getNextPendingEvent(event))
    {
        if (!_apiClient.syncFeedingEvent(event))
        {
            break;
        }

        if (!_historyService.markAsSynced(event.eventId))
        {
            break;
        }
    }
}
