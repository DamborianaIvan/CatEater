#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>

class WebServer
{
public:
    WebServer();

    void begin();

    void update();

private:
    ESP8266WebServer _server{80};

    void registerRoutes();
    void handleHome();
    void handleNotFound();
};

#endif