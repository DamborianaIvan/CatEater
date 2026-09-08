#include "services/HeartbetService.h"

HeartbeatService::HeartbeatService(ApiClient& apiClient, WiFiService& wifiService,
                                   DiagnosticService& diagnostics)
    : _apiClient(apiClient), _wifiService(wifiService), _diagnostics(diagnostics)
{
}

void HeartbeatService::begin()
{
    // Permite enviar el primer heartbeat apenas haya WiFi disponible.
    _lastHeartbeat = millis() - HEARTBEAT_INTERVAL;
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
        _diagnostics.record("HEARTBEAT_ERROR");
    }
}
