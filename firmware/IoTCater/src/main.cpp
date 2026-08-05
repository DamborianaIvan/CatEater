#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"
#include "TimeService.h"
#include "Scheduler.h"
#include "Configuration.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
Configuration configuration;
Scheduler scheduler(timeService, motor, configuration);
ConfigurationStorage storage;
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
        36
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