#pragma once

#include "ports/IConfigStorage.h"

class LittleFsConfigStorage final : public IConfigStorage {
 public:
  bool begin() override;
  bool load(DeviceConfig& config) override;
  bool save(const DeviceConfig& config) override;

 private:
  static constexpr const char* kConfigPath = "/config.bin";
};
