#include "services/RemoteStateService.h"

RemoteStateService::RemoteStateService(ApiClient& apiClient, Motor& motor)
    : _apiClient(apiClient), _motor(motor)
{
}

void RemoteStateService::update()
{
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

    if (!_apiClient.getMotorState(motorState, portions))
    {
        Serial.println("[RemoteStateService] Error al consultar motor state.");
        return;
    }

    Serial.print("[RemoteStateService] Motor state: ");
    Serial.println(motorState ? "ON" : "OFF");

    if (motorState && !_previousMotorState)
    {
        Serial.print("[RemoteStateService] Orden de alimentacion recibida. Porciones: ");
        Serial.println(portions);

        if (_motor.feed(portions))
        {
            _remoteFeedInProgress = true;
        }
        else
        {
            Serial.println("[RemoteStateService] No se pudo ejecutar la alimentacion.");
        }
    }
    if (_remoteFeedInProgress && !_motor.isFeeding())
    {
        Serial.println("[RemoteStateService] Alimentacion remota finalizada.");

        if (_apiClient.completeMotorCommand())
        {
            Serial.println("[RemoteStateService] Comando remoto confirmado.");

            _remoteFeedInProgress = false;
        }
        else
        {
            Serial.println("[RemoteStateService] Error al confirmar comando remoto.");
        }
    }
    _previousMotorState = motorState;
}