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
#include "network/ApiClient.h"

Motor motor;
WiFiService wifi;
TimeService timeService(wifi);
HttpClient httpClient;
Configuration configuration;
Scheduler scheduler(timeService, motor, configuration);
ConfigurationStorage storage;
DeviceInfo deviceInfo(wifi);
ApiClient apiClient(httpClient, deviceInfo);
WebServer webServer(motor, wifi, scheduler, configuration, storage);
void setup()
{
    Serial.begin(115200);
    motor.begin();
    storage.begin();
    timeService.begin();

    configuration = storage.loadConfiguration(Configuration{});
    motor.setStepsPerFeed(configuration.stepsPerFeed);

    wifi.begin(WIFI_SSID, WIFI_PASSWORD);

    scheduler.begin();
    webServer.begin();
}

void loop()
{
    static bool bootInfoPrinted = false;
    wifi.update();
    if (wifi.isConnected() && !bootInfoPrinted)
    {
        bootInfoPrinted = true;
        deviceInfo.printBootInfo();
        apiClient.registerDevice();
    }
    timeService.update();
    scheduler.update();
    motor.update();
    webServer.update();
}