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

bool ApiClient::canAttemptRequest() const
{
    return _backendConnectionService.canAttempt();
}

bool ApiClient::isBackendAvailable() const
{
    return _backendConnectionService.isAvailable();
}

void ApiClient::updateBackendAvailability(const HttpResponse& response)
{
    if (response.isHttpSuccess())
        _backendConnectionService.recordSuccess();
    else if (!response.transportSuccess || response.statusCode >= 500)
        _backendConnectionService.recordFailure();
}

bool ApiClient::getFeederInfo(FeederInfo& feederInfo)
{
    if (!canAttemptRequest())
        return false;

    String endpoint = String("/feeders/global/") + _deviceInfo.getFeederId();
    String deviceCredential;
    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar deviceCredential para feeder global.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers);
    updateBackendAvailability(response);

    if (!response.isHttpSuccess() || response.statusCode != 200)
        return false;

    JsonDocument document;
    if (deserializeJson(document, response.body))
        return false;

    feederInfo.feederQuantity = document["feederQuantity"] | 0;
    feederInfo.feederName = document["feederName"] | "";
    feederInfo.feederLogo = document["feederLogo"] | "";
    feederInfo.lastConnection = document["lastConection"] | "";
    return true;
}

bool ApiClient::getMotorState(bool& motorState, int& portions, String& commandId,
                              uint32_t& configRevision)
{
    if (!canAttemptRequest())
        return false;

    String endpoint = String("/feeders/motor-state/") + _deviceInfo.getFeederId();
    String deviceCredential;
    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar deviceCredential para motor state.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, MOTOR_STATE_TIMEOUT_MS);
    updateBackendAvailability(response);

    if (!response.isHttpSuccess() || response.statusCode != 200)
        return false;

    JsonDocument document;
    if (deserializeJson(document, response.body))
        return false;

    motorState = document["motorState"] | false;
    portions = document["portions"] | 1;
    commandId = document["commandId"] | "";
    configRevision = document["configRevision"] | 0;
    return true;
}

bool ApiClient::getRemoteConfiguration(Configuration& configuration, uint32_t& revision)
{
    if (!canAttemptRequest())
        return false;

    String endpoint = String("/feeders/config/") + _deviceInfo.getFeederId();
    String deviceCredential;

    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar deviceCredential para remote configuration.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
    HttpResponse response = _httpClient.get(buildUrl(endpoint), headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);

    if (!response.isHttpSuccess() || response.statusCode != 200)
        return false;

    JsonDocument document;
    if (deserializeJson(document, response.body) || !document["revision"].is<uint32_t>())
        return false;

    revision = document["revision"].as<uint32_t>();
    Configuration newConfiguration;
    newConfiguration.stepsPerFeed = document["stepsPerFeed"];
    JsonArray schedules = document["schedules"].as<JsonArray>();

    if (schedules.isNull())
        return false;

    for (uint8_t i = 0; i < MAX_SCHEDULES; ++i)
    {
        JsonObject schedule = schedules[i].as<JsonObject>();
        if (schedule.isNull())
            return false;

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
    if (!canAttemptRequest())
        return false;

    String deviceCredential;
    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar deviceCredential para completar comando.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
    headers.emplace_back("Content-Type", "application/json");

    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();
    document["commandId"] = commandId;

    String body;
    serializeJson(document, body);

    HttpResponse response =
        _httpClient.post(buildUrl("/feeder/complete"), body, headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);

    return response.isHttpSuccess();
}

bool ApiClient::sendHeartbeat()
{
    if (!canAttemptRequest())
        return false;

    String deviceCredential;
    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar deviceCredential para heartbeat.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    JsonDocument document;
    document["feederId"] = _deviceInfo.getFeederId();

    String body;
    serializeJson(document, body);

    HttpResponse response =
        _httpClient.post(buildUrl(DEVICE_HEARTBEAT_ENDPOINT), body, headers, BACKGROUND_TIMEOUT_MS);
    updateBackendAvailability(response);

    if (response.isHttpSuccess())
        return true;

    if (response.statusCode == 401)
        Serial.println("[ApiClient] Heartbeat rechazado: deviceCredential invalida o ausente.");
    else if (response.statusCode == 404)
        Serial.println("[ApiClient] Heartbeat rechazado: feeder no encontrado.");
    else
        Serial.println("[ApiClient] Error al enviar heartbeat autenticado.");

    return false;
}

bool ApiClient::syncFeedingEvent(const FeedingEvent& event)
{
    if (!canAttemptRequest())
        return false;

    String deviceCredential;
    if (!_deviceCredentialStorage.load(deviceCredential))
    {
        Serial.println(
            "[ApiClient] No se pudo cargar deviceCredential para sincronizar historial.");
        return false;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-credential", deviceCredential);
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
        _httpClient.post(buildUrl("/feeders/history"), body, headers, EVENT_SYNC_TIMEOUT_MS);
    updateBackendAvailability(response);

    return response.isHttpSuccess();
}

bool ApiClient::hasDeviceCredential() const
{
    String credential;
    return _deviceCredentialStorage.load(credential);
}
