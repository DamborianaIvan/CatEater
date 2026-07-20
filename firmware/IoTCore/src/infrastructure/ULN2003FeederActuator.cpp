#include "infrastructure/ULN2003FeederActuator.h"

namespace {
constexpr float kMaxSpeed = 200.0F;
constexpr float kAcceleration = 100.0F;
constexpr uint32_t kMaximumStepsPerOrder = 32767;
}  // namespace

ULN2003FeederActuator::ULN2003FeederActuator(uint8_t in1, uint8_t in2,
                                             uint8_t in3, uint8_t in4)
    // El orden electrico correcto del 28BYJ-48 es IN1, IN3, IN2, IN4.
    : motor_(AccelStepper::FULL4WIRE, in1, in3, in2, in4) {
  motor_.setMaxSpeed(kMaxSpeed);
  motor_.setAcceleration(kAcceleration);
}

bool ULN2003FeederActuator::dispense(uint16_t portionCount) {
  const uint32_t steps = stepsPerPortion_ * portionCount;
  if (steps == 0 || steps > kMaximumStepsPerOrder || motor_.isRunning()) {
    return false;
  }
  motor_.move(static_cast<long>(steps));
  return true;
}

void ULN2003FeederActuator::update() {
  motor_.run();
}

void ULN2003FeederActuator::setStepsPerPortion(uint16_t steps) {
  if (steps > 0 && !motor_.isRunning()) stepsPerPortion_ = steps;
}

uint16_t ULN2003FeederActuator::stepsPerPortion() const {
  return stepsPerPortion_;
}
