#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "Motor.h"
#include <ESP8266WebServer.h>


class WebServer
{
public:
    explicit WebServer(Motor& motor);

    void begin();
    void update();

private:
    Motor& _motor;

    ESP8266WebServer _server{80};

    void registerRoutes();
    void handleHome();
    void handleFeed();
    void handleNotFound();
};

#endif