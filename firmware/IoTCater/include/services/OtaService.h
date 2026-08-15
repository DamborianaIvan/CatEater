#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <Updater.h>
#include "hardware/Motor.h"

class OtaService
{
   public:
    explicit OtaService(Motor& motor);

    void handleUpdate(ESP8266WebServer& server);
    void handlePage(ESP8266WebServer& server);

   private:
    Motor& _motor;
    bool _updateFailed = false;
};

#endif