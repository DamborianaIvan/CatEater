#pragma once

#include "network/ApiClient.h"
#include "services/FeedingHistoryService.h"
#include "services/TimeService.h"
#include "services/WifiService.h"
#include "services/DiagnosticService.h"

class SyncService
{
   public:
    SyncService(ApiClient& apiClient, FeedingHistoryService& historyService, TimeService& timeService,
                WiFiService& wifiService, DiagnosticService& diagnostics);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;
    FeedingHistoryService& _historyService;
    TimeService& _timeService;
    WiFiService& _wifiService;
    DiagnosticService& _diagnostics;

    unsigned long _lastSync = 0;

    static constexpr unsigned long SYNC_INTERVAL = 10000;
};
