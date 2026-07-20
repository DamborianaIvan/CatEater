#pragma once

#include <stdint.h>

struct DeviceConfig {
  // Valor inicial de calibracion para el 28BYJ-48.
  uint16_t stepsPerPortion = 512;
};
