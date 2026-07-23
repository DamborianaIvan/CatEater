#include "Motor.h"

Motor::Motor()
        :_stepper(
        AccelStepper::FULL4WIRE,
        PIN_IN1,
        PIN_IN3,
        PIN_IN2,
        PIN_IN4)
        
{
}
;
//Esta funcion es la encargada de hacer que el motor avance. La linea .run hace que avance.
void Motor::update()
{
    _stepper.run();
    if (_isFeeding && _stepper.distanceToGo() == 0)
    {
        _isFeeding = false;
        Serial.println("Alimentacion finalizada.");
    }
}
//Inicializaciones 
void Motor::begin()
{
    _stepper.setMaxSpeed(300);
    _stepper.setAcceleration(200);
    Serial.println("Motor inicializado");
}

//Es la encargada de la logica para el funcionamiento de el motor
bool Motor::feed(int portions)
{
    if (_isFeeding)
    {
        Serial.println("El motor ya esta alimentando.");
        return false;
    }
    if (portions <= 0)
    {
        Serial.println("Cantidad de porciones invalida.");
        return false;
    }
    //el static_cast es para decir que el valor no va a cambiar yt explicita la conversion a long
    const long steps = static_cast<long>(_stepsPerFeed) * portions;
    _stepper.move(steps);
    _isFeeding = true;
    return true;
}
bool Motor::setStepsPerFeed(int steps)
{
    if (steps <= 0)
    {
        return false;
    }

    _stepsPerFeed = steps;
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
