#include "app/IoTCoreApplication.h"

IoTCoreApplication::IoTCoreApplication(ILogger& logger,
                                       IFeederActuator& feederActuator)
    : logger_(logger), feedController_(logger, feederActuator) {}

void IoTCoreApplication::begin() {
  logger_.info("IoT Core iniciado.");
}

void IoTCoreApplication::run() {
  // Los adaptadores de entrada se conectan aqui.

}
bool IoTCoreApplication::feed(uint16_t portionCount) {
  return feedController_.serve({portionCount});
}
