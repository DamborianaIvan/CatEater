#include "network/ApiClient.h"

ApiClient::ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo)
    : _httpClient(httpClient), _deviceInfo(deviceInfo)
{
}

String ApiClient::buildUrl(const String& endpoint) const
{
    return String(BASE_URL) + endpoint;
}