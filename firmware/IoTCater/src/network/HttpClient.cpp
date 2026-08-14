#include "network/HttpClient.h"

HttpClient::HttpClient()
{
    _secureClient.setInsecure();
}

HttpResponse HttpClient::get(const String& url, const HttpHeaders& headers, uint16_t timeoutMs)
{
    return executeRequest(url, HttpMethod::Get, "", headers, timeoutMs);
}

HttpResponse HttpClient::post(const String& url, const String& body, HttpHeaders headers,
                              uint16_t timeoutMs)
{
    return executeRequest(url, HttpMethod::Post, body, headers, timeoutMs);
}
HttpResponse HttpClient::executeRequest(const String& url, HttpMethod method, const String& body,
                                        const HttpHeaders& headers, uint16_t timeoutMs)
{
    HttpResponse response;

    HTTPClient http;

    bool started = false;

    if (url.startsWith("https://"))
    {
        started = http.begin(_secureClient, url);
    }
    else
    {
        started = http.begin(_client, url);
    }

    if (!started)
    {
        return response;
    }

    http.setTimeout(timeoutMs);

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