#include <LittleFS.h>

#include "infrastructure/LittleFsConfigStorage.h"

namespace {
struct StoredConfig {
  uint32_t magic;
  DeviceConfig config;
};
constexpr uint32_t kConfigMagic = 0x43415431;
}  // namespace

bool LittleFsConfigStorage::begin() {
  return LittleFS.begin();
}

bool LittleFsConfigStorage::load(DeviceConfig& config) {
  if (!LittleFS.exists(kConfigPath)) return false;
  File file = LittleFS.open(kConfigPath, "r");
  if (!file) {
    return false;
  }
  StoredConfig stored{};
  const bool wasRead = file.readBytes(reinterpret_cast<char*>(&stored), sizeof(stored)) == sizeof(stored);
  file.close();
  if (!wasRead || stored.magic != kConfigMagic) {
    return false;
  }
  config = stored.config;
  return true;
}

bool LittleFsConfigStorage::save(const DeviceConfig& config) {
  const StoredConfig stored = {kConfigMagic, config};
  File file = LittleFS.open(kConfigPath, "w");
  if (!file) {
    return false;
  }
  const bool wasWritten = file.write(reinterpret_cast<const uint8_t*>(&stored), sizeof(stored)) == sizeof(stored);
  file.close();
  return wasWritten;
}
