#pragma once

#include <stdint.h>

class IFeederActuator {
 public:
  virtual ~IFeederActuator() = default;
  virtual bool dispense(uint16_t portionCount) = 0;
  virtual void update() = 0;
};
