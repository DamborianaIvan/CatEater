#pragma once

#include "network/ApiClient.h"
#include "services/WifiService.h"
#include "services/DiagnosticService.h"

class HeartbeatService
{
   public:
    HeartbeatService(ApiClient& apiClient, WiFiService& wifiService, DiagnosticService& diagnostics);

    void begin();
    void update();

   private:
    ApiClient& _apiClient;
    WiFiService& _wifiService;
    DiagnosticService& _diagnostics;

    unsigned long _lastHeartbeat = 0;

    static constexpr unsigned long HEARTBEAT_INTERVAL = 10000;
};
