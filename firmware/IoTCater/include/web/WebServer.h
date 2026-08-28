#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "hardware/Motor.h"
#include <ESP8266WebServer.h>
#include "services/WifiService.h"
#include "services/FeedingService.h"
#include "services/OtaService.h"
#include "services/BackendConnectionService.h"

class WebServer
{
   public:
    explicit WebServer(Motor& motor, WiFiService& wifi, FeedingService& feedingService,
                       OtaService& otaService,
                       const BackendConnectionService& backendConnectionService);
    void begin();
    void stop();
    void update();

   private:
    Motor& _motor;
    WiFiService& _wifi;
    FeedingService& _feedingService;
    ESP8266WebServer _server{80};
    OtaService& _otaService;
    const BackendConnectionService& _backendConnectionService;
    bool _started = false;
    void registerRoutes();
    void handleHome();
    void handleFeed();
    void handleStatus();
    void handleNotFound();
};

#endif
