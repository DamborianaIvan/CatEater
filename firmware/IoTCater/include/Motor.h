#ifndef MOTOR_H
#define MOTOR_H

class Motor
{
public:
    Motor();

    void begin();

    void feed();

private:
    int _stepsPerPortion;
};

#endif