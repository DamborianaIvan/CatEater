#include <Arduino.h>
#include "Motor.h"

Motor motor;

void setup()
{
    Serial.begin(115200);

    motor.begin();

    Serial.println("CatFeeder iniciado");
}

void loop()
{
}