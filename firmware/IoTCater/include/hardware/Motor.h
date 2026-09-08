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

    bool setStepsPerFeed(int stepsPerFeed);
    bool isFeeding() const;
    bool feed(int portions = 1);

    int getStepsPerFeed() const;

   private:
    bool _isFeeding = false;
    int _stepsPerFeed;

    // A4988: STEP + DIR + ENABLE.
    // ENABLE is active LOW and is inverted in Motor::begin().
    static const uint8_t PIN_STEP = D6;
    static const uint8_t PIN_DIR = D5;
    static const uint8_t PIN_ENABLE = D7;

    AccelStepper _stepper;
};

#endif
