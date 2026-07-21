#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"

Motor motor;
WiFiService wifi;
WebServer webServer(motor);
void setup()
{
    Serial.begin(115200);
    motor.begin();
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    webServer.begin();

    motor.feed(2);
}

void loop()
{
    motor.update();
    wifi.update();
    webServer.update();
}