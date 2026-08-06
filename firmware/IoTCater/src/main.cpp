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
#include "network/HttpClient.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
HttpClient httpClient;
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
    Serial.begin(115200);
    motor.begin();
    storage.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);
    
   
    wifi.begin(WIFI_SSID, WIFI_PASSWORD);

    timeService.begin();
    scheduler.begin();
    webServer.begin();
}

void loop()
{
    static bool tested = false;
    wifi.update();
    if (wifi.isConnected() && !tested)
    {
        tested = true;
        deviceInfo.printBootInfo();

        HttpResponse response = httpClient.get("https://httpbin.org/get");
        Serial.println("--------------PruebaHTTP--------------"); 
        Serial.println(response.success); 
        Serial.println(response.statusCode); 
        Serial.println(response.body);
    }
    timeService.update();
    scheduler.update();
    motor.update();
    webServer.update();
}