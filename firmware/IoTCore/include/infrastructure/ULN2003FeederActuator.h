#pragma once

#include <Arduino.h>
#include <AccelStepper.h>

#include "ports/IFeederActuator.h"

class ULN2003FeederActuator final : public IFeederActuator {
 public:
  static constexpr uint16_t kDefaultStepsPerPortion = 512;
  ULN2003FeederActuator(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);
  bool dispense(uint16_t portionCount) override;
  void update() override;
  void setStepsPerPortion(uint16_t steps);
  uint16_t stepsPerPortion() const;

 private:
  AccelStepper motor_;
  uint16_t stepsPerPortion_ = kDefaultStepsPerPortion;
};
