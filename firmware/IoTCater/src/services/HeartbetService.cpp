#include "services/HeartbetService.h"

HeartbeatService::HeartbeatService(ApiClient& apiClient, WiFiService& wifiService)
    : _apiClient(apiClient), _wifiService(wifiService)
{
}

void HeartbeatService::begin()
{
    _lastHeartbeat = millis();
}

void HeartbeatService::update()
{
    if (!_wifiService.isConnected())
    {
        return;
    }

    if (millis() - _lastHeartbeat < HEARTBEAT_INTERVAL)
    {
        return;
    }

    _lastHeartbeat = millis();

    if (!_apiClient.sendHeartbeat())
    {
        Serial.println("[HeartbeatService] Error al enviar heartbeat.");
    }
}
