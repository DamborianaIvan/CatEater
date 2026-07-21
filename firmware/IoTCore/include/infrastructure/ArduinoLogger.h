#pragma once

#include "ports/ILogger.h"

class ArduinoLogger final : public ILogger {
 public:
  void info(const char* message) override;
  void error(const char* message) override;
};
