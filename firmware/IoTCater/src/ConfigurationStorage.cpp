#include "ConfigurationStorage.h"

bool ConfigurationStorage::begin()
{
    return true;
}

bool ConfigurationStorage::saveStepsPerFeed(int steps)
{
    return true;
}

int ConfigurationStorage::loadStepsPerFeed(int defaultValue) const
{
    return defaultValue;
}