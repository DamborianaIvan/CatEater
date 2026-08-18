#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/FeederInfo.h"
#include "domain/FeedingEvent.h"
#include "domain/Configuration.h"
#include "services/BackendConnectionService.h"

enum class RegistrationResult
{
    Registered,
    AlreadyRegistered,
    Unauthorized,
    InvalidData,
    ServerError,
    ConnectionError
};

class ApiClient
{
   public:
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo,
              BackendConnectionService& backendConnectionService);

    RegistrationResult registerDevice();
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

    static constexpr char CONTENT_TYPE[] = "application/json";
    static constexpr char API_KEY[] = "ExCECoLysoco";
    static constexpr char BASE_URL[] = "http://192.168.1.38:5000";

    static constexpr char REGISTER_ENDPOINT[] = "/feeders/register";
    static constexpr uint16_t MOTOR_STATE_TIMEOUT_MS = 500;
    static constexpr uint16_t EVENT_SYNC_TIMEOUT_MS = 1000;
    static constexpr uint16_t BACKGROUND_TIMEOUT_MS = 1000;

    String buildUrl(const String& endpoint) const;
    String buildRegistrationBody() const;

    bool canAttemptRequest() const;
    void updateBackendAvailability(const HttpResponse& response);
};
