#include <Arduino.h>
#include "Motor.h"

Motor motor;

void setup()
{
    Serial.begin(115200);
    motor.begin();
    delay(2000);
    motor.feed();
}

void loop()
{
    motor.update();
}