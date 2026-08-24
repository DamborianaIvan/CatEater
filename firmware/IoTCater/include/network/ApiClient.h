#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"
#include "network/FeederInfo.h"
#include "domain/FeedingEvent.h"
#include "domain/Configuration.h"
#include "services/BackendConnectionService.h"
#include "storage/DeviceCredentialStorage.h"
#include "storage/BootstrapCredentialStorage.h"

enum class RegistrationResult
{
    Registered,
    AlreadyRegistered,
    Unauthorized,
    InvalidData,
    ServerError,
    ConnectionError
};

enum class EnrollmentResult
{
    Enrolled,
    AlreadyEnrolled,
    Unauthorized,
    NotFound,
    ServerError,
    ConnectionError
};

class ApiClient
{
   public:
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo,
              BackendConnectionService& backendConnectionService,
              DeviceCredentialStorage& deviceCredentialStorage,
              BootstrapCredentialStorage& bootstrapCredentialStorage);

    RegistrationResult registerDevice();
    EnrollmentResult enrollDevice();
    bool hasDeviceCredential() const;
    bool hasBootstrapCredential() const;
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
    BootstrapCredentialStorage& _bootstrapCredentialStorage;

    static constexpr char CONTENT_TYPE[] = "application/json";
    static constexpr char BASE_URL[] = "http://192.168.1.39:5000";

    static constexpr char REGISTER_ENDPOINT[] = "/feeders/register";
    static constexpr char ENROLL_ENDPOINT[] = "/feeders/enroll";
    static constexpr char DEVICE_HEARTBEAT_ENDPOINT[] = "/feeders/heartbeat";
    static constexpr uint16_t MOTOR_STATE_TIMEOUT_MS = 500;
    static constexpr uint16_t EVENT_SYNC_TIMEOUT_MS = 1000;
    static constexpr uint16_t BACKGROUND_TIMEOUT_MS = 1000;

    String buildUrl(const String& endpoint) const;
    String buildRegistrationBody() const;

    bool canAttemptRequest() const;
    void updateBackendAvailability(const HttpResponse& response);
};
