#pragma once

#include "network/ApiClient.h"

class HeartbeatService
{
   public:
    explicit HeartbeatService(ApiClient& apiClient);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;

    unsigned long _lastHeartbeat = 0;

    static constexpr unsigned long HEARTBEAT_INTERVAL = 10000;
};