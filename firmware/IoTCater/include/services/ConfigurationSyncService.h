#pragma once

#include <Arduino.h>

#include "domain/Configuration.h"
#include "hardware/Motor.h"
#include "network/ApiClient.h"
#include "storage/ConfigurationRevisionStorage.h"
#include "storage/ConfigurationStorage.h"

class ConfigurationSyncService
{
   public:
    ConfigurationSyncService(ApiClient& apiClient, ConfigurationStorage& configurationStorage,
                             ConfigurationRevisionStorage& revisionStorage,
                             Configuration& configuration, Motor& motor);

    void begin();
    void update(uint32_t remoteRevision);

   private:
    ApiClient& _apiClient;
    ConfigurationStorage& _configurationStorage;
    ConfigurationRevisionStorage& _revisionStorage;
    Configuration& _configuration;
    Motor& _motor;

    uint32_t _localRevision = 0;

    bool applyRemoteConfiguration(uint32_t remoteRevision);
};