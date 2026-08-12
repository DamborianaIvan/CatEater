#pragma once

#include <Arduino.h>

#include "network/ApiClient.h"
#include "Motor.h"

class RemoteStateService
{
   public:
    explicit RemoteStateService(ApiClient& apiClient, Motor& motor);

    void update();

   private:
    ApiClient& _apiClient;
    Motor& _motor;

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