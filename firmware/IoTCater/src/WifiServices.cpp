#include "Config.h"
#include "WifiServices.h"
#include <ESP8266WiFi.h>

WiFiService::WiFiService()
{
}

void WiFiService::begin(const char* ssid, const char* password)
{
    _ssid = ssid;
    _password = password;

    WiFi.mode(WIFI_STA);

    attemptConnection();
}

void WiFiService::attemptConnection()
{
    if (_ssid == nullptr || _password == nullptr)
    {
        return;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }
    Serial.print("Conectando a ");
    Serial.println(_ssid);
    WiFi.begin(_ssid, _password);
    _lastReconnectAttempt = millis();
}

void WiFiService::update()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (_state != ConnectionState::Connected)
        {
            _state = ConnectionState::Connected;
            printConnectionInfo();
        }
        return;
    }

    if (_state == ConnectionState::Connected)
    {
        Serial.println("Conexion WiFi perdida.");
        _state = ConnectionState::Disconnected;
    }
    if (millis() - _lastReconnectAttempt >= RECONNECT_INTERVAL)
    {
        attemptConnection();
    }
}

void WiFiService::printConnectionInfo()
{
    Serial.println();
    Serial.println("WiFi conectado.");
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.println();
}