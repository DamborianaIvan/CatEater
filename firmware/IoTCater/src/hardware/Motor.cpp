#include "hardware/Motor.h"
#include "domain/Configuration.h"

Motor::Motor()
    : _stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR)
{
}

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
    // A4988 ENABLE is active LOW.
    _stepper.setEnablePin(PIN_ENABLE);
    _stepper.setPinsInverted(false, false, true);
    _stepper.enableOutputs();

    // Conservative initial values for the A4988 + NEMA 17 setup.
    // These can be tuned after the first hardware test.
    _stepper.setMaxSpeed(300);
    _stepper.setAcceleration(100);

    Serial.println("[Motor] Inicializado (A4988)");
}

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

    const long stepsPerFeed = static_cast<long>(_stepsPerFeed) * portions;

    // El sinfin esta montado con sentido de avance inverso.
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
