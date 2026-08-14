#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/FeederInfo.h"
#include "domain/FeedingEvent.h"

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
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo);

    RegistrationResult registerDevice();
    bool getFeederInfo(FeederInfo& feederInfo);
    bool getMotorState(bool& motorState, int& portions, String& commandId);
    bool completeMotorCommand(const String& commandId);
    bool sendHeartbeat();
    bool syncFeedingEvent(const FeedingEvent& event);

   private:
    HttpClient& _httpClient;
    const DeviceInfo& _deviceInfo;

    static constexpr char CONTENT_TYPE[] = "application/json";
    static constexpr char API_KEY[] = "ExCECoLysoco";
    static constexpr char BASE_URL[] = "http://192.168.1.33:5000";

    static constexpr char REGISTER_ENDPOINT[] = "/feeders/register";

    String buildUrl(const String& endpoint) const;
    String buildRegistrationBody() const;
};