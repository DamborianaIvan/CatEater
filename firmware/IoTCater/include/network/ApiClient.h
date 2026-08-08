#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"

class ApiClient
{
   public:
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo);

    String buildRegistrationBody() const;
    bool registerDevice();

   private:
    HttpClient& _httpClient;
    const DeviceInfo& _deviceInfo;
    static constexpr char CONTENT_TYPE[] = "application/json";
    static constexpr char API_KEY[] = "ExCECoLysoco";
    static constexpr char BASE_URL[] = "http://192.168.1.34:5000";

    static constexpr char REGISTER_ENDPOINT[] = "/feeders/register";

    String buildUrl(const String& endpoint) const;
};