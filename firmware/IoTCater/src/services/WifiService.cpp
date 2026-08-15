#include "services/WifiService.h"

WiFiService::WiFiService() {}

void WiFiService::begin(const String& ssid, const String& password, bool keepAccessPoint)
{
    _ssid = ssid;
    _password = password;

    WiFi.persistent(false);
    WiFi.mode(keepAccessPoint ? WIFI_AP_STA : WIFI_STA);

    attemptConnection();
}

void WiFiService::disconnect(bool eraseStoredCredentials)
{
    _ssid = "";
    _password = "";
    _state = ConnectionState::Disconnected;
    if (eraseStoredCredentials)
    {
        WiFi.persistent(true);
    }
    WiFi.disconnect(eraseStoredCredentials);
    WiFi.persistent(false);
}

void WiFiService::attemptConnection()
{
    if (_ssid.isEmpty())
    {
        return;
    }
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }
    Serial.print("[WifiService] Conectando a ");
    Serial.print(_ssid);
    Serial.println("...");
    WiFi.begin(_ssid.c_str(), _password.c_str());
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
        Serial.println("[WifiService] Conexion WiFi perdida.");
        _state = ConnectionState::Disconnected;
    }
    if (millis() - _lastReconnectAttempt >= RECONNECT_INTERVAL)
    {
        attemptConnection();
    }
}

String WiFiService::getIpAddress() const
{
    return WiFi.localIP().toString();
}

bool WiFiService::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiService::getMacAddress() const
{
    return WiFi.macAddress();
}

int WiFiService::getRssi() const
{
    if (!isConnected())
    {
        return 0;
    }

    return WiFi.RSSI();
}

void WiFiService::printConnectionInfo()
{
    Serial.println();
    Serial.println("[WifiService] WiFi conectado.");
    Serial.print("[WifiService] SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("[WifiService] IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("[WifiService] RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    Serial.println();
}

ConnectionState WiFiService::getConnectionState() const
{
    return _state;
}
