#include "ConfigurationStorage.h"
#include <EEPROM.h>


//EEPROM es UNO de los sitemas de almacenamiento que tiene nodemcu

void ConfigurationStorage::begin()
{
    EEPROM.begin(EEPROM_SIZE);
}


bool ConfigurationStorage::saveConfiguration(const Configuration& configuration)
{
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