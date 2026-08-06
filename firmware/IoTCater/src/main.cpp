#include <Arduino.h>
#include "Motor.h"
#include "Config.h"
#include "WifiServices.h"
#include "WebServer.h"
#include "ConfigurationStorage.h"
#include "TimeService.h"
#include "Scheduler.h"
#include "Configuration.h"
#include "device/DeviceInfo.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
Configuration configuration;
Scheduler scheduler(timeService, motor, configuration);
ConfigurationStorage storage;
DeviceInfo deviceInfo(wifi);
WebServer webServer(
    motor,
    wifi,
    scheduler,
    configuration,
    storage); 
void setup()
{
    delay(4000);
    Serial.begin(115200);
    motor.begin();
    storage.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);
    
   
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    delay(1000);
    deviceInfo.printBootInfo();
   
    timeService.begin();
    scheduler.begin();
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