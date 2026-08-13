#pragma once

#include "network/ApiClient.h"
#include "FeedingHistoryService.h"

class SyncService
{
   public:
    SyncService(ApiClient& apiClient, FeedingHistoryService& historyService);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;
    FeedingHistoryService& _historyService;

    unsigned long _lastSync = 0;

    static constexpr unsigned long SYNC_INTERVAL = 10000;
};