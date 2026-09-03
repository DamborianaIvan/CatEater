#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/FeederInfo.h"
#include "domain/FeedingEvent.h"
#include "domain/Configuration.h"
#include "services/BackendConnectionService.h"
#include "storage/DeviceCredentialStorage.h"

class ApiClient
{
   public:
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo,
              BackendConnectionService& backendConnectionService,
              DeviceCredentialStorage& deviceCredentialStorage);

    bool hasDeviceCredential() const;
    bool isBackendAvailable() const;
    bool getFeederInfo(FeederInfo& feederInfo);
    bool getRemoteConfiguration(Configuration& configuration, uint32_t& revision);
    bool getMotorState(bool& motorState, int& portions, String& commandId,
                       uint32_t& configRevision);
    bool completeMotorCommand(const String& commandId);
    bool sendHeartbeat();
    bool syncFeedingEvent(const FeedingEvent& event);

   private:
    HttpClient& _httpClient;
    const DeviceInfo& _deviceInfo;
    BackendConnectionService& _backendConnectionService;
    DeviceCredentialStorage& _deviceCredentialStorage;

    static constexpr char CONTENT_TYPE[] = "application/json";
    static constexpr char BASE_URL[] = "https://cat-feeder.onrender.com";
    static constexpr char DEVICE_HEARTBEAT_ENDPOINT[] = "/feeders/heartbeat";
    static constexpr uint16_t MOTOR_STATE_TIMEOUT_MS = 5000;
    static constexpr uint16_t EVENT_SYNC_TIMEOUT_MS = 5000;
    static constexpr uint16_t BACKGROUND_TIMEOUT_MS = 5000;

    String buildUrl(const String& endpoint) const;
    bool canAttemptRequest() const;
    void updateBackendAvailability(const HttpResponse& response);
};
