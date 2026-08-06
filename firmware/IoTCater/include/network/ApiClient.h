#pragma once

#include <Arduino.h>

#include "device/DeviceInfo.h"
#include "network/HttpClient.h"

class ApiClient
{
   public:
    ApiClient(HttpClient& httpClient, const DeviceInfo& deviceInfo);

   private:
    HttpClient& _httpClient;
    const DeviceInfo& _deviceInfo;

    static constexpr char BASE_URL[] = "http://TU_BACKEND";

    static constexpr char REGISTER_ENDPOINT[] = "/feeders/register";

    String buildUrl(const String& endpoint) const;
};