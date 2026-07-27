#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"
#include "TimeService.h"
#include "Scheduler.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
Scheduler scheduler(timeService, motor);
ConfigurationStorage storage;
Configuration configuration;
WebServer webServer(
    motor,
    wifi,
    scheduler,
    configuration,
    storage); 
void setup()
{
    Serial.begin(115200);
    motor.begin();
    storage.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);
    
   
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    timeService.begin();
    scheduler.begin();
    scheduler.setSchedule(
        13,
        40
    );
    webServer.begin();
}

void loop()
{
    wifi.update();
    timeService.update();
    scheduler.update();
    motor.update();
    webServer.update();
}