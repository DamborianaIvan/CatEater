#include "ConfigurationStorage.h"
#include <EEPROM.h>
namespace
{
    constexpr int SIGNATURE_ADDRESS = 0;
    constexpr int STEPS_PER_FEED_ADDRESS = 1;
    constexpr uint8_t CONFIG_SIGNATURE = 0xA5;
}
//EEPROM es UNO de los sitemas de almacenamiento que tiene nodemcu

void ConfigurationStorage::begin()
{
    EEPROM.begin(512);
    return;
}


bool ConfigurationStorage::saveConfiguration(const Configuration& configuration)
{
    EEPROM.put(SIGNATURE_ADDRESS, CONFIG_SIGNATURE);
    EEPROM.put(CONFIG_ADDRESS, configuration);

    return EEPROM.commit();
}


Configuration ConfigurationStorage::loadConfiguration(const Configuration& defaultConfiguration)
{
    uint8_t signature = 0;

    EEPROM.get(SIGNATURE_ADDRESS, signature);

    if (signature != CONFIG_SIGNATURE)
    {
        return defaultConfiguration;
    }

    Configuration configuration;

    EEPROM.get(CONFIG_ADDRESS, configuration);

    return configuration;
}