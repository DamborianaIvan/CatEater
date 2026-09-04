#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H
#include <Arduino.h>
#include <ESP8266WiFi.h>

enum class ConnectionState
{
    Disconnected,
    Connecting,
    Connected
};

class WiFiService
{
   public:
    WiFiService();

    void begin(const String& ssid, const String& password, bool keepAccessPoint = false);
    void disconnect(bool eraseStoredCredentials = false);
    void update();

    bool isConnected() const;
    String getIpAddress() const;
    String getMacAddress() const;
    ConnectionState getConnectionState() const;
    unsigned int getConsecutiveConnectionAttempts() const;

    int getRssi() const;

   private:
    ConnectionState _state = ConnectionState::Disconnected;

    String _ssid;
    String _password;

    unsigned long _lastReconnectAttempt = 0;
    unsigned int _consecutiveConnectionAttempts = 0;

    static constexpr unsigned long RECONNECT_INTERVAL = 5000;

    void attemptConnection();
    void printConnectionInfo();
};

#endif
