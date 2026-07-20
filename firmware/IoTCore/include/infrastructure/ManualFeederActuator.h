#pragma once

#include "ports/IFeederActuator.h"

class ManualFeederActuator final : public IFeederActuator {
 public:
  bool dispense(uint16_t portionGrams) override;
  void update() override;
};
