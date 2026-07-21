#pragma once

#include "domain/DeviceConfig.h"

class IConfigStorage {
 public:
  virtual ~IConfigStorage() = default;
  virtual bool begin() = 0;
  virtual bool load(DeviceConfig& config) = 0;
  virtual bool save(const DeviceConfig& config) = 0;
};
