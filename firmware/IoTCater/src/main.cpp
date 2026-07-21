#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
Motor motor;
WiFiService wifi;

void setup()
{
    Serial.begin(115200);
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    motor.begin();
    motor.feed(2);
}

void loop()
{
    motor.update();
    wifi.update();
}