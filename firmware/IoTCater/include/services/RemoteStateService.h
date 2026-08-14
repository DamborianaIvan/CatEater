#pragma once

#include <Arduino.h>

#include "network/ApiClient.h"
#include "FeedingService.h"
#include "WifiServices.h"

class RemoteStateService
{
   public:
    explicit RemoteStateService(ApiClient& apiClient, FeedingService& feedingService,
                                WiFiService& wifiService);
    void update();

   private:
    ApiClient& _apiClient;
    FeedingService& _feedingService;
    WiFiService& _wifiService;

    static constexpr unsigned long CONFIRMATION_RETRY_INTERVAL = 5000;

    unsigned long _lastConfirmationAttempt = 0;
    unsigned long _lastRequest = 0;

    bool motorState = false;
    bool _remoteFeedInProgress = false;

    String _activeCommandId;
    String _lastCommandId;

    int portions = 1;

    static constexpr unsigned long POLL_INTERVAL = 5000;

    void pollMotorState();
};