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

    document["feederId"] = _deviceInfo.getFeederId();
    document["feederName"] = _deviceInfo.getModel();

    String json;

    serializeJson(document, json);

    return json;
}

bool ApiClient::getFeederInfo(FeederInfo& feederInfo)
{
    String endpoint = String("/feeders/global/") + _deviceInfo.getFeederId();

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

bool ApiClient::getMotorState(bool& motorState, int& portions, String& commandId,
                              uint32_t& configRevision)
{
    String endpoint = String("/feeders/motor-state/") + _deviceInfo.getFeederId();

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);

    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, MOTOR_STATE_TIMEOUT_MS);

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
    configRevision = document["configRevision"] | 0;

    return true;
}

bool ApiClient::getRemoteConfiguration(Configuration& configuration, uint32_t& revision)
{
    String endpoint = String("/feeders/config/") + _deviceInfo.getFeederId();

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);

    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, BACKGROUND_TIMEOUT_MS);

    if (!response.success || response.statusCode != 200)
    {
        return false;
    }

    JsonDocument document;
    DeserializationError error = deserializeJson(document, response.body);

    if (error || !document["revision"].is<uint32_t>())
    {
        return false;
    }

    revision = document["revision"].as<uint32_t>();

    Configuration newConfiguration;
    newConfiguration.stepsPerFeed = document["stepsPerFeed"];

    JsonArray schedules = document["schedules"].as<JsonArray>();

    if (schedules.isNull())
    {
        return false;
    }

    for (uint8_t i = 0; i < MAX_SCHEDULES; ++i)
    {
        JsonObject schedule = schedules[i].as<JsonObject>();

        if (schedule.isNull())
        {
            return false;
        }

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
    String endpoint = "/feeder/complete";

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");

    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();
    document["commandId"] = commandId;

    String body;
    serializeJson(document, body);

    HttpResponse response =
        _httpClient.post(buildUrl(endpoint), body, headers, BACKGROUND_TIMEOUT_MS);

    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

bool ApiClient::sendHeartbeat()
{
    const String endpoint = "/feeders/heartbeat";

    HttpHeaders headers;
    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", "application/json");

    const String body = "{\"feederId\":\"" + _deviceInfo.getFeederId() + "\"}";

    HttpResponse response =
        _httpClient.post(buildUrl(endpoint), body, headers, BACKGROUND_TIMEOUT_MS);

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
    document["feederId"] = _deviceInfo.getFeederId();
    document["timestamp"] = event.timestamp;
    document["portions"] = event.portions;
    document["source"] = source;

    String body;
    serializeJson(document, body);

    HttpResponse response =
        _httpClient.post(buildUrl(endpoint), body, headers, EVENT_SYNC_TIMEOUT_MS);

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
