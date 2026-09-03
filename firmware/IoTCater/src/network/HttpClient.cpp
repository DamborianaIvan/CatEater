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
    const unsigned long startMs = millis();

    Serial.printf("[HttpClient] %s %s | timeout=%u ms\n",
                  method == HttpMethod::Get ? "GET" : "POST",
                  url.c_str(),
                  timeoutMs);

    bool started = false;

    if (url.startsWith("https://"))
    {
        Serial.println("[HttpClient] Transporte: HTTPS");
        started = http.begin(_secureClient, url);
    }
    else
    {
        Serial.println("[HttpClient] Transporte: HTTP");
        started = http.begin(_client, url);
    }

    if (!started)
    {
        Serial.printf("[HttpClient] ERROR: no se pudo iniciar la conexión | elapsed=%lu ms\n",
                      millis() - startMs);
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
            Serial.printf("[HttpClient] ERROR: método HTTP no soportado | elapsed=%lu ms\n",
                          millis() - startMs);
            return response;
    }

    const unsigned long elapsedMs = millis() - startMs;

    // ESP8266HTTPClient usa valores negativos para errores de transporte.
    // Un valor positivo significa que se recibió una respuesta HTTP, incluso 4xx/5xx.
    response.transportSuccess = response.statusCode > 0;

    Serial.printf("[HttpClient] Resultado | status=%d | transport=%s | elapsed=%lu ms\n",
                  response.statusCode,
                  response.transportSuccess ? "OK" : "FAIL",
                  elapsedMs);

    if (response.transportSuccess)
    {
        response.body = http.getString();

        if (response.statusCode < 200 || response.statusCode >= 300)
        {
            Serial.printf("[HttpClient] HTTP error body: %s\n", response.body.c_str());
        }
    }

    http.end();

    return response;
}
