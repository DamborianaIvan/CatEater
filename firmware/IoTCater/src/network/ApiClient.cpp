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