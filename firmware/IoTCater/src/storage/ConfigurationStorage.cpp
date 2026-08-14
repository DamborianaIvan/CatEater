#include "storage/ConfigurationStorage.h"
#include <EEPROM.h>

// EEPROM es UNO de los sitemas de almacenamiento que tiene nodemcu

void ConfigurationStorage::begin()
{
    EEPROM.begin(EEPROM_SIZE);
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
        saveConfiguration(defaultConfiguration);
        return defaultConfiguration;
    }

    Configuration configuration;

    EEPROM.get(CONFIG_ADDRESS, configuration);

    if (!isValidConfiguration(configuration))
    {
        Serial.println("[ConfigurationStorage] Configuracion invalida; restaurando valores por defecto.");
        saveConfiguration(defaultConfiguration);
        return defaultConfiguration;
    }

    return configuration;
}

bool ConfigurationStorage::isValidConfiguration(const Configuration& configuration) const
{
    if (configuration.stepsPerFeed <= 0)
    {
        return false;
    }

    for (const FeedSchedule& schedule : configuration.schedules)
    {
        if (schedule.hour > 23 || schedule.minute > 59 || schedule.portions == 0)
        {
            return false;
        }
    }

    return true;
}
