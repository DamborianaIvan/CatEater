#include "network/ApiClient.h"

#include <ArduinoJson.h>

ApiClient::ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo)
    : _httpClient(httpClient), _deviceInfo(deviceInfo)
{
}

String ApiClient::buildUrl(const String& endpoint) const
{
    return String(BASE_URL) + endpoint;
}

String ApiClient::buildRegistrationBody() const
{
    JsonDocument document;

    document["feederId"] = _deviceInfo.getDeviceId();
    document["feederName"] = _deviceInfo.getModel();

    String json;

    serializeJson(document, json);

    return json;
}

bool ApiClient::getFeederInfo(FeederInfo& feederInfo)
{
    String endpoint = String("/feeders/global/") + _deviceInfo.getDeviceId();

    HttpHeaders headers;

    headers.emplace_back("x-api-key", API_KEY);

    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers);

    if (!response.success || response.statusCode != 200)
    {
        return false;
    }

    JsonDocument document;

    DeserializationError error = deserializeJson(document, response.body);

    if (error)
    {
        return false;
    }

    feederInfo.feederQuantity = document["feederQuantity"] | 0;

    feederInfo.feederName = document["feederName"] | "";

    feederInfo.feederLogo = document["feederLogo"] | "";

    feederInfo.lastConnection = document["lastConection"] | "";

    return true;
}

bool ApiClient::getMotorState(bool& motorState, int& portions, String& commandId)
{
    String endpoint = String("/feeder/motor-state/") + _deviceInfo.getDeviceId();

    HttpHeaders headers;

    headers.emplace_back("x-api-key", API_KEY);

    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, 500);
    if (!response.success || response.statusCode != 200)
    {
        return false;
    }

    JsonDocument document;

    DeserializationError error = deserializeJson(document, response.body);

    if (error)
    {
        return false;
    }

    motorState = document["motorState"] | false;
    portions = document["portions"] | 1;
    commandId = document["commandId"] | "";
    return true;
}

bool ApiClient::completeMotorCommand(const String& commandId)
{
    String endpoint = "/feeder/complete";

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");

    JsonDocument document;

    document["feederId"] = _deviceInfo.getDeviceId();
    document["commandId"] = commandId;

    String body;
    serializeJson(document, body);

    HttpResponse response = _httpClient.post(buildUrl(endpoint), body, headers);
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

bool ApiClient::sendHeartbeat()
{
    const String endpoint = "/feeders/heartbeat";

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");

    const String body = "{\"feederId\":\"" + _deviceInfo.getDeviceId() + "\"}";

    HttpResponse response = _httpClient.post(buildUrl(endpoint), body, headers);

    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

bool ApiClient::syncFeedingEvent(const FeedingEvent& event)
{
    const String endpoint = "/feeders/history";

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");

    String source;

    switch (event.source)
    {
        case FeedingSource::Physical:
            source = "physical";
            break;

        case FeedingSource::Scheduled:
            source = "scheduled";
            break;

        case FeedingSource::Remote:
            source = "remote";
            break;
    }
    JsonDocument document;

    document["eventId"] = event.eventId;
    document["feederId"] = _deviceInfo.getDeviceId();
    document["timestamp"] = event.timestamp;
    document["portions"] = event.portions;
    document["source"] = source;

    String body;
    serializeJson(document, body);

    HttpResponse response = _httpClient.post(buildUrl(endpoint), body, headers, 1000);

    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

RegistrationResult ApiClient::registerDevice()
{
    String body = buildRegistrationBody();

    HttpHeaders headers;

    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    HttpResponse response = _httpClient.post(buildUrl(REGISTER_ENDPOINT), body, headers);

    if (!response.success)
    {
        return RegistrationResult::ConnectionError;
    }

    switch (response.statusCode)
    {
        case 201:
            return RegistrationResult::Registered;

        case 409:
            return RegistrationResult::AlreadyRegistered;

        case 401:
            return RegistrationResult::Unauthorized;

        case 400:
            return RegistrationResult::InvalidData;

        case 500:
            return RegistrationResult::ServerError;

        default:
            return RegistrationResult::ServerError;
    }
}