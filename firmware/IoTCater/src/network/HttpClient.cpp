#include "network/HttpClient.h"

HttpClient::HttpClient()
{
    _secureClient.setInsecure();
}

HttpResponse HttpClient::get(const String& url, const HttpHeaders& headers)
{
    return executeRequest(url, HttpMethod::Get, "", headers);
}

HttpResponse HttpClient::post(const String& url, const String& body, HttpHeaders headers)
{
    return executeRequest(url, HttpMethod::Post, body, headers);
}
HttpResponse HttpClient::executeRequest(const String& url, HttpMethod method, const String& body,
                                        const HttpHeaders& headers)
{
    HttpResponse response;

    HTTPClient http;

    bool started = false;

    Serial.print("[HttpClient] -> ");
    Serial.println(url);
    if (url.startsWith("https://"))
    {
        Serial.println("[HttpClient] Protocol: HTTPS");
        started = http.begin(_secureClient, url);
    }
    else
    {
        Serial.println("[HttpClient] Protocol: HTTP");
        started = http.begin(_client, url);
    }

    if (!started)
    {
        return response;
    }

    http.setTimeout(DEFAULT_TIMEOUT_MS);

    for (const auto& header : headers)
    {
        http.addHeader(header.first, header.second);
    }

    switch (method)
    {
        case HttpMethod::Get:
            response.statusCode = http.GET();
            break;

        case HttpMethod::Post:
            response.statusCode = http.POST(body);
            Serial.print("HTTP Error: ");
            Serial.println(http.errorToString(response.statusCode));
            break;

        default:
            http.end();
            return response;
    }

    if (response.statusCode > 0)
    {
        response.success = true;
        response.body = http.getString();
    }

    http.end();

    return response;
}