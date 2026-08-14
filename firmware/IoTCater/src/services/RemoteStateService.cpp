#include "services/RemoteStateService.h"

RemoteStateService::RemoteStateService(ApiClient& apiClient, FeedingService& feedingService,
                                       WiFiService& wifiService)
    : _apiClient(apiClient), _feedingService(feedingService), _wifiService(wifiService)
{
}

void RemoteStateService::update()
{
    if (!_wifiService.isConnected())
    {
        return;
    }

    if (millis() - _lastRequest < POLL_INTERVAL)
    {
        return;
    }

    _lastRequest = millis();

    pollMotorState();
}

void RemoteStateService::pollMotorState()
{
    bool motorState = false;
    int portions = 1;
    String commandId;

    if (!_apiClient.getMotorState(motorState, portions, commandId))
    {
        Serial.println("[RemoteStateService] Error al consultar motor state.");
        return;
    }

    if (motorState && !commandId.isEmpty() && commandId != _lastCommandId)
    {
        Serial.print("[RemoteStateService] Nueva orden recibida. ID: ");
        Serial.println(commandId);

        Serial.print("[RemoteStateService] Porciones: ");
        Serial.println(portions);

        if (_feedingService.feed(portions, FeedingSource::Remote))
        {
            _remoteFeedInProgress = true;
            _lastCommandId = commandId;
            _activeCommandId = commandId;
            _lastConfirmationAttempt = 0;
        }
    }
    if (_remoteFeedInProgress && !_feedingService.isFeeding())
    {
        if (millis() - _lastConfirmationAttempt >= CONFIRMATION_RETRY_INTERVAL)
        {
            _lastConfirmationAttempt = millis();

            Serial.println("[RemoteStateService] Confirmando alimentacion remota...");

            if (_apiClient.completeMotorCommand(_activeCommandId))
            {
                Serial.println("[RemoteStateService] Comando remoto confirmado.");

                _remoteFeedInProgress = false;
                _activeCommandId = "";
            }
            else
            {
                Serial.println("[RemoteStateService] Confirmacion fallida. Reintentando...");
            }
        }
    }
}