#pragma once

#include <ESP8266WebServer.h>

#include "app/IoTCoreApplication.h"
#include "infrastructure/ULN2003FeederActuator.h"
#include "ports/IConfigStorage.h"

class FeederWebServer {
 public:
  FeederWebServer(IoTCoreApplication& application, ULN2003FeederActuator& motor,
                  IConfigStorage& configStorage);
  void begin();
  void handleClient();

 private:
  void showHome();
  void serveFood();

  void showCalibration();
  void saveCalibration();
  ESP8266WebServer server_;
  IoTCoreApplication& application_;
  ULN2003FeederActuator& motor_;
  IConfigStorage& configStorage_;
};
