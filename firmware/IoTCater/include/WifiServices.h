#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H
#include <Arduino.h>

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

    void begin(const char* ssid, const char* password);
    void update();

    bool isConnected() const;
    String getIpAddress() const;

private:
    ConnectionState _state = ConnectionState::Disconnected;

    const char* _ssid = nullptr;
    const char* _password = nullptr;

    unsigned long _lastReconnectAttempt = 0;

    static constexpr unsigned long RECONNECT_INTERVAL = 5000;

    void attemptConnection();
    void printConnectionInfo();
};

#endif