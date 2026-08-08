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

bool ApiClient::registerDevice()
{
    String body = buildRegistrationBody();

    HttpHeaders headers;

    headers.emplace_back("x-api-key", API_KEY);
    headers.emplace_back("Content-Type", CONTENT_TYPE);
    HttpResponse response = _httpClient.post(buildUrl(REGISTER_ENDPOINT), body, headers);

    Serial.println("========== Register Device ==========");
    Serial.print("Status: ");
    Serial.println(response.statusCode);

    Serial.println();

    Serial.println("Response:");
    Serial.println(response.body);

    Serial.println("=====================================");

    return true;
}