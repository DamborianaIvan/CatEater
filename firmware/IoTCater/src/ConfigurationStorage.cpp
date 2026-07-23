#include "ConfigurationStorage.h"
#include <EEPROM.h>
namespace
{
    constexpr int SIGNATURE_ADDRESS = 0;
    constexpr int STEPS_PER_FEED_ADDRESS = 1;
    constexpr uint8_t CONFIG_SIGNATURE = 0xA5;
}
//EEPROM es UNO de los sitemas de almacenamiento que tiene nodemcu

bool ConfigurationStorage::begin()
{
    EEPROM.begin(512);
    return true;
}

bool ConfigurationStorage::saveStepsPerFeed(int steps)
{
    EEPROM.put(
        SIGNATURE_ADDRESS,
        CONFIG_SIGNATURE
    );

    EEPROM.put(
        STEPS_PER_FEED_ADDRESS,
        steps
    );

    return EEPROM.commit();
}

int ConfigurationStorage::loadStepsPerFeed(int defaultValue) const
{
    uint8_t signature = 0;
    //Obtenemos la firma inicial para validar si le agregamos un step nosotros o esat inicializandose por primera vez
    EEPROM.get(
        SIGNATURE_ADDRESS,
        signature
    );
    if(signature != CONFIG_SIGNATURE)
    {
        return defaultValue;
    }

    int steps;

    EEPROM.get(
        STEPS_PER_FEED_ADDRESS,
        steps
    );
    return steps;
}