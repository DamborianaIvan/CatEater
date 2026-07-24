#ifndef CONFIGURATION_STORAGE_H
#define CONFIGURATION_STORAGE_H

#include <EEPROM.h>

struct Configuration
{
    static constexpr int DEFAULT_STEPS_PER_FEED = 2048;
    static constexpr int DEFAULT_SCHEDULED_HOUR = 9;
    static constexpr int DEFAULT_SCHEDULED_MINUTE = 0;

    int stepsPerFeed = DEFAULT_STEPS_PER_FEED;
    int scheduledHour = DEFAULT_SCHEDULED_HOUR;
    int scheduledMinute = DEFAULT_SCHEDULED_MINUTE;
};
class ConfigurationStorage
{
public:
    void begin();

    bool saveConfiguration(const Configuration& configuration);

    Configuration loadConfiguration(
        const Configuration& defaultConfiguration);

private:
    static constexpr int EEPROM_SIZE = 512;

    static constexpr int SIGNATURE_ADDRESS = 0;
    static constexpr int CONFIG_ADDRESS = 1;

    static constexpr uint8_t CONFIG_SIGNATURE = 0xA5;
};

#endif