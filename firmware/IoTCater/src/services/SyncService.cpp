#include "services/SyncService.h"

SyncService::SyncService(ApiClient& apiClient, FeedingHistoryService& historyService,
                         TimeService& timeService, WiFiService& wifiService,
                         DiagnosticService& diagnostics)
    : _apiClient(apiClient),
      _historyService(historyService),
      _timeService(timeService),
      _wifiService(wifiService),
      _diagnostics(diagnostics)
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
            _diagnostics.record("SYNC_ERROR", event.eventId.c_str());
            break;
        }

        if (!_historyService.markAsSynced(event.eventId))
        {
            _diagnostics.record("SYNC_MARK_ERROR", event.eventId.c_str());
            break;
        }
    }
}
