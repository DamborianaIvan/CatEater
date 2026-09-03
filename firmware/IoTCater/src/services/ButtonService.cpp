#include "services/ButtonService.h"

ButtonService::ButtonService(FeedingService& feedingService, uint8_t pin)
    : _feedingService(feedingService), _pin(pin)
{
}

void ButtonService::begin()
{
    pinMode(_pin, INPUT_PULLUP);

    _lastState = digitalRead(_pin);

    if (_lastState == LOW)
        _pressedAt = millis();

    Serial.println("[ButtonService] Iniciado.");
}

void ButtonService::update()
{
    const bool currentState = digitalRead(_pin);

    if (_lastState == HIGH && currentState == LOW)
        _pressedAt = millis();

    if (_lastState == LOW && currentState == HIGH)
    {
        Serial.println("[ButtonService] Alimentacion manual solicitada.");

        if (_feedingService.feed(DEFAULT_PORTIONS, FeedingSource::Physical))
            Serial.println("[ButtonService] Alimentacion iniciada.");
        else
            Serial.println("[ButtonService] No se pudo iniciar la alimentacion.");
    }

    _lastState = currentState;
}