#ifndef CONFIGURATION_STORAGE_H
#define CONFIGURATION_STORAGE_H

class ConfigurationStorage
{
public:

    bool begin();

    bool saveStepsPerFeed(int steps);

    int loadStepsPerFeed(int defaultValue) const;
};

#endif