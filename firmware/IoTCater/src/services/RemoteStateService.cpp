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

    if (!_apiClient.getMotorState(motorState))
    {
        Serial.println("[RemoteStateService] Error al consultar motor state.");
        return;
    }

    Serial.print("[RemoteStateService] Motor state: ");
    Serial.println(motorState ? "ON" : "OFF");

    if (motorState && !_previousMotorState)
    {
        Serial.println("[RemoteStateService] Orden de alimentacion recibida.");

        if (!_motor.feed())
        {
            Serial.println("[RemoteStateService] No se pudo ejecutar la alimentacion.");
        }
    }

    _previousMotorState = motorState;
}