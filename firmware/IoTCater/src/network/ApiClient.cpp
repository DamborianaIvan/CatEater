#include "network/ApiClient.h"

#include <ArduinoJson.h>

ApiClient::ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo,
                     BackendConnectionService& backendConnectionService,
                     DeviceCredentialStorage& deviceCredentialStorage)
    : _httpClient(httpClient),
      _deviceInfo(deviceInfo),
      _backendConnectionService(backendConnectionService),
      _deviceCredentialStorage(deviceCredentialStorage)
{
}

String ApiClient::buildUrl(const String& endpoint) const
{
    return String(BASE_URL) + endpoint;
}

String ApiClient::buildRegistrationBody() const
{
    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();
    document["feederName"] = _deviceInfo.getModel();
    String json;
    serializeJson(document, json);
    return json;
}

bool ApiClient::canAttemptRequest() const
{
    return _backendConnectionService.canAttempt();
}

void ApiClient::updateBackendAvailability(const HttpResponse& response)
{
    if (response.success && response.statusCode > 0) _backendConnectionService.recordSuccess();
    else _backendConnectionService.recordFailure();
}

bool ApiClient::getFeederInfo(FeederInfo& feederInfo)
{
    if (!canAttemptRequest()) return false;
    String endpoint = String("/feeders/global/") + _deviceInfo.getFeederId();
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers);
    updateBackendAvailability(response);
    if (!response.success || response.statusCode != 200) return false;
    JsonDocument document;
    if (deserializeJson(document, response.body)) return false;
    feederInfo.feederQuantity = document["feederQuantity"] | 0;
    feederInfo.feederName = document["feederName"] | "";
    feederInfo.feederLogo = document["feederLogo"] | "";
    feederInfo.lastConnection = document["lastConection"] | "";
    return true;
}

bool ApiClient::getMotorState(bool& motorState, int& portions, String& commandId, uint32_t& configRevision)
{
    if (!canAttemptRequest()) return false;
    String endpoint = String("/feeders/motor-state/") + _deviceInfo.getFeederId();
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, MOTOR_STATE_TIMEOUT_MS);
    updateBackendAvailability(response);
    if (!response.success || response.statusCode != 200) return false;
    JsonDocument document;
    if (deserializeJson(document, response.body)) return false;
    motorState = document["motorState"] | false;
    portions = document["portions"] | 1;
    commandId = document["commandId"] | "";
    configRevision = document["configRevision"] | 0;
    return true;
}

bool ApiClient::getRemoteConfiguration(Configuration& configuration, uint32_t& revision)
{
    if (!canAttemptRequest()) return false;
    String endpoint = String("/feeders/config/") + _deviceInfo.getFeederId();
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);
    if (!response.success || response.statusCode != 200) return false;
    JsonDocument document;
    if (deserializeJson(document, response.body) || !document["revision"].is<uint32_t>()) return false;
    revision = document["revision"].as<uint32_t>();
    Configuration newConfiguration;
    newConfiguration.stepsPerFeed = document["stepsPerFeed"];
    JsonArray schedules = document["schedules"].as<JsonArray>();
    if (schedules.isNull()) return false;
    for (uint8_t i = 0; i < MAX_SCHEDULES; ++i)
    {
        JsonObject schedule = schedules[i].as<JsonObject>();
        if (schedule.isNull()) return false;
        newConfiguration.schedules[i].hour = schedule["hour"] | 0;
        newConfiguration.schedules[i].minute = schedule["minute"] | 0;
        newConfiguration.schedules[i].portions = schedule["portions"] | 1;
        newConfiguration.schedules[i].enabled = schedule["enabled"] | false;
    }
    configuration = newConfiguration;
    return true;
}

bool ApiClient::completeMotorCommand(const String& commandId)
{
    if (!canAttemptRequest()) return false;
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");
    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();
    document["commandId"] = commandId;
    String body;
    serializeJson(document, body);
    HttpResponse response = _httpClient.post(buildUrl("/feeder/complete"), body, headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

bool ApiClient::sendHeartbeat()
{
    if (!canAttemptRequest()) return false;
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");
    const String body = "{\"feederId\":\"" + _deviceInfo.getFeederId() + "\"}";
    HttpResponse response = _httpClient.post(buildUrl("/feeders/heartbeat"), body, headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

bool ApiClient::syncFeedingEvent(const FeedingEvent& event)
{
    if (!canAttemptRequest()) return false;
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");
    String source;
    switch (event.source)
    {
        case FeedingSource::Physical: source = "physical"; break;
        case FeedingSource::Scheduled: source = "scheduled"; break;
        case FeedingSource::Remote: source = "remote"; break;
    }
    JsonDocument document;
    document["eventId"] = event.eventId;
    document["feederId"] = _deviceInfo.getFeederId();
    document["timestamp"] = event.timestamp;
    document["portions"] = event.portions;
    document["source"] = source;
    String body;
    serializeJson(document, body);
    HttpResponse response = _httpClient.post(buildUrl("/feeders/history"), body, headers, EVENT_SYNC_TIMEOUT_MS);
    updateBackendAvailability(response);
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

RegistrationResult ApiClient::registerDevice()
{
    if (!canAttemptRequest()) return RegistrationResult::ConnectionError;

    const String body = buildRegistrationBody();
    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    HttpResponse response = _httpClient.post(buildUrl(REGISTER_ENDPOINT), body, headers);
    updateBackendAvailability(response);

    if (!response.success) return RegistrationResult::ConnectionError;
    if (response.statusCode == 401) return RegistrationResult::Unauthorized;
    if (response.statusCode == 400) return RegistrationResult::InvalidData;
    if (response.statusCode >= 500) return RegistrationResult::ServerError;
    if (response.statusCode == 201) return RegistrationResult::Registered;
    if (response.statusCode == 409) return RegistrationResult::AlreadyRegistered;

    return RegistrationResult::ServerError;
}

EnrollmentResult ApiClient::enrollDevice()
{
    if (!canAttemptRequest()) return EnrollmentResult::ConnectionError;

    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();

    String body;
    serializeJson(document, body);

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    HttpResponse response = _httpClient.post(buildUrl(ENROLL_ENDPOINT), body, headers);
    updateBackendAvailability(response);

    if (!response.success) return EnrollmentResult::ConnectionError;
    if (response.statusCode == 401) return EnrollmentResult::Unauthorized;
    if (response.statusCode == 404) return EnrollmentResult::NotFound;
    if (response.statusCode == 409) return EnrollmentResult::AlreadyEnrolled;
    if (response.statusCode >= 500) return EnrollmentResult::ServerError;
    if (response.statusCode != 201) return EnrollmentResult::ServerError;

    JsonDocument responseDocument;
    if (deserializeJson(responseDocument, response.body)) return EnrollmentResult::ServerError;

    const char* credential = responseDocument["deviceCredential"] | nullptr;
    if (!credential || !DeviceCredentialStorage::isValid(String(credential)))
    {
        return EnrollmentResult::ServerError;
    }

    if (!_deviceCredentialStorage.save(String(credential)))
    {
        return EnrollmentResult::ServerError;
    }

    return EnrollmentResult::Enrolled;
}

bool ApiClient::hasDeviceCredential() const
{
    String credential;
    return _deviceCredentialStorage.load(credential);
}
