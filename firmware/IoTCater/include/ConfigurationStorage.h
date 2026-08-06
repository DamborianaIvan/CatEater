#ifndef CONFIGURATION_STORAGE_H
#define CONFIGURATION_STORAGE_H
#include "Configuration.h"
#include <EEPROM.h>

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

    static constexpr uint8_t CONFIG_SIGNATURE = 0xA5;
};

#endif