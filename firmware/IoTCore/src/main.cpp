#include <Arduino.h>

#include "app/IoTCoreApplication.h"
#include "infrastructure/ArduinoLogger.h"
#include "infrastructure/FeederWebServer.h"
#include "infrastructure/ULN2003FeederActuator.h"
#include "infrastructure/LittleFsConfigStorage.h"

namespace {
ArduinoLogger logger;
ULN2003FeederActuator feederActuator(D1, D2, D6, D7);
IoTCoreApplication application(logger, feederActuator);
}  // namespace
LittleFsConfigStorage configStorage;

FeederWebServer webServer(application, feederActuator, configStorage);
void setup() {
  Serial.begin(SERIAL_BAUD);
  if (configStorage.begin()) {
    DeviceConfig config;
    const bool hasValidConfig = configStorage.load(config) &&
                                config.stepsPerPortion >= 16 &&
                                config.stepsPerPortion <= 4096;
    if (!hasValidConfig) {
      config = DeviceConfig{};
      configStorage.save(config);
    }
    feederActuator.setStepsPerPortion(config.stepsPerPortion);
    logger.info("Configuracion cargada desde LittleFS.");
  } else {
    logger.error("No se pudo montar LittleFS.");
  }
  application.begin();
  webServer.begin();
}

void loop() {
  application.run();
  feederActuator.update();
  webServer.handleClient();
}
