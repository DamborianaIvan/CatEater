#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "Motor.h"
#include <ESP8266WebServer.h>
#include "WifiServices.h"
#include "ConfigurationStorage.h"


class WebServer
{
public:
    explicit WebServer(Motor& motor,
          WiFiService& wifi,
        ConfigurationStorage& storage);

    void begin();
    void update();

private:
    Motor& _motor;
    WiFiService& _wifi;
    ConfigurationStorage& _storage;
    ESP8266WebServer _server{80};

    void registerRoutes();
    void handleHome();
    void handleFeed();
    void handleStatus();
    void handleNotFound();
    void handleUpdateConfig();
    void handleConfig();
};

#endif