#pragma once

#include "application/FeedController.h"

class IoTCoreApplication {
 public:
  IoTCoreApplication(ILogger& logger, IFeederActuator& feederActuator);
  void begin();
  void run();
  bool feed(uint16_t portionCount);

 private:
  ILogger& logger_;
  FeedController feedController_;
};
