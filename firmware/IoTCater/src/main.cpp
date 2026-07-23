#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"

Motor motor;
WiFiService wifi;
ConfigurationStorage storage;
WebServer webServer(motor, wifi, storage); 
void setup()
{
    Serial.begin(115200);

    motor.begin();

    storage.begin();

    int steps = storage.loadStepsPerFeed(1024);
    motor.setStepsPerFeed(steps);

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    webServer.begin();
}

void loop()
{
    motor.update();
    wifi.update();
    webServer.update();
}