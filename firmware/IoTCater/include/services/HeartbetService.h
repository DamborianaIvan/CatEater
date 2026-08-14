#pragma once

#include "network/ApiClient.h"
#include "services/WifiService.h"

class HeartbeatService
{
   public:
    HeartbeatService(ApiClient& apiClient, WiFiService& wifiService);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;
    WiFiService& _wifiService;

    unsigned long _lastHeartbeat = 0;

    static constexpr unsigned long HEARTBEAT_INTERVAL = 10000;
};
