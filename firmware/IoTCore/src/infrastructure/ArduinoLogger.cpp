#include <Arduino.h>

#include "infrastructure/ArduinoLogger.h"

void ArduinoLogger::info(const char* message) {
  Serial.print("[INFO] ");
  Serial.println(message);
}

void ArduinoLogger::error(const char* message) {
  Serial.print("[ERROR] ");
  Serial.println(message);
}
