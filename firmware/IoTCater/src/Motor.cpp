#include "Motor.h"

Motor::Motor()
    : _stepper(
        AccelStepper::FULL4WIRE,
        PIN_IN1,
        PIN_IN3,
        PIN_IN2,
        PIN_IN4)
{
}
;

//Inicializaciones 
void Motor::begin()
{
    _stepper.setMaxSpeed(500);
    _stepper.setAcceleration(300);
    Serial.println("Motor inicializado");
}

//Es la encargada de la logica para el funcionamiento de el motor
void Motor::feed(int portions)
{
    if (_isFeeding)
    {
        Serial.println("El motor ya esta alimentando.");
        return;
    }
    if (portions <= 0)
    {
        Serial.println("Cantidad de porciones invalida.");
        return;
    }
    long steps = STEPS_PER_PORTION * portions;
    _stepper.move(steps);
    _isFeeding = true;
}

//Esta funcion es la encargada de hacer que el motor avance. La lunea .run hace que avance.
void Motor::update()
{
    _stepper.run();
    if (_isFeeding && _stepper.distanceToGo() == 0)
    {
        _isFeeding = false;
        Serial.println("Alimentacion finalizada.");
    }
}