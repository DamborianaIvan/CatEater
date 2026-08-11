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

    unsigned long _lastRequest = 0;

    bool _previousMotorState = false;
    bool motorState = false;
    bool _remoteFeedInProgress = false;

    int portions = 1;

    static constexpr unsigned long POLL_INTERVAL = 5000;

    void pollMotorState();
};