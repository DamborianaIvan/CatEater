#include "hardware/Motor.h"
#include "domain/Configuration.h"

Motor::Motor()
    : _stepper(AccelStepper::FULL4WIRE, PIN_IN1, PIN_IN3, PIN_IN2, PIN_IN4)

{};
// Esta funcion es la encargada de hacer que el motor avance. La linea .run hace que avance.
void Motor::update()
{
    _stepper.run();
    if (_isFeeding && _stepper.distanceToGo() == 0)
    {
        _isFeeding = false;
        Serial.println("[Motor] Alimentacion finalizada.");
    }
}

void Motor::begin()
{
    _stepper.setMaxSpeed(600);
    _stepper.setAcceleration(100);
    Serial.println("[Motor] Inicializado");
}

// logica para el funcionamiento de el motor
bool Motor::feed(int portions)
{
    Serial.printf("[Motor] feed(%d)\n", portions);
    Serial.printf("[Motor] stepsPerFeed = %d\n", _stepsPerFeed);
    if (_isFeeding)
    {
        Serial.println("[Motor] El motor ya esta alimentando.");
        return false;
    }
    if (!Configuration::isValidPortions(portions))
    {
        Serial.println("[Motor] Cantidad de porciones invalida.");
        return false;
    }
    // el static_cast es para decir que el valor no va a cambiar yt explicita la conversion a long
    // El sinfin esta montado con sentido de avance inverso.
    const long stepsPerFeed = static_cast<long>(_stepsPerFeed) * portions;
    _stepper.move(-stepsPerFeed);
    _isFeeding = true;
    return true;
}
bool Motor::setStepsPerFeed(int stepsPerFeed)
{
    if (!Configuration::isValidStepsPerFeed(stepsPerFeed))
    {
        return false;
    }

    _stepsPerFeed = stepsPerFeed;
    return true;
}
int Motor::getStepsPerFeed() const
{
    return _stepsPerFeed;
}
bool Motor::isFeeding() const
{
    return _isFeeding;
}
