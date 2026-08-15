#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include "hardware/Motor.h"
#include <ESP8266WebServer.h>
#include "services/WifiService.h"
#include "services/FeedingService.h"
#include "storage/ConfigurationStorage.h"
#include "services/Scheduler.h"

class WebServer
{
   public:
    explicit WebServer(Motor& motor, WiFiService& wifi, FeedingService& feedingService,
                       Scheduler& scheduler,
                       Configuration& configuration, ConfigurationStorage& storage);

    void begin();
    void stop();
    void update();

   private:
    Motor& _motor;
    WiFiService& _wifi;
    FeedingService& _feedingService;
    ConfigurationStorage& _storage;
    ESP8266WebServer _server{80};
    Configuration& _configuration;
    Scheduler& _scheduler;
    bool _started = false;
    bool isValidSchedule(int hour, int minute, int portions) const;
    void registerRoutes();
    void handleHome();
    void handleFeed();
    void handleStatus();
    void handleNotFound();
    void handleUpdateConfig();
    void handleGetConfiguration();
};

#endif
