#include "services/ButtonService.h"

ButtonService::ButtonService(FeedingService& feedingService, uint8_t pin)
    : _feedingService(feedingService), _pin(pin)
{
}

void ButtonService::begin()
{
    pinMode(_pin, INPUT_PULLUP);

    _lastState = digitalRead(_pin);
    _lastDebouncedState = _lastState;
    _lastStateChangeAt = millis();

    Serial.println("[ButtonService] Iniciado.");
}

void ButtonService::update()
{
    const bool currentState = digitalRead(_pin);
    const unsigned long now = millis();

    if (currentState != _lastState)
    {
        _lastState = currentState;
        _lastStateChangeAt = now;
        return;
    }

    if ((now - _lastStateChangeAt) < DEBOUNCE_MS)
    {
        return;
    }

    if (currentState == _lastDebouncedState)
    {
        return;
    }

    _lastDebouncedState = currentState;

    if (currentState == LOW)
    {
        Serial.println("[ButtonService] Alimentacion manual solicitada.");

        if (_feedingService.feed(DEFAULT_PORTIONS, FeedingSource::Physical))
        {
            Serial.println("[ButtonService] Alimentacion iniciada.");
        }
        else
        {
            Serial.println("[ButtonService] No se pudo iniciar la alimentacion.");
        }
    }
}
