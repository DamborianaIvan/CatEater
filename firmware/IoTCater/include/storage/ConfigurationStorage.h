#ifndef CONFIGURATION_STORAGE_H
#define CONFIGURATION_STORAGE_H
#include "domain/Configuration.h"
#include <EEPROM.h>
#include <cstddef>

class ConfigurationStorage
{
   public:
    void begin();

    bool saveConfiguration(const Configuration& configuration);

    Configuration loadConfiguration(const Configuration& defaultConfiguration);

   private:
    static constexpr int EEPROM_SIZE = 512;

    static constexpr int SIGNATURE_ADDRESS = 0;
    static constexpr int CONFIG_ADDRESS = 1;
    static constexpr uint8_t LEGACY_CONFIG_SIGNATURE = 0xA5;
    static constexpr uint8_t CONFIG_SIGNATURE = 0xC7;
    static constexpr uint8_t CONFIG_VERSION = 1;

    struct StoredConfiguration
    {
        uint8_t signature;
        uint8_t version;
        uint16_t length;
        Configuration configuration;
        uint32_t crc;
    };

    bool isValidConfiguration(const Configuration& configuration) const;
    bool isValidStoredConfiguration(const StoredConfiguration& stored) const;
    bool loadLegacyConfiguration(Configuration& configuration) const;
    uint32_t calculateCrc(const uint8_t* data, size_t length) const;
};

#endif
