#include "HeartbetServices.h"

HeartbeatService::HeartbeatService(ApiClient& apiClient) : _apiClient(apiClient) {}

void HeartbeatService::begin()
{
    _lastHeartbeat = millis();

    Serial.println("[HeartbeatService] Iniciado.");
}

void HeartbeatService::update()
{
    if (millis() - _lastHeartbeat < HEARTBEAT_INTERVAL)
    {
        return;
    }

    _lastHeartbeat = millis();

    if (_apiClient.sendHeartbeat())
    {
        Serial.println("[HeartbeatService] Heartbeat enviado.");
    }
    else
    {
        Serial.println("[HeartbeatService] Error al enviar heartbeat.");
    }
}