#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include <AccelStepper.h>

class Motor
{
public:
    Motor();

    void begin();
    void update();
    
    bool setStepsPerFeed(int steps);
    bool isFeeding() const;
    bool feed(int portions=1);
    bool getStepsPerFeed() const;

    
private:
    bool _isFeeding = false;
    int _stepsPerFeed;
    static const long STEPS_PER_PORTION = 2048;
    static const uint8_t PIN_IN1 = D1;
    static const uint8_t PIN_IN2 = D3;
    static const uint8_t PIN_IN3 = D6;
    static const uint8_t PIN_IN4 = D7;
    AccelStepper _stepper;
};

#endif