#include "infrastructure/ManualFeederActuator.h"

bool ManualFeederActuator::dispense(uint16_t portionGrams) {
  (void)portionGrams;
  return false;
}

void ManualFeederActuator::update() {
  // No hay hardware conectado en este adaptador temporal.
}
