#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"
#include "TimeService.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
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
    timeService.begin();
    webServer.begin();
}

void loop()
{
    wifi.update();
    timeService.update();
    motor.update();
    webServer.update();
    if(timeService.isTimeAvailable())
    {
        Serial.printf(
            "%02d:%02d:%02d\n",
            timeService.getHour(),
            timeService.getMinute(),
            timeService.getSecond()
        );

        delay(1000);
    }
}