#include "network/ApiClient.h"

#include <ArduinoJson.h>

ApiClient::ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo,
                     BackendConnectionService& backendConnectionService,
                     DeviceCredentialStorage& deviceCredentialStorage,
                     BootstrapCredentialStorage& bootstrapCredentialStorage)
    : _httpClient(httpClient),
      _deviceInfo(deviceInfo),
      _backendConnectionService(backendConnectionService),
      _deviceCredentialStorage(deviceCredentialStorage),
      _bootstrapCredentialStorage(bootstrapCredentialStorage)
{
}

String ApiClient::buildUrl(const String& endpoint) const
{
    return String(BASE_URL) + endpoint;
}

String ApiClient::buildRegistrationBody() const
{
    JsonDocument document;
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
    if (response.success && response.statusCode > 0)
        _backendConnectionService.recordSuccess();
    else
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
    if (!response.success || response.statusCode != 200)
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
    if (!response.success || response.statusCode != 200)
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
    if (!response.success || response.statusCode != 200)
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
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
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

    if (response.success && response.statusCode >= 200 && response.statusCode < 300)
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
    return response.success && response.statusCode >= 200 && response.statusCode < 300;
}

RegistrationResult ApiClient::registerDevice()
{
    if (!canAttemptRequest())
        return RegistrationResult::ConnectionError;

    String bootstrapCredential;
    if (!_bootstrapCredentialStorage.load(bootstrapCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar bootstrap credential para registro.");
        return RegistrationResult::Unauthorized;
    }

    const String body = buildRegistrationBody();
    HttpHeaders headers;
    headers.emplace_back("x-device-bootstrap", bootstrapCredential);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    HttpResponse response = _httpClient.post(buildUrl(REGISTER_ENDPOINT), body, headers);
    updateBackendAvailability(response);

    if (!response.success)
        return RegistrationResult::ConnectionError;
    if (response.statusCode == 401 || response.statusCode == 403)
        return RegistrationResult::Unauthorized;
    if (response.statusCode == 400)
        return RegistrationResult::InvalidData;
    if (response.statusCode >= 500)
        return RegistrationResult::ServerError;
    if (response.statusCode == 201)
        return RegistrationResult::Registered;
    if (response.statusCode == 409)
        return RegistrationResult::AlreadyRegistered;

    return RegistrationResult::ServerError;
}

EnrollmentResult ApiClient::enrollDevice()
{
    if (!canAttemptRequest())
        return EnrollmentResult::ConnectionError;

    String bootstrapCredential;
    if (!_bootstrapCredentialStorage.load(bootstrapCredential))
    {
        Serial.println("[ApiClient] No se pudo cargar bootstrap credential para enrollment.");
        return EnrollmentResult::Unauthorized;
    }

    HttpHeaders headers;
    headers.emplace_back("x-device-bootstrap", bootstrapCredential);
    headers.emplace_back("Content-Type", CONTENT_TYPE);

    Serial.println("[ApiClient] Iniciando enrollment del dispositivo...");

    HttpResponse response = _httpClient.post(buildUrl(ENROLL_ENDPOINT), "{}", headers);
    updateBackendAvailability(response);

    Serial.print("[ApiClient] Enrollment HTTP status: ");
    Serial.println(response.statusCode);

    if (!response.success)
    {
        Serial.println("[ApiClient] Enrollment sin respuesta HTTP valida.");
        return EnrollmentResult::ConnectionError;
    }

    if (response.statusCode == 401 || response.statusCode == 403)
        return EnrollmentResult::Unauthorized;
    if (response.statusCode == 404)
        return EnrollmentResult::NotFound;
    if (response.statusCode == 409)
        return EnrollmentResult::AlreadyEnrolled;
    if (response.statusCode >= 500)
        return EnrollmentResult::ServerError;
    if (response.statusCode != 200 && response.statusCode != 201)
    {
        Serial.println("[ApiClient] Codigo HTTP inesperado durante enrollment.");
        return EnrollmentResult::ServerError;
    }

    JsonDocument responseDocument;
    DeserializationError error = deserializeJson(responseDocument, response.body);

    if (error)
    {
        Serial.print("[ApiClient] Error parseando respuesta de enrollment: ");
        Serial.println(error.c_str());
        return EnrollmentResult::ServerError;
    }

    const char* credential = responseDocument["deviceCredential"];

    if (!credential)
    {
        Serial.println("[ApiClient] La respuesta de enrollment no contiene deviceCredential.");
        return EnrollmentResult::ServerError;
    }

    const String deviceCredential = String(credential);

    if (!DeviceCredentialStorage::isValid(deviceCredential))
    {
        Serial.print("[ApiClient] deviceCredential invalida. Longitud: ");
        Serial.println(deviceCredential.length());
        return EnrollmentResult::ServerError;
    }

    if (!_deviceCredentialStorage.save(deviceCredential))
    {
        Serial.println("[ApiClient] No se pudo persistir deviceCredential.");
        return EnrollmentResult::ServerError;
    }

    Serial.println("[ApiClient] deviceCredential almacenada correctamente.");

    if (!_bootstrapCredentialStorage.clear())
    {
        Serial.println("[ApiClient] Advertencia: no se pudo eliminar bootstrap credential.");
        return EnrollmentResult::ServerError;
    }

    Serial.println("[ApiClient] Bootstrap credential eliminada correctamente.");
    return EnrollmentResult::Enrolled;
}

bool ApiClient::hasDeviceCredential() const
{
    String credential;
    return _deviceCredentialStorage.load(credential);
}

bool ApiClient::hasBootstrapCredential() const
{
    String credential;
    return _bootstrapCredentialStorage.load(credential);
}
