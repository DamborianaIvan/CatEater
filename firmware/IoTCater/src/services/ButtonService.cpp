#include "services/ButtonService.h"

namespace
{
volatile bool* interruptPressedFlag = nullptr;

void IRAM_ATTR handleButtonInterrupt()
{
    if (interruptPressedFlag != nullptr)
        *interruptPressedFlag = true;
}
}

ButtonService::ButtonService(FeedingService& feedingService, uint8_t pin)
    : _feedingService(feedingService), _pin(pin)
{
}

void ButtonService::begin()
{
    pinMode(_pin, INPUT_PULLUP);

    _interruptPressed = false;
    _armed = digitalRead(_pin) == HIGH;

    interruptPressedFlag = &_interruptPressed;
    attachInterrupt(digitalPinToInterrupt(_pin), handleButtonInterrupt, FALLING);

    Serial.println("[ButtonService] Iniciado.");
}

void ButtonService::update()
{
    if (!_interruptPressed)
        return;

    noInterrupts();
    _interruptPressed = false;
    interrupts();

    if (!_armed)
        return;

    _armed = false;

    Serial.println("[ButtonService] Alimentacion manual solicitada.");

    if (_feedingService.feed(DEFAULT_PORTIONS, FeedingSource::Physical))
    {
        Serial.println("[ButtonService] Alimentacion iniciada.");
    }
    else
    {
        Serial.println("[ButtonService] No se pudo iniciar la alimentacion.");
    }

    // La siguiente pulsacion solo se acepta cuando el boton haya sido liberado.
    if (digitalRead(_pin) == HIGH)
        _armed = true;
}
