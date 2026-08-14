#include "domain/Configuration.h"

Configuration::Configuration() : stepsPerFeed(DEFAULT_STEPS_PER_FEED)
{
    initializeDefaultSchedules();
}

void Configuration::initializeDefaultSchedules()
{
    schedules[0].hour = DEFAULT_SCHEDULE_HOUR;
    schedules[0].minute = DEFAULT_SCHEDULE_MINUTE;
    schedules[0].portions = DEFAULT_FEED_PORTIONS;
    schedules[0].enabled = true;
}

bool Configuration::isValidStepsPerFeed(int stepsPerFeed)
{
    return stepsPerFeed >= MIN_STEPS_PER_FEED && stepsPerFeed <= MAX_STEPS_PER_FEED;
}

bool Configuration::isValidPortions(int portions)
{
    return portions >= MIN_FEED_PORTIONS && portions <= MAX_FEED_PORTIONS;
}
