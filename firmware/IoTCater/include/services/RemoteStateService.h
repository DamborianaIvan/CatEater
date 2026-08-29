#pragma once

#include <Arduino.h>

#include "network/ApiClient.h"
#include "services/FeedingService.h"
#include "services/WifiService.h"
#include "storage/RemoteCommandStorage.h"
#include "services/ConfigurationSyncService.h"

class RemoteStateService
{
   public:
    explicit RemoteStateService(ApiClient& apiClient, FeedingService& feedingService,
                                WiFiService& wifiService, RemoteCommandStorage& commandStorage,
                                ConfigurationSyncService& configurationSyncService);
    void update();
    bool loadCommand();

   private:
    ApiClient& _apiClient;
    FeedingService& _feedingService;
    WiFiService& _wifiService;
    RemoteCommandStorage& _commandStorage;
    ConfigurationSyncService& _configurationSyncService;

    static constexpr unsigned long CONFIRMATION_RETRY_INTERVAL = 5000;
    static constexpr unsigned long POLL_INTERVAL = 5000;

    unsigned long _lastConfirmationAttempt = 0;
    unsigned long _lastRequest = 0;

    bool _remoteFeedInProgress = false;

    String _activeCommandId;

    RemoteCommand _command;
    bool _hasCommand = false;

    void pollMotorState();
    bool processCommand(const String& commandId, int portions);
};
