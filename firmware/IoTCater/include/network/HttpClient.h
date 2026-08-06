#pragma once

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include "network/HttpHeaders.h"
#include "network/HttpMethod.h"
#include "network/HttpResponse.h"

class HttpClient
{
   public:
    HttpClient();

    HttpResponse get(const String& url, const HttpHeaders& headers = {});
    HttpResponse post(const String& url, const String& body, HttpHeaders headers = {});

   private:
    static constexpr uint16_t DEFAULT_TIMEOUT_MS = 5000;
    static constexpr char CONTENT_TYPE_JSON[] = "application/json";
    BearSSL::WiFiClientSecure _client;

    HttpResponse executeRequest(const String& url, HttpMethod method, const String& body,
                                const HttpHeaders& headers);
};