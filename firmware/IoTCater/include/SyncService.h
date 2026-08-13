#pragma once

#include "network/ApiClient.h"
#include "FeedingHistoryService.h"
#include "TimeService.h"

class SyncService
{
   public:
    SyncService(ApiClient& apiClient, FeedingHistoryService& historyService,
                TimeService& timeService);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;
    FeedingHistoryService& _historyService;
    TimeService& _timeService;

    unsigned long _lastSync = 0;

    static constexpr unsigned long SYNC_INTERVAL = 10000;
};