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
    static constexpr size_t MAX_FIRMWARE_SIZE = 1024UL * 1024UL;

    Motor& _motor;
    bool _updateInProgress = false;
    bool _updateFailed = false;
    size_t _expectedSize = 0;
    size_t _receivedSize = 0;

    bool isAuthorized(ESP8266WebServer& server) const;
    bool beginUpdate(HTTPUpload& upload);
    bool writeUpdateChunk(HTTPUpload& upload);
    bool finishUpdate(ESP8266WebServer& server, HTTPUpload& upload);
    void abortUpdate();
};

#endif