#include "storage/ConfigurationStorage.h"
#include <EEPROM.h>

// EEPROM es UNO de los sitemas de almacenamiento que tiene nodemcu

void ConfigurationStorage::begin()
{
    EEPROM.begin(EEPROM_SIZE);
}

bool ConfigurationStorage::saveConfiguration(const Configuration& configuration)
{
    if (!isValidConfiguration(configuration))
    {
        return false;
    }

    StoredConfiguration stored{};
    stored.signature = CONFIG_SIGNATURE;
    stored.version = CONFIG_VERSION;
    stored.length = sizeof(Configuration);
    stored.configuration = configuration;
    stored.crc = calculateCrc(reinterpret_cast<const uint8_t*>(&stored),
                              offsetof(StoredConfiguration, crc));

    EEPROM.put(SIGNATURE_ADDRESS, stored);

    return EEPROM.commit();
}

Configuration ConfigurationStorage::loadConfiguration(const Configuration& defaultConfiguration)
{
    StoredConfiguration stored{};
    EEPROM.get(SIGNATURE_ADDRESS, stored);

    if (isValidStoredConfiguration(stored))
    {
        return stored.configuration;
    }

    Configuration legacyConfiguration;
    if (loadLegacyConfiguration(legacyConfiguration))
    {
        Serial.println("[ConfigurationStorage] Migrando configuracion EEPROM heredada.");
        if (!saveConfiguration(legacyConfiguration))
        {
            Serial.println("[ConfigurationStorage] Error migrando configuracion EEPROM.");
        }
        return legacyConfiguration;
    }

    Serial.println("[ConfigurationStorage] Configuracion invalida; restaurando valores por defecto.");
    if (!saveConfiguration(defaultConfiguration))
    {
        Serial.println("[ConfigurationStorage] Error guardando valores por defecto.");
    }
    return defaultConfiguration;
}

bool ConfigurationStorage::isValidConfiguration(const Configuration& configuration) const
{
    if (!Configuration::isValidStepsPerFeed(configuration.stepsPerFeed))
    {
        return false;
    }

    for (const FeedSchedule& schedule : configuration.schedules)
    {
        if (schedule.hour > 23 || schedule.minute > 59 ||
            !Configuration::isValidPortions(schedule.portions))
        {
            return false;
        }
    }

    return true;
}

bool ConfigurationStorage::isValidStoredConfiguration(const StoredConfiguration& stored) const
{
    if (stored.signature != CONFIG_SIGNATURE || stored.version != CONFIG_VERSION ||
        stored.length != sizeof(Configuration))
    {
        return false;
    }

    const uint32_t expectedCrc = calculateCrc(reinterpret_cast<const uint8_t*>(&stored),
                                              offsetof(StoredConfiguration, crc));
    return stored.crc == expectedCrc && isValidConfiguration(stored.configuration);
}

bool ConfigurationStorage::loadLegacyConfiguration(Configuration& configuration) const
{
    uint8_t signature = 0;
    EEPROM.get(SIGNATURE_ADDRESS, signature);
    if (signature != LEGACY_CONFIG_SIGNATURE)
    {
        return false;
    }

    EEPROM.get(CONFIG_ADDRESS, configuration);
    return isValidConfiguration(configuration);
}

uint32_t ConfigurationStorage::calculateCrc(const uint8_t* data, size_t length) const
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i)
    {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            const bool lsbSet = (crc & 1U) != 0;
            crc >>= 1U;
            if (lsbSet)
            {
                crc ^= 0xEDB88320U;
            }
        }
    }
    return ~crc;
}
